#pragma once

namespace twlm::ccpl::abstraction
{
    // Data Types
    enum class DATA_TYPE
    {
        VOID,
        INT,
        CHAR,
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

    // Symbol Types
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

    // Symbol Scope
    enum class SYM_SCOPE
    {
        GLOBAL,
        LOCAL
    };

    // TAC Operations
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
        MEMBER,    // a = b.member (member access: a is destination, b is struct var, c holds field info)
        STRUCT_DECL // struct declaration
    };
}