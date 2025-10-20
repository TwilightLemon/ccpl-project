#pragma once
#include <string>
#include <variant>
#include <memory>

enum class SYM_TYPE
{
    UNDEF,
    VAR,
    FUNC,
    TEXT,
    LABEL,
    INT,
    CHAR
};
enum class SYM_SCOPE
{
    GLOBAL,
    LOCAL
};
struct SYM
{
    SYM_TYPE type;
    SYM_SCOPE scope;
    std::string name;
    std::variant<int, std::string, char> value;
    int offset;
    int address;
    int label;
};

enum class TAC_OP
{
    UNDEF,     // undefine
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
    BEGINFUNC, // function begin
    ENDFUNC,   // function end
    LABEL,     // label a
    VAR,       // int a
    FORMAL,    // formal a
    ACTUAL,    // actual a
    CALL,      // a = call b
    RETURN,    // return a
    INPUT,     // input a
    OUTPUT     // output a
};

struct TAC
{
    TAC_OP op;
    std::shared_ptr<SYM> a;
    std::shared_ptr<SYM> b;
    std::shared_ptr<SYM> c;
};

struct EXP{
    std::shared_ptr<TAC> code;
    std::shared_ptr<SYM> place;
};

