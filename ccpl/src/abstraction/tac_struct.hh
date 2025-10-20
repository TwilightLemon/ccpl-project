#pragma once
#include <string>
#include <variant>
#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "tac_definitions.hh"

namespace twlm::ccpl::abstraction
{
    // Symbol structure
    struct SYM
    {
        SYM_TYPE type;       // Symbol type (VAR, FUNC, etc.)
        DATA_TYPE data_type; // Data type (INT, CHAR, VOID)
        SYM_SCOPE scope;
        std::string name;
        std::variant<int, char, std::string> value;
        int offset;
        int label;

        // For functions
        std::vector<DATA_TYPE> param_types; // Parameter types
        DATA_TYPE return_type;              // Return type

        SYM() : type(SYM_TYPE::UNDEF), data_type(DATA_TYPE::UNDEF),
                scope(SYM_SCOPE::GLOBAL), offset(-1), label(-1),
                return_type(DATA_TYPE::UNDEF) {}
    };

    // TAC instruction structure
    struct TAC
    {
        TAC_OP op;
        std::shared_ptr<SYM> a, b, c;
        std::shared_ptr<TAC> prev, next;

        TAC(TAC_OP op = TAC_OP::UNDEF) : op(op), prev(nullptr), next(nullptr) {}
    };

    // Expression structure
    struct EXP
    {
        std::shared_ptr<TAC> code;
        std::shared_ptr<SYM> place;
        DATA_TYPE data_type;       // Type of the expression result
        std::shared_ptr<EXP> next; // for argument lists

        EXP() : code(nullptr), place(nullptr),
                data_type(DATA_TYPE::UNDEF), next(nullptr) {}
    };

}