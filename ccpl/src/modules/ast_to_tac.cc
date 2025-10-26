#include "ast_to_tac.hh"
#include <iostream>

namespace twlm::ccpl::modules
{
    ASTToTACGenerator::ASTToTACGenerator()
        : current_function(nullptr) {
        tac_gen.init();
    }

    void ASTToTACGenerator::generate(std::shared_ptr<Program> program) {
        if (!program) {
            std::cerr << "Error: Null program node" << std::endl;
            return;
        }
        
        // Generate code for all declarations and link them together
        for (auto& decl : program->declarations) {
            if (decl->kind == ASTNodeKind::VAR_DECL) {
                // Global variable declaration
                auto var_decl = std::dynamic_pointer_cast<VarDecl>(decl);
                DATA_TYPE dtype = convert_type_to_data_type(var_decl->var_type);
                auto var_tac = tac_gen.declare_var(var_decl->name, dtype);
                
                // Link to global TAC chain
                tac_gen.link_tac(var_tac);
                
            } else if (decl->kind == ASTNodeKind::FUNC_DECL) {
                // Function declaration - generates and links TAC
                // Each call to generate_func_decl will update tac_gen.tac_last
                // and the TAC will be automatically linked via the prev pointers
                auto func_decl = std::dynamic_pointer_cast<FuncDecl>(decl);
                generate_func_decl(func_decl);
            }
        }
        
        // Complete TAC generation (reverses the list and sets next pointers)
        tac_gen.complete();
    }

    // ============ Declaration Generation ============

    void ASTToTACGenerator::generate_declaration(std::shared_ptr<Declaration> decl) {
        if (!decl) return;
        
        switch (decl->kind) {
            case ASTNodeKind::VAR_DECL:
                generate_var_decl(std::dynamic_pointer_cast<VarDecl>(decl));
                break;
            case ASTNodeKind::FUNC_DECL:
                generate_func_decl(std::dynamic_pointer_cast<FuncDecl>(decl));
                break;
            case ASTNodeKind::STRUCT_DECL:
                generate_struct_decl(std::dynamic_pointer_cast<StructDecl>(decl));
                break;
            default:
                std::cerr << "Unknown declaration type" << std::endl;
                break;
        }
    }

    void ASTToTACGenerator::generate_var_decl(std::shared_ptr<VarDecl> decl) {
        if (!decl) return;
        
        DATA_TYPE dtype = convert_type_to_data_type(decl->var_type);
        auto var_tac = tac_gen.declare_var(decl->name, dtype);
        
        // For global variables, we don't generate initialization code in TAC
        // For local variables, initialization would be handled as a separate assignment statement
        // Only handle initialization if there's an init value and we're in a function context
        if (decl->init_value && current_function) {
            auto init_exp = generate_expression(decl->init_value);
            auto var_sym = tac_gen.get_var(decl->name);
            auto assign_tac = tac_gen.do_assign(var_sym, init_exp);
            // This assignment TAC will be part of the function body
        }
    }

    void ASTToTACGenerator::generate_func_decl(std::shared_ptr<FuncDecl> decl) {
        if (!decl) return;
        
        DATA_TYPE return_type = convert_type_to_data_type(decl->return_type);
        auto func_sym = tac_gen.declare_func(decl->name, return_type);
        current_function = func_sym;
        
        tac_gen.enter_scope();
        
        // Generate parameter declarations
        std::shared_ptr<TAC> param_code = nullptr;
        for (auto& param : decl->parameters) {
            DATA_TYPE param_type = convert_type_to_data_type(param->param_type);
            auto param_tac = tac_gen.declare_para(param->name, param_type);
            param_code = tac_gen.join_tac(param_code, param_tac);
        }
        
        // Generate function body
        std::shared_ptr<TAC> body_code = nullptr;
        if (decl->body) {
            body_code = generate_block_stmt(decl->body);
        }
        
        // Create function TAC - this also updates tac_gen.tac_last
        auto func_tac = tac_gen.do_func(func_sym, param_code, body_code);
        
        tac_gen.leave_scope();
        current_function = nullptr;
    }

    void ASTToTACGenerator::generate_param_decl(std::shared_ptr<ParamDecl> decl) {
        if (!decl) return;
        
        DATA_TYPE dtype = convert_type_to_data_type(decl->param_type);
        tac_gen.declare_para(decl->name, dtype);
    }

    void ASTToTACGenerator::generate_struct_decl(std::shared_ptr<StructDecl> decl) {
        if (!decl) return;
        
        // Struct declarations are not yet fully supported in TAC
        // This would require extending the TAC structure
        std::cerr << "Warning: Struct declarations are not yet fully supported in TAC generation" << std::endl;
    }

    // ============ Statement Generation ============

    std::shared_ptr<TAC> ASTToTACGenerator::generate_statement(std::shared_ptr<Statement> stmt) {
        if (!stmt) return nullptr;
        
        switch (stmt->kind) {
            case ASTNodeKind::VAR_DECL: {
                // Local variable declaration
                auto var_decl = std::dynamic_pointer_cast<VarDecl>(stmt);
                DATA_TYPE dtype = convert_type_to_data_type(var_decl->var_type);
                auto var_tac = tac_gen.declare_var(var_decl->name, dtype);
                
                // Handle initialization
                if (var_decl->init_value) {
                    auto init_exp = generate_expression(var_decl->init_value);
                    auto var_sym = tac_gen.get_var(var_decl->name);
                    auto assign_tac = tac_gen.do_assign(var_sym, init_exp);
                    return tac_gen.join_tac(var_tac, assign_tac);
                }
                return var_tac;
            }
            case ASTNodeKind::EXPR_STMT:
                return generate_expr_stmt(std::dynamic_pointer_cast<ExprStmt>(stmt));
            case ASTNodeKind::BLOCK_STMT:
                return generate_block_stmt(std::dynamic_pointer_cast<BlockStmt>(stmt));
            case ASTNodeKind::IF_STMT:
                return generate_if_stmt(std::dynamic_pointer_cast<IfStmt>(stmt));
            case ASTNodeKind::WHILE_STMT:
                return generate_while_stmt(std::dynamic_pointer_cast<WhileStmt>(stmt));
            case ASTNodeKind::FOR_STMT:
                return generate_for_stmt(std::dynamic_pointer_cast<ForStmt>(stmt));
            case ASTNodeKind::RETURN_STMT:
                return generate_return_stmt(std::dynamic_pointer_cast<ReturnStmt>(stmt));
            case ASTNodeKind::BREAK_STMT:
                return generate_break_stmt(std::dynamic_pointer_cast<BreakStmt>(stmt));
            case ASTNodeKind::CONTINUE_STMT:
                return generate_continue_stmt(std::dynamic_pointer_cast<ContinueStmt>(stmt));
            case ASTNodeKind::INPUT_STMT:
                return generate_input_stmt(std::dynamic_pointer_cast<InputStmt>(stmt));
            case ASTNodeKind::OUTPUT_STMT:
                return generate_output_stmt(std::dynamic_pointer_cast<OutputStmt>(stmt));
            case ASTNodeKind::SWITCH_STMT:
                return generate_switch_stmt(std::dynamic_pointer_cast<SwitchStmt>(stmt));
            case ASTNodeKind::CASE_STMT:
                return generate_case_stmt(std::dynamic_pointer_cast<CaseStmt>(stmt));
            case ASTNodeKind::DEFAULT_STMT:
                return generate_default_stmt(std::dynamic_pointer_cast<DefaultStmt>(stmt));
            default:
                std::cerr << "Unknown statement type" << std::endl;
                return nullptr;
        }
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_expr_stmt(std::shared_ptr<ExprStmt> stmt) {
        if (!stmt || !stmt->expression) return nullptr;
        
        auto exp = generate_expression(stmt->expression);
        return exp ? exp->code : nullptr;
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_block_stmt(std::shared_ptr<BlockStmt> stmt) {
        if (!stmt) return nullptr;
        
        std::shared_ptr<TAC> result = nullptr;
        for (auto& statement : stmt->statements) {
            auto stmt_code = generate_statement(statement);
            result = tac_gen.join_tac(result, stmt_code);
        }
        
        return result;
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_if_stmt(std::shared_ptr<IfStmt> stmt) {
        if (!stmt) return nullptr;
        
        auto cond_exp = generate_expression(stmt->condition);
        auto then_code = generate_statement(stmt->then_branch);
        
        if (stmt->else_branch) {
            auto else_code = generate_statement(stmt->else_branch);
            return tac_gen.do_if_else(cond_exp, then_code, else_code);
        } else {
            return tac_gen.do_if(cond_exp, then_code);
        }
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_while_stmt(std::shared_ptr<WhileStmt> stmt) {
        if (!stmt) return nullptr;
        
        tac_gen.begin_while_loop();
        auto cond_exp = generate_expression(stmt->condition);
        auto body_code = generate_statement(stmt->body);
        return tac_gen.end_while_loop(cond_exp, body_code);
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_for_stmt(std::shared_ptr<ForStmt> stmt) {
        if (!stmt) return nullptr;
        
        tac_gen.begin_for_loop();
        
        auto init_code = generate_statement(stmt->init);
        auto cond_exp = generate_expression(stmt->condition);
        auto update_exp = generate_expression(stmt->update);
        auto update_code = update_exp ? update_exp->code : nullptr;
        auto body_code = generate_statement(stmt->body);
        
        return tac_gen.end_for_loop(init_code, cond_exp, update_code, body_code);
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_return_stmt(std::shared_ptr<ReturnStmt> stmt) {
        if (!stmt) return nullptr;
        
        if (stmt->return_value) {
            auto ret_exp = generate_expression(stmt->return_value);
            return tac_gen.do_return(ret_exp);
        } else {
            return tac_gen.do_return(nullptr);
        }
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_break_stmt(std::shared_ptr<BreakStmt> stmt) {
        return tac_gen.do_break();
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_continue_stmt(std::shared_ptr<ContinueStmt> stmt) {
        return tac_gen.do_continue();
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_input_stmt(std::shared_ptr<InputStmt> stmt) {
        if (!stmt) return nullptr;
        
        auto var_sym = tac_gen.get_var(stmt->var_name);
        return tac_gen.do_input(var_sym);
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_output_stmt(std::shared_ptr<OutputStmt> stmt) {
        if (!stmt) return nullptr;
        
        auto exp = generate_expression(stmt->expression);
        auto output_tac = tac_gen.do_output(exp->place);
        return tac_gen.join_tac(exp->code, output_tac);
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_switch_stmt(std::shared_ptr<SwitchStmt> stmt) {
        if (!stmt) return nullptr;
        
        tac_gen.begin_switch();
        auto cond_exp = generate_expression(stmt->condition);
        auto body_code = generate_statement(stmt->body);
        return tac_gen.end_switch(cond_exp, body_code);
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_case_stmt(std::shared_ptr<CaseStmt> stmt) {
        if (!stmt) return nullptr;
        
        return tac_gen.do_case(stmt->value);
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_default_stmt(std::shared_ptr<DefaultStmt> stmt) {
        return tac_gen.do_default();
    }

    // ============ Expression Generation ============

    std::shared_ptr<EXP> ASTToTACGenerator::generate_expression(std::shared_ptr<Expression> expr) {
        if (!expr) return nullptr;
        
        switch (expr->kind) {
            case ASTNodeKind::CONST_INT:
                return generate_const_int(std::dynamic_pointer_cast<ConstIntExpr>(expr));
            case ASTNodeKind::CONST_CHAR:
                return generate_const_char(std::dynamic_pointer_cast<ConstCharExpr>(expr));
            case ASTNodeKind::STRING_LITERAL:
                return generate_string_literal(std::dynamic_pointer_cast<StringLiteralExpr>(expr));
            case ASTNodeKind::IDENTIFIER:
                return generate_identifier(std::dynamic_pointer_cast<IdentifierExpr>(expr));
            case ASTNodeKind::BINARY_OP:
                return generate_binary_op(std::dynamic_pointer_cast<BinaryOpExpr>(expr));
            case ASTNodeKind::UNARY_OP:
                return generate_unary_op(std::dynamic_pointer_cast<UnaryOpExpr>(expr));
            case ASTNodeKind::ASSIGN:
                return generate_assign(std::dynamic_pointer_cast<AssignExpr>(expr));
            case ASTNodeKind::FUNC_CALL:
                return generate_func_call(std::dynamic_pointer_cast<FuncCallExpr>(expr));
            case ASTNodeKind::ARRAY_ACCESS:
                return generate_array_access(std::dynamic_pointer_cast<ArrayAccessExpr>(expr));
            case ASTNodeKind::MEMBER_ACCESS:
                return generate_member_access(std::dynamic_pointer_cast<MemberAccessExpr>(expr));
            case ASTNodeKind::ADDRESS_OF:
                return generate_address_of(std::dynamic_pointer_cast<AddressOfExpr>(expr));
            case ASTNodeKind::DEREFERENCE:
                return generate_dereference(std::dynamic_pointer_cast<DereferenceExpr>(expr));
            default:
                std::cerr << "Unknown expression type" << std::endl;
                return nullptr;
        }
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_const_int(std::shared_ptr<ConstIntExpr> expr) {
        if (!expr) return nullptr;
        
        auto sym = tac_gen.mk_const(expr->value);
        auto exp = tac_gen.mk_exp(sym, nullptr);
        exp->data_type = DATA_TYPE::INT;
        return exp;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_const_char(std::shared_ptr<ConstCharExpr> expr) {
        if (!expr) return nullptr;
        
        auto sym = tac_gen.mk_const_char(expr->value);
        auto exp = tac_gen.mk_exp(sym, nullptr);
        exp->data_type = DATA_TYPE::CHAR;
        return exp;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_string_literal(std::shared_ptr<StringLiteralExpr> expr) {
        if (!expr) return nullptr;
        
        auto sym = tac_gen.mk_text(expr->value);
        auto exp = tac_gen.mk_exp(sym, nullptr);
        exp->data_type = DATA_TYPE::CHAR;  // String is char*
        return exp;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_identifier(std::shared_ptr<IdentifierExpr> expr) {
        if (!expr) return nullptr;
        
        auto var = tac_gen.get_var(expr->name);
        auto exp = tac_gen.mk_exp(var, nullptr);
        if (var) {
            exp->data_type = var->data_type;
        }
        return exp;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_binary_op(std::shared_ptr<BinaryOpExpr> expr) {
        if (!expr) return nullptr;
        
        auto left_exp = generate_expression(expr->left);
        auto right_exp = generate_expression(expr->right);
        return tac_gen.do_bin(expr->op, left_exp, right_exp);
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_unary_op(std::shared_ptr<UnaryOpExpr> expr) {
        if (!expr) return nullptr;
        
        auto operand_exp = generate_expression(expr->operand);
        return tac_gen.do_un(expr->op, operand_exp);
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_assign(std::shared_ptr<AssignExpr> expr) {
        if (!expr) return nullptr;
        
        // For simple identifier assignment
        if (expr->target->kind == ASTNodeKind::IDENTIFIER) {
            auto id_expr = std::dynamic_pointer_cast<IdentifierExpr>(expr->target);
            auto var = tac_gen.get_var(id_expr->name);
            auto value_exp = generate_expression(expr->value);
            auto assign_tac = tac_gen.do_assign(var, value_exp);
            return tac_gen.mk_exp(var, assign_tac);
        }
        
        // For more complex targets (array access, dereference, etc.)
        // This would require extending TAC to support these operations
        std::cerr << "Warning: Complex assignment targets not yet fully supported" << std::endl;
        return nullptr;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_func_call(std::shared_ptr<FuncCallExpr> expr) {
        if (!expr) return nullptr;
        
        // Convert argument vector to linked list
        auto arg_list = expr_vector_to_list(expr->arguments);
        return tac_gen.do_call_ret(expr->func_name, arg_list);
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_array_access(std::shared_ptr<ArrayAccessExpr> expr) {
        if (!expr) return nullptr;
        
        // Array access would require extending TAC with address arithmetic
        // For now, we'll emit a warning
        std::cerr << "Warning: Array access not yet fully supported in TAC generation" << std::endl;
        return nullptr;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_member_access(std::shared_ptr<MemberAccessExpr> expr) {
        if (!expr) return nullptr;
        
        // Member access would require extending TAC with struct support
        std::cerr << "Warning: Member access not yet fully supported in TAC generation" << std::endl;
        return nullptr;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_address_of(std::shared_ptr<AddressOfExpr> expr) {
        if (!expr) return nullptr;
        
        // Address-of would require extending TAC with pointer operations
        std::cerr << "Warning: Address-of operator not yet fully supported in TAC generation" << std::endl;
        return nullptr;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_dereference(std::shared_ptr<DereferenceExpr> expr) {
        if (!expr) return nullptr;
        
        // Dereference would require extending TAC with pointer operations
        std::cerr << "Warning: Dereference operator not yet fully supported in TAC generation" << std::endl;
        return nullptr;
    }

    // ============ Helper Functions ============

    DATA_TYPE ASTToTACGenerator::convert_type_to_data_type(std::shared_ptr<Type> type) {
        if (!type) return DATA_TYPE::UNDEF;
        
        if (type->kind == TypeKind::BASIC) {
            return type->basic_type;
        }
        
        // For pointer, array, function types, we currently map them to basic types
        // This is a simplification and would need to be extended for full support
        if (type->kind == TypeKind::POINTER || type->kind == TypeKind::ARRAY) {
            // Return the base type for now
            return convert_type_to_data_type(type->base_type);
        }
        
        if (type->kind == TypeKind::FUNCTION) {
            return convert_type_to_data_type(type->return_type);
        }
        
        return DATA_TYPE::UNDEF;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::expr_vector_to_list(
        const std::vector<std::shared_ptr<Expression>>& exprs) {
        
        if (exprs.empty()) return nullptr;
        
        // Build the linked list in reverse order
        std::shared_ptr<EXP> result = nullptr;
        for (auto it = exprs.rbegin(); it != exprs.rend(); ++it) {
            auto exp = generate_expression(*it);
            if (exp) {
                exp->next = result;
                result = exp;
            }
        }
        
        return result;
    }

} // namespace twlm::ccpl::modules
