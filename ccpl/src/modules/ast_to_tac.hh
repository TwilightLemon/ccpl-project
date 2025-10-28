#pragma once
#include <memory>
#include <vector>
#include <string>
#include "abstraction/ast_nodes.hh"
#include "tac.hh"

namespace twlm::ccpl::modules
{
    using namespace twlm::ccpl::abstraction;

    /**
     * AST to TAC Generator
     */
    class ASTToTACGenerator
    {
    private:
        TACGenerator tac_gen;
        
        // Current context
        std::shared_ptr<SYM> current_function;
        
    public:
        ASTToTACGenerator();
        
        // Main entry point
        void generate(std::shared_ptr<Program> program);
        
        // Get the generated TAC
        TACGenerator& get_tac_generator() { return tac_gen; }
        
    private:
        // Declaration generation
        void generate_declaration(std::shared_ptr<Declaration> decl);
        void generate_var_decl(std::shared_ptr<VarDecl> decl);
        void generate_func_decl(std::shared_ptr<FuncDecl> decl);
        void generate_param_decl(std::shared_ptr<ParamDecl> decl);
        void generate_struct_decl(std::shared_ptr<StructDecl> decl);
        
        // Statement generation
        std::shared_ptr<TAC> generate_statement(std::shared_ptr<Statement> stmt);
        std::shared_ptr<TAC> generate_expr_stmt(std::shared_ptr<ExprStmt> stmt);
        std::shared_ptr<TAC> generate_block_stmt(std::shared_ptr<BlockStmt> stmt);
        std::shared_ptr<TAC> generate_if_stmt(std::shared_ptr<IfStmt> stmt);
        std::shared_ptr<TAC> generate_while_stmt(std::shared_ptr<WhileStmt> stmt);
        std::shared_ptr<TAC> generate_for_stmt(std::shared_ptr<ForStmt> stmt);
        std::shared_ptr<TAC> generate_return_stmt(std::shared_ptr<ReturnStmt> stmt);
        std::shared_ptr<TAC> generate_break_stmt(std::shared_ptr<BreakStmt> stmt);
        std::shared_ptr<TAC> generate_continue_stmt(std::shared_ptr<ContinueStmt> stmt);
        std::shared_ptr<TAC> generate_input_stmt(std::shared_ptr<InputStmt> stmt);
        std::shared_ptr<TAC> generate_output_stmt(std::shared_ptr<OutputStmt> stmt);
        std::shared_ptr<TAC> generate_switch_stmt(std::shared_ptr<SwitchStmt> stmt);
        std::shared_ptr<TAC> generate_case_stmt(std::shared_ptr<CaseStmt> stmt);
        std::shared_ptr<TAC> generate_default_stmt(std::shared_ptr<DefaultStmt> stmt);
        
        // Expression generation (returns EXP structure with code and place)
        std::shared_ptr<EXP> generate_expression(std::shared_ptr<Expression> expr);
        std::shared_ptr<EXP> generate_const_int(std::shared_ptr<ConstIntExpr> expr);
        std::shared_ptr<EXP> generate_const_char(std::shared_ptr<ConstCharExpr> expr);
        std::shared_ptr<EXP> generate_string_literal(std::shared_ptr<StringLiteralExpr> expr);
        std::shared_ptr<EXP> generate_identifier(std::shared_ptr<IdentifierExpr> expr);
        std::shared_ptr<EXP> generate_binary_op(std::shared_ptr<BinaryOpExpr> expr);
        std::shared_ptr<EXP> generate_unary_op(std::shared_ptr<UnaryOpExpr> expr);
        std::shared_ptr<EXP> generate_assign(std::shared_ptr<AssignExpr> expr);
        std::shared_ptr<EXP> generate_func_call(std::shared_ptr<FuncCallExpr> expr);
        std::shared_ptr<EXP> generate_array_access(std::shared_ptr<ArrayAccessExpr> expr);
        std::shared_ptr<EXP> generate_member_access(std::shared_ptr<MemberAccessExpr> expr);
        std::shared_ptr<EXP> generate_address_of(std::shared_ptr<AddressOfExpr> expr);
        std::shared_ptr<EXP> generate_dereference(std::shared_ptr<DereferenceExpr> expr);
        
        // Type conversion helper
        DATA_TYPE convert_type_to_data_type(std::shared_ptr<Type> type);
        
        // Helper to convert expression vector to linked list for function calls
        std::shared_ptr<EXP> expr_vector_to_list(const std::vector<std::shared_ptr<Expression>>& exprs);

        void extract_struct_fields(const std::shared_ptr<VarDecl> &field,std::vector<std::pair<std::string, DATA_TYPE>> &fields);
    };

} // namespace twlm::ccpl::modules
