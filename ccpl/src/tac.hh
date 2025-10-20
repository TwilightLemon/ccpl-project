#pragma once
#include <string>
#include <variant>
#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>

// Symbol Types
enum class SYM_TYPE {
    UNDEF,
    VAR,
    FUNC,
    TEXT,
    LABEL,
    INT,
    CHAR
};

// Symbol Scope
enum class SYM_SCOPE {
    GLOBAL,
    LOCAL
};

// TAC Operations
enum class TAC_OP {
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

// Symbol structure
struct SYM {
    SYM_TYPE type;
    SYM_SCOPE scope;
    std::string name;
    std::variant<int, char, std::string> value;
    int offset;
    int label;
    
    SYM() : type(SYM_TYPE::UNDEF), scope(SYM_SCOPE::GLOBAL), 
            offset(-1), label(-1) {}
};

// TAC instruction structure
struct TAC {
    TAC_OP op;
    std::shared_ptr<SYM> a;
    std::shared_ptr<SYM> b;
    std::shared_ptr<SYM> c;
    std::shared_ptr<TAC> prev;
    std::shared_ptr<TAC> next;
    
    TAC(TAC_OP op = TAC_OP::UNDEF) : op(op), prev(nullptr), next(nullptr) {}
};

// Expression structure
struct EXP {
    std::shared_ptr<TAC> code;
    std::shared_ptr<SYM> place;
    std::shared_ptr<EXP> next;  // for argument lists
    
    EXP() : code(nullptr), place(nullptr), next(nullptr) {}
};

// TAC Generator class
class TACGenerator {
private:
    SYM_SCOPE scope;
    int next_tmp;
    int next_label;
    
    std::unordered_map<std::string, std::shared_ptr<SYM>> sym_tab_global;
    std::unordered_map<std::string, std::shared_ptr<SYM>> sym_tab_local;
    
    std::shared_ptr<TAC> tac_first;
    std::shared_ptr<TAC> tac_last;

public:
    TACGenerator();
    
    // Initialization
    void init();
    void complete();
    
    // Symbol table operations
    std::shared_ptr<SYM> lookup_sym(const std::string& name);
    std::shared_ptr<SYM> mk_var(const std::string& name);
    std::shared_ptr<SYM> mk_tmp();
    std::shared_ptr<SYM> mk_const(int value);
    std::shared_ptr<SYM> mk_const_char(char value);
    std::shared_ptr<SYM> mk_text(const std::string& text);
    std::shared_ptr<SYM> mk_label(const std::string& name);
    std::shared_ptr<SYM> get_var(const std::string& name);
    std::shared_ptr<SYM> declare_func(const std::string& name);
    
    // TAC operations
    std::shared_ptr<TAC> mk_tac(TAC_OP op, 
                                std::shared_ptr<SYM> a = nullptr,
                                std::shared_ptr<SYM> b = nullptr,
                                std::shared_ptr<SYM> c = nullptr);
    std::shared_ptr<TAC> join_tac(std::shared_ptr<TAC> c1, std::shared_ptr<TAC> c2);
    
    // Declaration operations
    std::shared_ptr<TAC> declare_var(const std::string& name);
    std::shared_ptr<TAC> declare_para(const std::string& name);
    
    // Statement operations
    std::shared_ptr<TAC> do_func(std::shared_ptr<SYM> func, 
                                 std::shared_ptr<TAC> args, 
                                 std::shared_ptr<TAC> code);
    std::shared_ptr<TAC> do_assign(std::shared_ptr<SYM> var, std::shared_ptr<EXP> exp);
    std::shared_ptr<TAC> do_input(std::shared_ptr<SYM> var);
    std::shared_ptr<TAC> do_output(std::shared_ptr<SYM> sym);
    std::shared_ptr<TAC> do_return(std::shared_ptr<EXP> exp);
    std::shared_ptr<TAC> do_if(std::shared_ptr<EXP> exp, std::shared_ptr<TAC> stmt);
    std::shared_ptr<TAC> do_if_else(std::shared_ptr<EXP> exp, 
                                    std::shared_ptr<TAC> stmt1, 
                                    std::shared_ptr<TAC> stmt2);
    std::shared_ptr<TAC> do_while(std::shared_ptr<EXP> exp, std::shared_ptr<TAC> stmt);
    std::shared_ptr<TAC> do_for(std::shared_ptr<TAC> init,
                                std::shared_ptr<EXP> cond,
                                std::shared_ptr<TAC> update,
                                std::shared_ptr<TAC> body);
    std::shared_ptr<TAC> do_call(const std::string& name, std::shared_ptr<EXP> arglist);
    
    // Expression operations
    std::shared_ptr<EXP> mk_exp(std::shared_ptr<SYM> place, 
                                std::shared_ptr<TAC> code,
                                std::shared_ptr<EXP> next = nullptr);
    std::shared_ptr<EXP> do_bin(TAC_OP op, std::shared_ptr<EXP> exp1, std::shared_ptr<EXP> exp2);
    std::shared_ptr<EXP> do_un(TAC_OP op, std::shared_ptr<EXP> exp);
    std::shared_ptr<EXP> do_call_ret(const std::string& name, std::shared_ptr<EXP> arglist);
    
    // Scope management
    void enter_scope();
    void leave_scope();
    
    // Output
    void print_tac(std::ostream& os = std::cout);
    std::string sym_to_string(std::shared_ptr<SYM> s);
    std::string tac_to_string(std::shared_ptr<TAC> t);
    
    // Error handling
    void error(const std::string& msg);
    
    // Getters
    std::shared_ptr<TAC> get_tac_first() const { return tac_first; }
    std::shared_ptr<TAC> get_tac_last() const { return tac_last; }
};

// Global TAC generator instance
extern TACGenerator tac_gen;

