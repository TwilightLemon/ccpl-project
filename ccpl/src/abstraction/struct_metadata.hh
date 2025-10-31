#pragma once
#include <string>
#include <memory>
#include <vector>
#include "tac_definitions.hh"

namespace twlm::ccpl::abstraction
{
    // Forward declaration
    struct Type;

    /**
     * Metadata for a struct field, preserving complete type information
     */
    struct StructFieldMetadata
    {
        std::string name;                     // Field name
        std::shared_ptr<Type> type;          // Complete type information (can be basic, array, nested struct, etc.)
        int offset;                          // Offset in struct layout (for later code generation)

        StructFieldMetadata() : offset(0) {}
        
        StructFieldMetadata(const std::string& n, std::shared_ptr<Type> t, int off = 0)
            : name(n), type(t), offset(off) {}
    };

    /**
     * Metadata for a struct type definition
     * Stores complete type information without flattening/expanding
     */
    struct StructTypeMetadata
    {
        std::string name;                                    // Struct type name
        std::vector<StructFieldMetadata> fields;            // Field definitions with complete type info
        int total_size;                                      // Total size in bytes/words (calculated after expansion)

        StructTypeMetadata() : total_size(0) {}
        
        StructTypeMetadata(const std::string& n) : name(n), total_size(0) {}

        /**
         * Calculate the total size of the struct
         * This should be called after all fields are added
         */
        void calculate_size();

        /**
         * Get field by name
         */
        const StructFieldMetadata* get_field(const std::string& field_name) const;

        /**
         * Check if a field exists
         */
        bool has_field(const std::string& field_name) const;
    };

} // namespace twlm::ccpl::abstraction
