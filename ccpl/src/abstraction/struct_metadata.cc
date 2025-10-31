#include "struct_metadata.hh"
#include "ast_nodes.hh"

namespace twlm::ccpl::abstraction
{
    void StructTypeMetadata::calculate_size()
    {
        total_size = 0;
        for (auto& field : fields)
        {
            field.offset = total_size;
            
            if (field.type)
            {
                switch (field.type->kind)
                {
                    //int/char/ptr is 4b
                    case TypeKind::BASIC:
                        total_size += 4;
                        break;
                        
                    case TypeKind::POINTER:
                        total_size += 4;
                        break;
                        
                    //array and struct should be calculated properly
                    case TypeKind::ARRAY:
                    {
                        // Calculate array total size
                        int array_size = 4; // Base element size
                        auto current_type = field.type;
                        while (current_type && current_type->kind == TypeKind::ARRAY)
                        {
                            array_size *= current_type->array_size;
                            current_type = current_type->base_type;
                        }
                        total_size += array_size;
                        break;
                    }
                    
                    case TypeKind::STRUCT:
                        // For nested structs, size needs to be looked up
                        // This will be handled during expansion
                        total_size += 4; // Placeholder
                        break;
                        
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

    const StructFieldMetadata* StructTypeMetadata::get_field(const std::string& field_name) const
    {
        for (const auto& field : fields)
        {
            if (field.name == field_name)
            {
                return &field;
            }
        }
        return nullptr;
    }

    bool StructTypeMetadata::has_field(const std::string& field_name) const
    {
        return get_field(field_name) != nullptr;
    }

} // namespace twlm::ccpl::abstraction
