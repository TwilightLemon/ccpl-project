#include "ast_builder.hh"
#include <iostream>

namespace twlm::ccpl::modules
{
    ASTBuilder::ASTBuilder() 
        : program_root(std::make_shared<Program>()),
          current_type(nullptr) {}

    void ASTBuilder::init() {
        program_root = std::make_shared<Program>();
        current_type = nullptr;
    }

    void ASTBuilder::set_current_type(DATA_TYPE basic_type) {
        current_type = Type::make_basic(basic_type);
    }

    std::shared_ptr<Type> ASTBuilder::make_basic_type(DATA_TYPE dt) {
        return Type::make_basic(dt);
    }

    std::shared_ptr<Type> ASTBuilder::make_pointer_type(std::shared_ptr<Type> base) {
        return Type::make_pointer(base);
    }

    std::shared_ptr<Type> ASTBuilder::make_array_type(std::shared_ptr<Type> base, int size) {
        return Type::make_array(base, size);
    }

    // ============ Expression Builders ============

    std::shared_ptr<Expression> ASTBuilder::make_const_int(int value) {
        return std::make_shared<ConstIntExpr>(value);
    }

    std::shared_ptr<Expression> ASTBuilder::make_const_char(char value) {
        return std::make_shared<ConstCharExpr>(value);
    }

    std::shared_ptr<Expression> ASTBuilder::make_string_literal(const std::string& value) {
        return std::make_shared<StringLiteralExpr>(value);
    }

    std::shared_ptr<Expression> ASTBuilder::make_identifier(const std::string& name) {
        return std::make_shared<IdentifierExpr>(name);
    }

    std::shared_ptr<Expression> ASTBuilder::make_binary_op(
        TAC_OP op,
        std::shared_ptr<Expression> left,
        std::shared_ptr<Expression> right) {
        return std::make_shared<BinaryOpExpr>(op, left, right);
    }

    std::shared_ptr<Expression> ASTBuilder::make_unary_op(
        TAC_OP op,
        std::shared_ptr<Expression> operand) {
        return std::make_shared<UnaryOpExpr>(op, operand);
    }

    std::shared_ptr<Expression> ASTBuilder::make_assign(
        std::shared_ptr<Expression> target,
        std::shared_ptr<Expression> value) {
        return std::make_shared<AssignExpr>(target, value);
    }

    std::shared_ptr<Expression> ASTBuilder::make_func_call(
        const std::string& func_name,
        std::shared_ptr<Expression> arg_list) {
        auto func_call = std::make_shared<FuncCallExpr>(func_name);
        func_call->arguments = expr_list_to_vector(arg_list);
        return func_call;
    }

    std::shared_ptr<Expression> ASTBuilder::make_array_access(
        std::shared_ptr<Expression> array,
        std::shared_ptr<Expression> index) {
        return std::make_shared<ArrayAccessExpr>(array, index);
    }

    std::shared_ptr<Expression> ASTBuilder::make_member_access(
        std::shared_ptr<Expression> object,
        const std::string& member,
        bool is_pointer_access) {
        return std::make_shared<MemberAccessExpr>(object, member, is_pointer_access);
    }

    std::shared_ptr<Expression> ASTBuilder::make_address_of(
        std::shared_ptr<Expression> operand) {
        return std::make_shared<AddressOfExpr>(operand);
    }

    std::shared_ptr<Expression> ASTBuilder::make_dereference(
        std::shared_ptr<Expression> operand) {
        return std::make_shared<DereferenceExpr>(operand);
    }

    std::vector<std::shared_ptr<Expression>> ASTBuilder::expr_list_to_vector(
        std::shared_ptr<Expression> expr_list) {
        std::vector<std::shared_ptr<Expression>> result;
        if (!expr_list) return result;
        
        // The expression list is built in reverse order (next points to previous)
        // We need to reverse it
        std::vector<std::shared_ptr<Expression>> temp;
        auto current = expr_list;
        while (current) {
            temp.push_back(current);
            if (current->kind == ASTNodeKind::BINARY_OP ||
                current->kind == ASTNodeKind::UNARY_OP ||
                current->kind == ASTNodeKind::ASSIGN ||
                current->kind == ASTNodeKind::FUNC_CALL) {
                // These nodes don't have a 'next' field in the same way
                // We need to check if Expression has a next field
                // Looking at the EXP structure in tac_struct.hh, we see it has a 'next' field
                // But our new Expression doesn't have this
                // We need to handle argument lists differently
                break;
            }
            // For now, we'll just add the single expression
            break;
        }
        
        // Reverse to get correct order
        for (auto it = temp.rbegin(); it != temp.rend(); ++it) {
            result.push_back(*it);
        }
        
        return result;
    }

    // ============ Statement Builders ============

    std::shared_ptr<Statement> ASTBuilder::make_expr_stmt(
        std::shared_ptr<Expression> expr) {
        return std::make_shared<ExprStmt>(expr);
    }

    std::shared_ptr<BlockStmt> ASTBuilder::make_block() {
        return std::make_shared<BlockStmt>();
    }

    std::shared_ptr<BlockStmt> ASTBuilder::make_block_from_list(
        std::shared_ptr<Statement> stmt_list) {
        auto block = std::make_shared<BlockStmt>();
        
        // Collect all statements from the linked list
        std::vector<std::shared_ptr<Statement>> stmts;
        auto current = stmt_list;
        while (current) {
            stmts.push_back(current);
            // We need a way to track the next statement in the list
            // For now, we'll just add the single statement
            break;
        }
        
        block->statements = stmts;
        return block;
    }

    std::shared_ptr<Statement> ASTBuilder::make_if_stmt(
        std::shared_ptr<Expression> condition,
        std::shared_ptr<Statement> then_branch,
        std::shared_ptr<Statement> else_branch) {
        return std::make_shared<IfStmt>(condition, then_branch, else_branch);
    }

    std::shared_ptr<Statement> ASTBuilder::make_while_stmt(
        std::shared_ptr<Expression> condition,
        std::shared_ptr<Statement> body) {
        return std::make_shared<WhileStmt>(condition, body);
    }

    std::shared_ptr<Statement> ASTBuilder::make_for_stmt(
        std::shared_ptr<Statement> init,
        std::shared_ptr<Expression> condition,
        std::shared_ptr<Expression> update,
        std::shared_ptr<Statement> body) {
        return std::make_shared<ForStmt>(init, condition, update, body);
    }

    std::shared_ptr<Statement> ASTBuilder::make_return_stmt(
        std::shared_ptr<Expression> return_value) {
        return std::make_shared<ReturnStmt>(return_value);
    }

    std::shared_ptr<Statement> ASTBuilder::make_break_stmt() {
        return std::make_shared<BreakStmt>();
    }

    std::shared_ptr<Statement> ASTBuilder::make_continue_stmt() {
        return std::make_shared<ContinueStmt>();
    }

    std::shared_ptr<Statement> ASTBuilder::make_input_stmt(const std::string& var_name) {
        return std::make_shared<InputStmt>(var_name);
    }

    std::shared_ptr<Statement> ASTBuilder::make_output_stmt(
        std::shared_ptr<Expression> expr) {
        return std::make_shared<OutputStmt>(expr);
    }

    std::shared_ptr<Statement> ASTBuilder::make_switch_stmt(
        std::shared_ptr<Expression> condition,
        std::shared_ptr<Statement> body) {
        return std::make_shared<SwitchStmt>(condition, body);
    }

    std::shared_ptr<Statement> ASTBuilder::make_case_stmt(int value) {
        return std::make_shared<CaseStmt>(value);
    }

    std::shared_ptr<Statement> ASTBuilder::make_default_stmt() {
        return std::make_shared<DefaultStmt>();
    }

    // ============ Declaration Builders ============

    std::shared_ptr<VarDecl> ASTBuilder::make_var_decl(
        std::shared_ptr<Type> type,
        const std::string& name,
        std::shared_ptr<Expression> init) {
        return std::make_shared<VarDecl>(type, name, init);
    }

    std::shared_ptr<ParamDecl> ASTBuilder::make_param_decl(
        std::shared_ptr<Type> type,
        const std::string& name) {
        return std::make_shared<ParamDecl>(type, name);
    }

    std::shared_ptr<FuncDecl> ASTBuilder::make_func_decl(
        std::shared_ptr<Type> return_type,
        const std::string& name,
        std::shared_ptr<ParamDecl> param_list,
        std::shared_ptr<BlockStmt> body) {
        auto func_decl = std::make_shared<FuncDecl>(
            return_type, name, param_list_to_vector(param_list), body);
        return func_decl;
    }

    std::shared_ptr<StructDecl> ASTBuilder::make_struct_decl(
        const std::string& name,
        std::shared_ptr<VarDecl> field_list) {
        auto struct_decl = std::make_shared<StructDecl>(name);
        struct_decl->fields = var_decl_list_to_vector(field_list);
        return struct_decl;
    }

    // ============ Helper Functions ============

    std::shared_ptr<Statement> ASTBuilder::join_statements(
        std::shared_ptr<Statement> s1,
        std::shared_ptr<Statement> s2) {
        // For AST, we'll handle this differently
        // We'll create a temporary list structure
        // The parser will need to collect these into a vector for BlockStmt
        // For now, just return s2 and we'll handle aggregation in the parser
        return s2;
    }

    std::vector<std::shared_ptr<ParamDecl>> ASTBuilder::param_list_to_vector(
        std::shared_ptr<ParamDecl> param_list) {
        std::vector<std::shared_ptr<ParamDecl>> result;
        // Since we're building a new structure, parameters will be added directly
        // to a vector in the parser. This function is a placeholder.
        if (param_list) {
            result.push_back(param_list);
        }
        return result;
    }

    std::vector<std::shared_ptr<VarDecl>> ASTBuilder::var_decl_list_to_vector(
        std::shared_ptr<VarDecl> decl_list) {
        std::vector<std::shared_ptr<VarDecl>> result;
        if (decl_list) {
            result.push_back(decl_list);
        }
        return result;
    }

    void ASTBuilder::add_declaration(std::shared_ptr<Declaration> decl) {
        if (program_root && decl) {
            program_root->declarations.push_back(decl);
        }
    }

    void ASTBuilder::complete() {
        // Nothing special needed for AST completion
        std::clog << "AST building completed successfully." << std::endl;
    }

    void ASTBuilder::error(const std::string& msg) {
        std::cerr << "AST Builder Error: " << msg << std::endl;
    }

    void ASTBuilder::warning(const std::string& msg) {
        std::cerr << "AST Builder Warning: " << msg << std::endl;
    }

} // namespace twlm::ccpl::modules
