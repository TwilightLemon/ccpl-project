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
            if (decl->kind == ASTNodeKind::STRUCT_DECL) {
                // Struct type declaration - must be processed first
                auto struct_decl = std::dynamic_pointer_cast<StructDecl>(decl);
                generate_struct_decl(struct_decl);
                
            } else if (decl->kind == ASTNodeKind::VAR_DECL) {
                // Global variable declaration
                auto var_decl = std::dynamic_pointer_cast<VarDecl>(decl);
                
                if (var_decl->var_type && var_decl->var_type->kind == TypeKind::ARRAY) {
                    expand_array_elements(var_decl->var_type, var_decl->name,
                        [this](const std::string& name, DATA_TYPE dtype) {
                            tac_gen.declare_var(name, dtype);
                        });
                }
                else if (var_decl->var_type && var_decl->var_type->kind == TypeKind::STRUCT) {
                    auto struct_type = tac_gen.get_struct_type(var_decl->var_type->struct_name);
                    if (struct_type) {
                        for (const auto& field : struct_type->struct_fields) {
                            std::string field_var_name = var_decl->name + "." + std::get<0>(field);
                            DATA_TYPE field_type = std::get<1>(field);
                            auto field_var_tac = tac_gen.declare_var(field_var_name, field_type);
                            
                            tac_gen.link_tac(field_var_tac);
                        }
                    }
                } else {
                    DATA_TYPE dtype = convert_type_to_data_type(var_decl->var_type);
                    auto var_tac = tac_gen.declare_var(var_decl->name, dtype);
                    
                    tac_gen.link_tac(var_tac);
                }
                
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
        
        if (decl->var_type && decl->var_type->kind == TypeKind::ARRAY) {
            // For array declaration, expand array elements for storage
            // Array name will decay to pointer automatically when used
            expand_array_elements(decl->var_type, decl->name,
                [this](const std::string& name, DATA_TYPE dtype) {
                    tac_gen.declare_var(name, dtype);
                });
            return;
        }
        
        if (decl->var_type && decl->var_type->kind == TypeKind::STRUCT) {
            auto struct_type = tac_gen.get_struct_type(decl->var_type->struct_name);
            if (!struct_type) {
                std::cerr << "Error: Unknown struct type: " << decl->var_type->struct_name << std::endl;
                return;
            }
            
            for (const auto& field : struct_type->struct_fields) {
                std::string field_var_name = decl->name + "." + std::get<0>(field);
                DATA_TYPE field_type = std::get<1>(field);
                auto field_var_tac = tac_gen.declare_var(field_var_name, field_type);
            }
            return;
        }
        
        DATA_TYPE dtype = convert_type_to_data_type(decl->var_type);
        auto var_tac = tac_gen.declare_var(decl->name, dtype);
        
        if (decl->init_value && current_function) {
            auto init_exp = generate_expression(decl->init_value);
            auto var_sym = tac_gen.get_var(decl->name);
            auto assign_tac = tac_gen.do_assign(var_sym, init_exp);
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
            // Check if parameter is a struct type
            if (param->param_type && param->param_type->kind == TypeKind::STRUCT) {
                // Get the struct type definition
                auto struct_type = tac_gen.get_struct_type(param->param_type->struct_name);
                if (struct_type) {
                    // Expand the struct parameter into individual field parameters
                    for (const auto& field : struct_type->struct_fields) {
                        std::string field_param_name = param->name + "." + std::get<0>(field);
                        DATA_TYPE field_type = std::get<1>(field);
                        auto field_param_tac = tac_gen.declare_para(field_param_name, field_type);
                        param_code = tac_gen.join_tac(param_code, field_param_tac);
                    }
                }
            } else {
                // Normal parameter
                DATA_TYPE param_type = convert_type_to_data_type(param->param_type);
                auto param_tac = tac_gen.declare_para(param->name, param_type);
                param_code = tac_gen.join_tac(param_code, param_tac);
            }
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

    void ASTToTACGenerator::extract_struct_fields(const std::shared_ptr<VarDecl> &field,std::vector<std::pair<std::string, DATA_TYPE>> &fields){
        if (field->var_type && field->var_type->kind == TypeKind::ARRAY) {
            expand_array_elements(field->var_type, field->name,
                [&fields](const std::string& name, DATA_TYPE dtype) {
                    fields.push_back({name, dtype});
                });
            return;
        }
        
        if(field->var_type && field->var_type->kind==TypeKind::STRUCT){
            auto struct_type = tac_gen.get_struct_type(field->var_type->struct_name);
            if(struct_type){
                for(const auto &sub_field:struct_type->struct_fields){
                    std::string nested_field_name=field->name+"."+std::get<0>(sub_field);
                    DATA_TYPE nested_field_type=std::get<1>(sub_field);
                    fields.push_back({nested_field_name,nested_field_type});
                }
            }else{
                throw std::runtime_error("Unknown struct type: "+field->var_type->struct_name);
            }
        }
    }

    void ASTToTACGenerator::expand_array_elements(std::shared_ptr<Type> array_type, 
                                                   const std::string& base_name,
                                                   std::function<void(const std::string&, DATA_TYPE)> handler) {
        if (!array_type || array_type->kind != TypeKind::ARRAY) return;
        
        int array_size = array_type->array_size;
        auto element_type = array_type->base_type;
        
        for (int i = 0; i < array_size; ++i) {
            std::string elem_name = base_name + "[" + std::to_string(i) + "]";
            
            if (element_type->kind == TypeKind::ARRAY) {
                expand_array_elements(element_type, elem_name, handler);
            } else if (element_type->kind == TypeKind::STRUCT) {
                auto struct_type = tac_gen.get_struct_type(element_type->struct_name);
                if (struct_type) {
                    for (const auto& field : struct_type->struct_fields) {
                        std::string field_name = elem_name + "." + std::get<0>(field);
                        DATA_TYPE field_type = std::get<1>(field);
                        handler(field_name, field_type);
                    }
                }
            } else {
                DATA_TYPE dtype = convert_type_to_data_type(element_type);
                handler(elem_name, dtype);
            }
        }
    }

    void ASTToTACGenerator::generate_struct_decl(std::shared_ptr<StructDecl> decl) {
        if (!decl) return;
        
        std::vector<std::pair<std::string, DATA_TYPE>> fields;
        for (const auto& field : decl->fields) {
            if (field->var_type && field->var_type->kind == TypeKind::ARRAY) {
                expand_array_elements(field->var_type, field->name,
                    [&fields](const std::string& name, DATA_TYPE dtype) {
                        fields.push_back({name, dtype});
                    });
                continue;
            }
            
            DATA_TYPE field_type = convert_type_to_data_type(field->var_type);
            if(field_type==DATA_TYPE::STRUCT){
                extract_struct_fields(field, fields);
                continue;
            }
            fields.push_back({field->name, field_type});
        }
        
        auto struct_type = tac_gen.declare_struct_type(decl->name, fields);
    }

    // ============ Statement Generation ============

    std::shared_ptr<TAC> ASTToTACGenerator::generate_statement(std::shared_ptr<Statement> stmt) {
        if (!stmt) return nullptr;
        
        switch (stmt->kind) {
            case ASTNodeKind::VAR_DECL: {
                auto var_decl = std::dynamic_pointer_cast<VarDecl>(stmt);
                
                if (var_decl->var_type && var_decl->var_type->kind == TypeKind::ARRAY) {
                    std::shared_ptr<TAC> result_tac = nullptr;
                    
                    // Expand and declare array elements
                    expand_array_elements(var_decl->var_type, var_decl->name,
                        [this, &result_tac](const std::string& name, DATA_TYPE dtype) {
                            auto elem_tac = tac_gen.declare_var(name, dtype);
                            result_tac = tac_gen.join_tac(result_tac, elem_tac);
                        });
                    
                    // Note: Array name will decay to pointer (&arr[0]) automatically
                    // when used in expressions, so we don't need to create a separate variable
                    
                    return result_tac;
                }
                else if (var_decl->var_type && var_decl->var_type->kind == TypeKind::STRUCT) {
                    auto struct_type = tac_gen.get_struct_type(var_decl->var_type->struct_name);
                    if (struct_type) {
                        std::shared_ptr<TAC> result_tac = nullptr;
                        for (const auto& field : struct_type->struct_fields) {
                            std::string field_var_name = var_decl->name + "." + std::get<0>(field);
                            DATA_TYPE field_type = std::get<1>(field);
                            auto field_var_tac = tac_gen.declare_var(field_var_name, field_type);
                            result_tac = tac_gen.join_tac(result_tac, field_var_tac);
                        }
                        
                        if (var_decl->init_value) {
                            std::cerr << "Warning: Struct initialization not yet supported" << std::endl;
                        }
                        
                        return result_tac;
                    }
                } else {
                    DATA_TYPE dtype = convert_type_to_data_type(var_decl->var_type);
                    auto var_tac = tac_gen.declare_var(var_decl->name, dtype);
                    
                    if (var_decl->init_value) {
                        auto init_exp = generate_expression(var_decl->init_value);
                        auto var_sym = tac_gen.get_var(var_decl->name);
                        auto assign_tac = tac_gen.do_assign(var_sym, init_exp);
                        return tac_gen.join_tac(var_tac, assign_tac);
                    }
                    return var_tac;
                }
                return nullptr;
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
            // Check if returning a struct (identifier that doesn't exist as a single variable)
            // In TAC, struct returns are not supported since TAC is type-agnostic
            if (stmt->return_value->kind == ASTNodeKind::IDENTIFIER) {
                auto id_expr = std::dynamic_pointer_cast<IdentifierExpr>(stmt->return_value);
                if (id_expr) {
                    auto var = tac_gen.get_var(id_expr->name);
                    if (!var) {
                        // This might be a struct variable (which is expanded into fields)
                        // For now, just return without a value (struct return not supported in TAC)
                        std::cerr << "Warning: Returning struct value '" << id_expr->name 
                                  << "' - TAC doesn't support struct returns, returning void" << std::endl;
                        return tac_gen.do_return(nullptr);
                    }
                }
            }
            
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
        
        // If variable is found, just return it
        if (var) {
            auto exp = tac_gen.mk_exp(var, nullptr);
            exp->data_type = var->data_type;
            return exp;
        }
        
        // If variable is not found, check if it's an array (try name[0])
        auto first_elem = tac_gen.get_var(expr->name + "[0]");
        if (first_elem) {
            // This identifier refers to an array
            // Generate &arr[0] to get the address of the first element
            auto first_elem_exp = tac_gen.mk_exp(first_elem, nullptr);
            auto addr_result = tac_gen.do_address_of(first_elem_exp);
            return addr_result;
        }
        
        std::cerr << "Error: Variable not found: " << expr->name << std::endl;
        return nullptr;
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
            
            // Special case: assigning a string literal to a char array
            if (expr->value->kind == ASTNodeKind::STRING_LITERAL) {
                auto string_literal = std::dynamic_pointer_cast<StringLiteralExpr>(expr->value);
                
                // Check if the target is an array by checking if name[0] exists
                auto first_elem = tac_gen.get_var(id_expr->name + "[0]");
                if (first_elem) {
                    // This is an array assignment
                    std::shared_ptr<TAC> assign_tac = nullptr;
                    std::shared_ptr<SYM> last_var = nullptr;
                    
                    // Assign each character to the corresponding array element
                    for (size_t i = 0; i < string_literal->value.length(); ++i) {
                        std::string elem_name = id_expr->name + "[" + std::to_string(i) + "]";
                        auto elem_var = tac_gen.get_var(elem_name);
                        if (!elem_var) {
                            std::cerr << "Error: Array element not found: " << elem_name << std::endl;
                            break;
                        }
                        
                        // Create a constant for this character
                        auto char_sym = tac_gen.mk_const_char(string_literal->value[i]);
                        auto char_exp = tac_gen.mk_exp(char_sym, nullptr);
                        
                        // Generate assignment TAC
                        auto elem_assign = tac_gen.do_assign(elem_var, char_exp);
                        assign_tac = tac_gen.join_tac(assign_tac, elem_assign);
                        last_var = elem_var;
                    }
                    
                    // Add null terminator if there's space
                    std::string null_elem_name = id_expr->name + "[" + std::to_string(string_literal->value.length()) + "]";
                    auto null_elem_var = tac_gen.get_var(null_elem_name);
                    if (null_elem_var) {
                        auto null_sym = tac_gen.mk_const_char('\0');
                        auto null_exp = tac_gen.mk_exp(null_sym, nullptr);
                        auto null_assign = tac_gen.do_assign(null_elem_var, null_exp);
                        assign_tac = tac_gen.join_tac(assign_tac, null_assign);
                        last_var = null_elem_var;
                    }
                    
                    // Return expression with the last assigned variable
                    return tac_gen.mk_exp(last_var, assign_tac);
                }
            }
            
            // Normal identifier assignment (not array)
            auto var = tac_gen.get_var(id_expr->name);
            
            // Check if the value is an array name (identifier that should be treated as pointer)
            std::shared_ptr<EXP> value_exp;
            if (expr->value->kind == ASTNodeKind::IDENTIFIER) {
                auto value_id = std::dynamic_pointer_cast<IdentifierExpr>(expr->value);
                // Check if this identifier is an array by seeing if name[0] exists
                auto first_elem = tac_gen.get_var(value_id->name + "[0]");
                if (first_elem) {
                    // This is an array name, convert to &array[0]
                    // This implements the array-to-pointer decay
                    auto addr_exp = tac_gen.mk_exp(first_elem, nullptr);
                    value_exp = tac_gen.do_address_of(addr_exp);
                } else {
                    value_exp = generate_expression(expr->value);
                }
            } else {
                value_exp = generate_expression(expr->value);
            }
            
            auto assign_tac = tac_gen.do_assign(var, value_exp);
            return tac_gen.mk_exp(var, assign_tac);
        }
        
        if (expr->target->kind == ASTNodeKind::MEMBER_ACCESS) {
            auto member_expr = std::dynamic_pointer_cast<MemberAccessExpr>(expr->target);
            std::string field_var_name=member_expr->to_string();
            
            auto value_exp = generate_expression(expr->value);
            if (!value_exp) {
                return nullptr;
            }
            
            auto field_var = tac_gen.get_var(field_var_name);
            if (!field_var) {
                std::cerr << "Error: Field variable not found: " << field_var_name << std::endl;
                return nullptr;
            }
            
            // Generate assignment: field_var = value
            auto assign_tac = tac_gen.do_assign(field_var, value_exp);
            
            // Return expression with the field variable as result
            return tac_gen.mk_exp(field_var, assign_tac);
        }
        
        // For array access assignment
        if (expr->target->kind == ASTNodeKind::ARRAY_ACCESS) {
            // Generate the target array access expression to get the variable
            auto target_exp = generate_array_access(std::dynamic_pointer_cast<ArrayAccessExpr>(expr->target));
            if (!target_exp || !target_exp->place) {
                std::cerr << "Error: Failed to generate array access target" << std::endl;
                return nullptr;
            }
            
            // Generate the value expression
            auto value_exp = generate_expression(expr->value);
            if (!value_exp) {
                return nullptr;
            }
            
            // Generate assignment: array_element = value
            auto assign_tac = tac_gen.do_assign(target_exp->place, value_exp);
            
            // Combine the target code (if any) with the assignment
            auto combined_tac = tac_gen.join_tac(target_exp->code, assign_tac);
            
            // Return expression with the array element as result
            return tac_gen.mk_exp(target_exp->place, combined_tac);
        }
        
        // For dereference assignment: *ptr = value
        if (expr->target->kind == ASTNodeKind::DEREFERENCE) {
            auto deref_expr = std::dynamic_pointer_cast<DereferenceExpr>(expr->target);
            
            // Generate the pointer expression
            auto ptr_exp = generate_expression(deref_expr->operand);
            if (!ptr_exp || !ptr_exp->place) {
                std::cerr << "Error: Invalid pointer for dereference assignment" << std::endl;
                return nullptr;
            }
            
            // Generate the value expression
            auto value_exp = generate_expression(expr->value);
            if (!value_exp) {
                return nullptr;
            }
            
            // Generate pointer assignment: *ptr = value
            auto assign_tac = tac_gen.do_pointer_assign(ptr_exp, value_exp);
            
            // Return expression with the value as result
            return tac_gen.mk_exp(value_exp->place, assign_tac);
        }
        
        // For other complex targets
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
        
        // For constant array indices, we can flatten the access into a variable name
        // e.g., a[2] -> a[2], arr[i][j] -> requires runtime computation
        
        // Check if the index is a constant
        if (expr->index->kind == ASTNodeKind::CONST_INT) {
            auto const_index = std::dynamic_pointer_cast<ConstIntExpr>(expr->index);
            int index_value = const_index->value;
            
            // Build the flattened variable name
            std::string base_name;
            
            // Check if the base is an identifier (simple array)
            if (expr->array->kind == ASTNodeKind::IDENTIFIER) {
                auto id = std::dynamic_pointer_cast<IdentifierExpr>(expr->array);
                base_name = id->name + "[" + std::to_string(index_value) + "]";
            }
            // Check if the base is another array access (multi-dimensional)
            else if (expr->array->kind == ASTNodeKind::ARRAY_ACCESS) {
                // Build the full expression string using to_string
                auto base_array = std::dynamic_pointer_cast<ArrayAccessExpr>(expr->array);
                
                // Use to_string to build the base name for the entire nested access
                std::string full_expr = expr->to_string();
                base_name = full_expr;
            }
            // Check if the base is a member access (struct field array)
            else if (expr->array->kind == ASTNodeKind::MEMBER_ACCESS) {
                auto member = std::dynamic_pointer_cast<MemberAccessExpr>(expr->array);
                base_name = member->to_string() + "[" + std::to_string(index_value) + "]";
            }
            
            if (!base_name.empty()) {
                // Look up the flattened array element variable
                auto elem_var = tac_gen.get_var(base_name);
                if (!elem_var) {
                    std::cerr << "Error: Array element variable not found: " << base_name << std::endl;
                    return nullptr;
                }
                
                // Create expression with the array element variable
                auto result_exp = tac_gen.mk_exp(elem_var, nullptr);
                result_exp->data_type = elem_var->data_type;
                return result_exp;
            }
        }
        
        // For non-constant indices, use runtime address computation
        // arr[i] is equivalent to *(arr + i*sizeof(element))
        // Since we store everything as 4-byte values, sizeof(element) = 4
        
        // Generate the base expression (should be a pointer or array name)
        auto base_exp = generate_expression(expr->array);
        if (!base_exp || !base_exp->place) {
            std::cerr << "Error: Failed to generate base array expression" << std::endl;
            return nullptr;
        }
        
        // Generate the index expression
        auto index_exp = generate_expression(expr->index);
        if (!index_exp || !index_exp->place) {
            std::cerr << "Error: Failed to generate index expression" << std::endl;
            return nullptr;
        }
        
        // Calculate offset: index * 4 (since each element is 4 bytes)
        auto four_const = tac_gen.mk_const(4);
        auto four_exp = tac_gen.mk_exp(four_const, nullptr);
        four_exp->data_type = DATA_TYPE::INT;
        
        auto offset_exp = tac_gen.do_bin(TAC_OP::MUL, index_exp, four_exp);
        
        // Calculate address: base + offset
        auto addr_exp = tac_gen.do_bin(TAC_OP::ADD, base_exp, offset_exp);
        
        // Dereference the address to get the value
        auto result_exp = tac_gen.do_dereference(addr_exp);
        
        return result_exp;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_member_access(std::shared_ptr<MemberAccessExpr> expr) {
        if (!expr) return nullptr;

        std::string field_var_name=expr->to_string();

        // Look up the flattened field variable
        auto field_var = tac_gen.get_var(field_var_name);
        if (!field_var) {
            std::cerr << "Error: Field variable not found: " << field_var_name << std::endl;
            return nullptr;
        }
        
        // Create expression with the field variable, no additional code needed
        auto result_exp = tac_gen.mk_exp(field_var, nullptr);
        result_exp->data_type = field_var->data_type;
        
        return result_exp;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_address_of(std::shared_ptr<AddressOfExpr> expr) {
        if (!expr) return nullptr;
        
        // Generate the operand expression
        auto operand_exp = generate_expression(expr->operand);
        if (!operand_exp || !operand_exp->place) {
            std::cerr << "Error: Invalid operand for address-of operation" << std::endl;
            return nullptr;
        }
        
        // Use TAC generator to create address-of operation
        return tac_gen.do_address_of(operand_exp);
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_dereference(std::shared_ptr<DereferenceExpr> expr) {
        if (!expr) return nullptr;
        
        // Generate the operand expression (should be a pointer)
        auto operand_exp = generate_expression(expr->operand);
        if (!operand_exp || !operand_exp->place) {
            std::cerr << "Error: Invalid operand for dereference operation" << std::endl;
            return nullptr;
        }
        
        // Use TAC generator to create dereference operation
        return tac_gen.do_dereference(operand_exp);
    }

    DATA_TYPE ASTToTACGenerator::convert_type_to_data_type(std::shared_ptr<Type> type) {
        if (!type) return DATA_TYPE::UNDEF;
        
        if (type->kind == TypeKind::BASIC) {
            return type->basic_type;
        }
        
        if (type->kind == TypeKind::STRUCT) {
            return DATA_TYPE::STRUCT;
        }
        
        if (type->kind == TypeKind::POINTER || type->kind == TypeKind::ARRAY) {
             return DATA_TYPE::INT;
        }
        
        if (type->kind == TypeKind::FUNCTION) {
            return convert_type_to_data_type(type->return_type);
        }
        
        return DATA_TYPE::UNDEF;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::expr_vector_to_list(
        const std::vector<std::shared_ptr<Expression>>& exprs) {
        
        if (exprs.empty()) return nullptr;
        
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

