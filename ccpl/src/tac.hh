#pragma once
#include <string>
#include <variant>
#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>

// Data Types
enum class DATA_TYPE {
    VOID,
    INT,
    CHAR,
    UNDEF
};

// Symbol Types
enum class SYM_TYPE {
    UNDEF,
    VAR,
    FUNC,
    TEXT,
    LABEL,
    CONST_INT,
    CONST_CHAR
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
    SYM_TYPE type;           // Symbol type (VAR, FUNC, etc.)
    DATA_TYPE data_type;     // Data type (INT, CHAR, VOID)
    SYM_SCOPE scope;
    std::string name;
    std::variant<int, char, std::string> value;
    int offset;
    int label;
    
    // For functions
    std::vector<DATA_TYPE> param_types;  // Parameter types
    DATA_TYPE return_type;                // Return type
    
    SYM() : type(SYM_TYPE::UNDEF), data_type(DATA_TYPE::UNDEF),
            scope(SYM_SCOPE::GLOBAL), offset(-1), label(-1),
            return_type(DATA_TYPE::UNDEF) {}
};

// TAC instruction structure
struct TAC {
    TAC_OP op;
    std::shared_ptr<SYM> a,b,c;
    std::shared_ptr<TAC> prev,next;
    
    TAC(TAC_OP op = TAC_OP::UNDEF) : op(op), prev(nullptr), next(nullptr) {}
};

// Expression structure
struct EXP {
    std::shared_ptr<TAC> code;
    std::shared_ptr<SYM> place;
    DATA_TYPE data_type;         // Type of the expression result
    std::shared_ptr<EXP> next;   // for argument lists
    
    EXP() : code(nullptr), place(nullptr), 
            data_type(DATA_TYPE::UNDEF), next(nullptr) {}
};

// TAC Generator class
class TACGenerator {
private:
    SYM_SCOPE scope;
    int next_tmp;
    int next_label;
    
    DATA_TYPE current_var_type;
    std::shared_ptr<SYM> current_func;
    
    std::unordered_map<std::string, std::shared_ptr<SYM>> sym_tab_global;
    std::unordered_map<std::string, std::shared_ptr<SYM>> sym_tab_local;
    
    std::shared_ptr<TAC> tac_first;
    std::shared_ptr<TAC> tac_last;

public:
    TACGenerator();
    
    // Initialization
    void init();
    void complete();
    
    // Type management
    void set_current_type(DATA_TYPE type);
    DATA_TYPE get_current_type() const;
    
    // Symbol table operations
    std::shared_ptr<SYM> lookup_sym(const std::string& name);
    std::shared_ptr<SYM> mk_var(const std::string& name, DATA_TYPE dtype);
    std::shared_ptr<SYM> mk_tmp(DATA_TYPE dtype = DATA_TYPE::INT);
    std::shared_ptr<SYM> mk_const(int value);
    std::shared_ptr<SYM> mk_const_char(char value);
    std::shared_ptr<SYM> mk_text(const std::string& text);
    std::shared_ptr<SYM> mk_label(const std::string& name);
    std::shared_ptr<SYM> get_var(const std::string& name);
    std::shared_ptr<SYM> declare_func(const std::string& name, DATA_TYPE return_type);
    
    // TAC operations
    std::shared_ptr<TAC> mk_tac(TAC_OP op, 
                                std::shared_ptr<SYM> a = nullptr,
                                std::shared_ptr<SYM> b = nullptr,
                                std::shared_ptr<SYM> c = nullptr);
    std::shared_ptr<TAC> join_tac(std::shared_ptr<TAC> c1, std::shared_ptr<TAC> c2);
    
    // Declaration operations
    std::shared_ptr<TAC> declare_var(const std::string& name, DATA_TYPE dtype);
    std::shared_ptr<TAC> declare_para(const std::string& name, DATA_TYPE dtype);
    
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
    
    // Type checking
    bool check_type_compatibility(DATA_TYPE t1, DATA_TYPE t2);
    DATA_TYPE infer_binary_type(DATA_TYPE t1, DATA_TYPE t2);
    void check_assignment_type(std::shared_ptr<SYM> var, std::shared_ptr<EXP> exp);
    void check_return_type(std::shared_ptr<EXP> exp);
    
    // Output
    void print_tac(std::ostream& os = std::cout);
    void print_symbol_table(std::ostream& os = std::cout);
    std::string sym_to_string(std::shared_ptr<SYM> s);
    std::string tac_to_string(std::shared_ptr<TAC> t);
    std::string data_type_to_string(DATA_TYPE type);
    
    // Error handling
    void error(const std::string& msg);
    void warning(const std::string& msg);
    
    // Getters
    std::shared_ptr<TAC> get_tac_first() const { return tac_first; }
    std::shared_ptr<TAC> get_tac_last() const { return tac_last; }
};

// Global TAC generator instance
extern TACGenerator tac_gen;

