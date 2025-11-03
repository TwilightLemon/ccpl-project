#pragma once
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include "abstraction/ast_nodes.hh"
#include "abstraction/array_metadata.hh"
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
        std::shared_ptr<Program> _program;
        std::shared_ptr<SYM> current_function;
        
        // Array metadata storage: maps array name to its metadata
        std::map<std::string, std::shared_ptr<ArrayMetadata>> array_metadata_map;
        
    public:
        ASTToTACGenerator();
        void generate(std::shared_ptr<Program> program);
        TACGenerator& get_tac_generator() { return tac_gen; }
        
    private:
        // Declaration generation
        void generate_declaration(std::shared_ptr<Declaration> decl);
        std::shared_ptr<TAC> generate_var_decl(std::shared_ptr<VarDecl> decl);
        void generate_func_decl(std::shared_ptr<FuncDecl> decl);
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
        std::shared_ptr<EXP> generate_member_address(std::shared_ptr<MemberAccessExpr> expr);
        std::shared_ptr<EXP> generate_address_of(std::shared_ptr<AddressOfExpr> expr);
        std::shared_ptr<EXP> generate_dereference(std::shared_ptr<DereferenceExpr> expr);
        
        // for struct / array expansion
        DATA_TYPE convert_type_to_data_type(std::shared_ptr<Type> type);

        // convert expression vector to linked list for function calls
        std::shared_ptr<EXP> expr_vector_to_list(const std::vector<std::shared_ptr<Expression>>& exprs);
        
        void expand_array_elements(std::shared_ptr<Type> array_type, const std::string& base_name,
                                  std::function<void(const std::string&, DATA_TYPE)> handler);

        //expand declared struct fields
        void expand_struct_fields(const std::string& struct_name, const std::string& base_name,
                                 std::function<void(const std::string&, DATA_TYPE)> handler);
        
        // find the first SYM of a expanded array..
        std::shared_ptr<SYM> find_array_first_element(const std::string& base_name);
        
        // Array metadata management
        std::shared_ptr<ArrayMetadata> create_array_metadata(const std::string& name, std::shared_ptr<Type> array_type);
        void record_array_metadata(const std::string& name, std::shared_ptr<Type> array_type);
        std::shared_ptr<ArrayMetadata> get_array_metadata(const std::string& name) const;
        std::shared_ptr<ArrayMetadata> infer_array_metadata_from_access(std::shared_ptr<Expression> array_expr, const std::string& fallback_name);
        
        // Generate array address without dereferencing (for assignment targets)
        std::shared_ptr<EXP> generate_array_address(std::shared_ptr<ArrayAccessExpr> expr);

    };

} // namespace twlm::ccpl::modules
