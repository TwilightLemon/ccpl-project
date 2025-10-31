#include "struct_metadata.hh"
#include "ast_nodes.hh"
#include "tac_struct.hh"

namespace twlm::ccpl::abstraction
{
    static int calculate_type_size(const std::shared_ptr<Type> &type,
                                   const std::unordered_map<std::string, std::shared_ptr<SYM>> &declared_structs)
    {
        if (!type)
            return 4; // Default size

        switch (type->kind)
        {
        case TypeKind::BASIC:
        case TypeKind::POINTER:
            return 4;

        case TypeKind::ARRAY:
        {
            if (type->base_type && type->array_size > 0)
            {
                int element_size = calculate_type_size(type->base_type, declared_structs);
                return element_size * type->array_size;
            }
            return 4;
        }

        case TypeKind::STRUCT:
        {
            auto struct_it = declared_structs.find(type->struct_name);
            if (struct_it != declared_structs.end() &&
                struct_it->second->struct_metadata)
            {
                return struct_it->second->struct_metadata->total_size;
            }
            return 4;
        }

        default:
            return 4;
        }
    }
    void StructTypeMetadata::calculate_size(const std::unordered_map<std::string, std::shared_ptr<SYM>> &declared_structs)
    {
        total_size = 0;
        for (auto &field : fields)
        {
            field.offset = total_size;

            if (field.type)
            {
                switch (field.type->kind)
                {
                // int/char/ptr is 4 bytes
                case TypeKind::BASIC:
                    total_size += 4;
                    break;

                case TypeKind::POINTER:
                    total_size += 4;
                    break;

                // array and struct should be calculated properly
                case TypeKind::ARRAY:
                {
                    // Calculate array total size = element_size * array_size
                    if (field.type->base_type && field.type->array_size > 0)
                    {
                        int element_size = calculate_type_size(field.type->base_type, declared_structs);
                        total_size += element_size * field.type->array_size;
                    }
                    else
                    {
                        total_size += 4; // Default if array info is invalid
                    }
                    break;
                }

                case TypeKind::STRUCT:
                {
                    // For nested structs, look up the struct metadata
                    auto struct_it = declared_structs.find(field.type->struct_name);
                    if (struct_it != declared_structs.end() &&
                        struct_it->second->struct_metadata)
                    {
                        total_size += struct_it->second->struct_metadata->total_size;
                    }
                    else
                    {
                        total_size += 4; // Default if struct not found
                    }
                    break;
                }

                default:
                    total_size += 4;
                    break;
                }
            }
            else
            {
                total_size += 4; // Default size
            }
        }
    }

    const StructFieldMetadata *StructTypeMetadata::get_field(const std::string &field_name) const
    {
        for (const auto &field : fields)
        {
            if (field.name == field_name)
            {
                return &field;
            }
        }
        return nullptr;
    }

    bool StructTypeMetadata::has_field(const std::string &field_name) const
    {
        return get_field(field_name) != nullptr;
    }

} // namespace twlm::ccpl::abstraction
