#pragma once
#include <string>
#include <variant>
#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "abstraction/tac_struct.hh"

namespace twlm::ccpl::modules
{
    using namespace twlm::ccpl::abstraction;
    
    class TACGenerator
    {
    private:
        SYM_SCOPE scope;
        int next_tmp;
        int next_label;

        DATA_TYPE current_var_type;
        std::shared_ptr<SYM> current_func;

        std::unordered_map<std::string, std::shared_ptr<SYM>> sym_tab_global;
        std::unordered_map<std::string, std::shared_ptr<SYM>> sym_tab_local;
        std::unordered_map<std::string, std::shared_ptr<SYM>> struct_types; // Struct type definitions

        std::shared_ptr<TAC> tac_first;
        std::shared_ptr<TAC> tac_last;
        
        // Loop context stack for break/continue
        std::vector<LoopContext> loop_stack;
        std::vector<SwitchContext> switch_stack;

    public:
        TACGenerator();

        // Initialization
        void init();
        void complete();

        // Type management
        void set_current_type(DATA_TYPE type);
        DATA_TYPE get_current_type() const;

        // Symbol table operations
        std::shared_ptr<SYM> lookup_sym(const std::string &name);
        std::shared_ptr<SYM> mk_var(const std::string &name, DATA_TYPE dtype);
        std::shared_ptr<SYM> mk_tmp(DATA_TYPE dtype = DATA_TYPE::INT);
        std::shared_ptr<SYM> mk_const(int value);
        std::shared_ptr<SYM> mk_const_char(char value);
        std::shared_ptr<SYM> mk_text(const std::string &text);
        std::shared_ptr<SYM> mk_label(const std::string &name);
        std::shared_ptr<SYM> get_var(const std::string &name);
        std::shared_ptr<SYM> declare_func(const std::string &name, DATA_TYPE return_type);
        
        // Struct operations
        std::shared_ptr<SYM> declare_struct_type(const std::string &name,
                                                 std::shared_ptr<StructTypeMetadata> metadata);
        std::shared_ptr<SYM> get_struct_type(const std::string &name);
        std::shared_ptr<TAC> do_member_access(std::shared_ptr<SYM> struct_var, const std::string& field_name);

        // TAC operations
        std::shared_ptr<TAC> mk_tac(TAC_OP op,
                                    std::shared_ptr<SYM> a = nullptr,
                                    std::shared_ptr<SYM> b = nullptr,
                                    std::shared_ptr<SYM> c = nullptr);
        std::shared_ptr<TAC> join_tac(std::shared_ptr<TAC> c1, std::shared_ptr<TAC> c2);

        // Declaration operations
        std::shared_ptr<TAC> declare_var(const std::string &name, DATA_TYPE dtype, bool is_pointer = false);
        std::shared_ptr<TAC> declare_para(const std::string &name, DATA_TYPE dtype, bool is_pointer = false);

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
        
        // Prepare loop context before parsing loop body
        void begin_while_loop();
        void begin_for_loop();
        std::shared_ptr<TAC> end_while_loop(std::shared_ptr<EXP> exp, std::shared_ptr<TAC> stmt);
        std::shared_ptr<TAC> end_for_loop(std::shared_ptr<TAC> init,
                                          std::shared_ptr<EXP> cond,
                                          std::shared_ptr<TAC> update,
                                          std::shared_ptr<TAC> body);
        
        std::shared_ptr<TAC> do_call(const std::string &name, std::shared_ptr<EXP> arglist);
        std::shared_ptr<TAC> do_break();
        std::shared_ptr<TAC> do_continue();

        void begin_switch();
        std::shared_ptr<TAC> end_switch(std::shared_ptr<EXP> exp, std::shared_ptr<TAC> body);
        std::shared_ptr<TAC> do_case(int value);
        std::shared_ptr<TAC> do_default();

        // Expression operations
        std::shared_ptr<EXP> mk_exp(std::shared_ptr<SYM> place,
                                    std::shared_ptr<TAC> code,
                                    std::shared_ptr<EXP> next = nullptr);
        std::shared_ptr<EXP> do_bin(TAC_OP op, std::shared_ptr<EXP> exp1, std::shared_ptr<EXP> exp2);
        std::shared_ptr<EXP> do_un(TAC_OP op, std::shared_ptr<EXP> exp);
        std::shared_ptr<EXP> do_call_ret(const std::string &name, std::shared_ptr<EXP> arglist);
        
        // Pointer operations
        std::shared_ptr<EXP> do_address_of(std::shared_ptr<EXP> exp);
        std::shared_ptr<EXP> do_dereference(std::shared_ptr<EXP> exp);
        std::shared_ptr<TAC> do_pointer_assign(std::shared_ptr<EXP> ptr_exp, std::shared_ptr<EXP> value_exp);

        // Scope management
        void enter_scope();
        void leave_scope();
        
        // Loop management for break/continue
        void enter_loop(std::shared_ptr<SYM> break_label, std::shared_ptr<SYM> continue_label, std::shared_ptr<SYM> loop_start_label=nullptr);
        void leave_loop();
        bool in_loop() const;

        // Switch management
        void enter_switch(std::shared_ptr<SYM> break_label, std::shared_ptr<SYM> default_label);
        void leave_switch();
        bool in_switch() const;

        // Type checking
        bool check_type_compatibility(DATA_TYPE t1, DATA_TYPE t2);
        DATA_TYPE infer_binary_type(DATA_TYPE t1, DATA_TYPE t2);
        void check_assignment_type(std::shared_ptr<SYM> var, std::shared_ptr<EXP> exp);
        void check_return_type(std::shared_ptr<EXP> exp);

        // Output
        void print_tac(std::ostream &os = std::cout);
        void print_symbol_table(std::ostream &os = std::cout);

        // Error handling
        void error(const std::string &msg);
        void warning(const std::string &msg);

        // Getters
        std::shared_ptr<TAC> get_tac_first() const { return tac_first; }
        std::shared_ptr<TAC> get_tac_last() const { return tac_last; }
        const std::unordered_map<std::string, std::shared_ptr<SYM>>& get_global_symbols() const { return sym_tab_global; }
        
        // Link a TAC node to the global TAC chain
        void link_tac(std::shared_ptr<TAC> tac);
    };
}
