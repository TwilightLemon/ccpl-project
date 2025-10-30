#pragma once

namespace twlm::ccpl::abstraction
{
    enum class DATA_TYPE
    {
        VOID,
        INT,
        CHAR,
        STRUCT, //for recognizing struct types only, not a real basic TAC type(
        UNDEF
    };

    static std::string data_type_to_string(DATA_TYPE type)
    {
        switch (type)
        {
        case DATA_TYPE::VOID:
            return "void";
        case DATA_TYPE::INT:
            return "int";
        case DATA_TYPE::CHAR:
            return "char";
        case DATA_TYPE::UNDEF:
            return "undefined";
        default:
            return "unknown";
        }
    }

    enum class SYM_TYPE
    {
        UNDEF,
        VAR,
        FUNC,
        TEXT,
        LABEL,
        CONST_INT,
        CONST_CHAR,
        STRUCT_TYPE  // For struct type definitions
    };

    enum class SYM_SCOPE
    {
        GLOBAL,
        LOCAL
    };

    enum class TAC_OP
    {
        UNDEF,     // undefined
        ADD,       // a = b + c
        SUB,       // a = b - c
        MUL,       // a = b * c
        DIV,       // a = b / c
        EQ,        // a = (b == c)
        NE,        // a = (b != c)
        LT,        // a = (b < c)
        LE,        // a = (b <= c)
        GT,        // a = (b > c)
        GE,        // a = (b >= c)
        NEG,       // a = -b
        COPY,      // a = b
        GOTO,      // goto a
        IFZ,       // ifz b goto a
        //IF,        // if b goto a
        BEGINFUNC, // function begin
        ENDFUNC,   // function end
        LABEL,     // label a
        VAR,       // int a
        FORMAL,    // formal a
        ACTUAL,    // actual a
        CALL,      // a = call b
        RETURN,    // return a
        INPUT,     // input a
        OUTPUT,    // output a
        ADDR,      // a = &b (address of)
        LOAD_PTR,  // a = *b (load from pointer)
        STORE_PTR  // *a = b (store to pointer)
    };
}