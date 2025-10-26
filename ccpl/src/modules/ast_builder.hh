#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "abstraction/ast_nodes.hh"

namespace twlm::ccpl::modules
{
    using namespace twlm::ccpl::abstraction;

    class ASTBuilder
    {
    private:
        std::shared_ptr<Program> program_root;
        std::shared_ptr<Type> current_type; // Current type being declared

    public:
        ASTBuilder();

        void init();

        // Type management
        void set_current_type(DATA_TYPE basic_type);
        std::shared_ptr<Type> get_current_type() const { return current_type; }

        // Type builders
        std::shared_ptr<Type> make_basic_type(DATA_TYPE dt);
        std::shared_ptr<Type> make_pointer_type(std::shared_ptr<Type> base);
        std::shared_ptr<Type> make_array_type(std::shared_ptr<Type> base, int size);

        // Expression builders
        std::shared_ptr<Expression> make_const_int(int value);
        std::shared_ptr<Expression> make_const_char(char value);
        std::shared_ptr<Expression> make_string_literal(const std::string &value);
        std::shared_ptr<Expression> make_identifier(const std::string &name);
        std::shared_ptr<Expression> make_binary_op(TAC_OP op,
                                                   std::shared_ptr<Expression> left,
                                                   std::shared_ptr<Expression> right);
        std::shared_ptr<Expression> make_unary_op(TAC_OP op,
                                                  std::shared_ptr<Expression> operand);
        std::shared_ptr<Expression> make_assign(std::shared_ptr<Expression> target,
                                                std::shared_ptr<Expression> value);
        std::shared_ptr<Expression> make_func_call(const std::string &func_name,
                                                   std::vector<std::shared_ptr<Expression>> arg_list);
        std::shared_ptr<Expression> make_array_access(std::shared_ptr<Expression> array,
                                                      std::shared_ptr<Expression> index);
        std::shared_ptr<Expression> make_member_access(std::shared_ptr<Expression> object,
                                                       const std::string &member,
                                                       bool is_pointer_access = false);
        std::shared_ptr<Expression> make_address_of(std::shared_ptr<Expression> operand);
        std::shared_ptr<Expression> make_dereference(std::shared_ptr<Expression> operand);

        // Statement builders
        std::shared_ptr<Statement> make_expr_stmt(std::shared_ptr<Expression> expr);
        std::shared_ptr<BlockStmt> make_block();
        std::shared_ptr<Statement> make_if_stmt(std::shared_ptr<Expression> condition,
                                                std::shared_ptr<Statement> then_branch,
                                                std::shared_ptr<Statement> else_branch = nullptr);
        std::shared_ptr<Statement> make_while_stmt(std::shared_ptr<Expression> condition,
                                                   std::shared_ptr<Statement> body);
        std::shared_ptr<Statement> make_for_stmt(std::shared_ptr<Statement> init,
                                                 std::shared_ptr<Expression> condition,
                                                 std::shared_ptr<Expression> update,
                                                 std::shared_ptr<Statement> body);
        std::shared_ptr<Statement> make_return_stmt(std::shared_ptr<Expression> return_value = nullptr);
        std::shared_ptr<Statement> make_break_stmt();
        std::shared_ptr<Statement> make_continue_stmt();
        std::shared_ptr<Statement> make_input_stmt(const std::string &var_name);
        std::shared_ptr<Statement> make_output_stmt(std::shared_ptr<Expression> expr);
        std::shared_ptr<Statement> make_switch_stmt(std::shared_ptr<Expression> condition,
                                                    std::shared_ptr<Statement> body);
        std::shared_ptr<Statement> make_case_stmt(int value);
        std::shared_ptr<Statement> make_default_stmt();

        // Declaration builders
        std::shared_ptr<VarDecl> make_var_decl(std::shared_ptr<Type> type,
                                               const std::string &name,
                                               std::shared_ptr<Expression> init = nullptr);
        std::shared_ptr<ParamDecl> make_param_decl(std::shared_ptr<Type> type,
                                                   const std::string &name);
        std::shared_ptr<FuncDecl> make_func_decl(std::shared_ptr<Type> return_type,
                                                 const std::string &name,
                                                 std::vector<std::shared_ptr<ParamDecl>> param_list,
                                                 std::shared_ptr<BlockStmt> body);
        std::shared_ptr<StructDecl> make_struct_decl(const std::string &name,
                                                     std::vector<std::shared_ptr<VarDecl>> field_list);

        // Program building
        void add_declaration(std::shared_ptr<Declaration> decl);
        void complete();
        std::shared_ptr<Program> get_program() { return program_root; }

        // Error handling
        void error(const std::string &msg);
        void warning(const std::string &msg);
    };

}
