#pragma once
#include <string>
#include <vector>
#include <memory>
#include "tac_definitions.hh"

namespace twlm::ccpl::abstraction
{
    struct ArrayMetadata
    {
        std::string name;                    // Full variable name (e.g., "a1.a" or "arr")
        std::vector<int> dimensions;         // Dimension sizes from outer to inner (e.g., [5, 10] for char[5][10])
        int element_size;                    // Size of each element in bytes/words (usually 4 for int/char in this TAC)
        DATA_TYPE base_type;                 // Base element type (e.g., CHAR for char[5][10])
        
        ArrayMetadata() : element_size(4), base_type(DATA_TYPE::UNDEF) {}
        
        ArrayMetadata(const std::string& n, const std::vector<int>& dims, DATA_TYPE btype, int elem_size = 4)
            : name(n), dimensions(dims), element_size(elem_size), base_type(btype) {}
        
        /**
         * Get the total number of elements in the array
         */
        int get_total_elements() const {
            int total = 1;
            for (int dim : dimensions) {
                total *= dim;
            }
            return total;
        }
        
        /**
         * Get the number of dimensions
         */
        size_t get_dimension_count() const {
            return dimensions.size();
        }
        
        /**
         * Calculate the stride for a given dimension
         * Stride is the number of elements to skip when incrementing the index at that dimension
         * For example, in a[5][10], stride for dimension 0 is 10, stride for dimension 1 is 1
         */
        int get_stride(size_t dim_index) const {
            if (dim_index >= dimensions.size()) {
                return 0;
            }
            
            int stride = 1;
            // Multiply all dimension sizes after the current dimension
            for (size_t i = dim_index + 1; i < dimensions.size(); ++i) {
                stride *= dimensions[i];
            }
            return stride;
        }
        
        /**
         * Get a readable string representation
         */
        std::string to_string() const {
            std::string result = name + "[";
            for (size_t i = 0; i < dimensions.size(); ++i) {
                if (i > 0) result += "][";
                result += std::to_string(dimensions[i]);
            }
            result += "]";
            return result;
        }
    };

} // namespace twlm::ccpl::abstraction
