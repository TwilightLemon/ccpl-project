#include "ast_to_tac.hh"
#include <iostream>

namespace twlm::ccpl::modules
{
    ASTToTACGenerator::ASTToTACGenerator()
        : current_function(nullptr)
    {
        tac_gen.init();
    }

    void ASTToTACGenerator::generate(std::shared_ptr<Program> program)
    {
        if (!program)
        {
            std::cerr << "Error: Null program node" << std::endl;
            return;
        }

        for (auto &decl : program->declarations)
        {
            generate_declaration(decl);
        }

        tac_gen.complete();
    }

    // Declaration Generation

    void ASTToTACGenerator::generate_declaration(std::shared_ptr<Declaration> decl)
    {
        if (!decl)
            return;

        switch (decl->kind)
        {
        case ASTNodeKind::VAR_DECL:
            {
                auto tac = generate_var_decl(std::dynamic_pointer_cast<VarDecl>(decl));
                tac_gen.link_tac(tac);
            }
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

    std::shared_ptr<TAC> ASTToTACGenerator::generate_var_decl(std::shared_ptr<VarDecl> decl)
    {
        if (!decl)
            return nullptr;

        if (decl->var_type && decl->var_type->kind == TypeKind::ARRAY)
        {
            std::shared_ptr<TAC> result_tac = nullptr;
            expand_array_elements(decl->var_type, decl->name,
                                  [this, &result_tac](const auto &name, DATA_TYPE dtype)
                                  {
                                      auto elem_tac = tac_gen.declare_var(name, dtype);
                                      result_tac = tac_gen.join_tac(result_tac, elem_tac);
                                  });
            return result_tac;
        }

        if (decl->var_type && decl->var_type->kind == TypeKind::STRUCT)
        {
            std::shared_ptr<TAC> result_tac = nullptr;
            expand_struct_fields(decl->var_type->struct_name, decl->name,
                                [this, &result_tac](const std::string& field_name, DATA_TYPE field_type)
                                {
                                    auto field_var_tac = tac_gen.declare_var(field_name, field_type);
                                    result_tac = tac_gen.join_tac(result_tac, field_var_tac);
                                });
            
            if (decl->init_value)
            {
                std::cerr << "Warning: Struct initialization not yet supported" << std::endl;
            }
            return result_tac;
        }

        //basic type
        DATA_TYPE dtype = convert_type_to_data_type(decl->var_type);
        bool is_pointer = (decl->var_type && decl->var_type->kind == TypeKind::POINTER);
        auto var_tac = tac_gen.declare_var(decl->name, dtype, is_pointer);

        if (decl->init_value && current_function)
        {
            auto init_exp = generate_expression(decl->init_value);
            auto var_sym = tac_gen.get_var(decl->name);
            auto assign_tac = tac_gen.do_assign(var_sym, init_exp);
            return tac_gen.join_tac(var_tac, assign_tac);
        }
        return var_tac;
    }

    void ASTToTACGenerator::generate_func_decl(std::shared_ptr<FuncDecl> decl)
    {
        if (!decl)
            return;

        DATA_TYPE return_type = convert_type_to_data_type(decl->return_type);
        auto func_sym = tac_gen.declare_func(decl->name, return_type);
        current_function = func_sym;

        tac_gen.enter_scope();

        // para decl
        std::shared_ptr<TAC> param_code = nullptr;
        for (auto &param : decl->parameters)
        {
            // struct type
            if (param->param_type && param->param_type->kind == TypeKind::STRUCT)
            {
                expand_struct_fields(param->param_type->struct_name, param->name,
                                    [this, &param_code](const std::string& field_name, DATA_TYPE field_type)
                                    {
                                        auto field_param_tac = tac_gen.declare_para(field_name, field_type);
                                        param_code = tac_gen.join_tac(param_code, field_param_tac);
                                    });
            }
            else
            {
                // Normal parameter
                DATA_TYPE param_type = convert_type_to_data_type(param->param_type);
                auto param_tac = tac_gen.declare_para(param->name, param_type);
                param_code = tac_gen.join_tac(param_code, param_tac);
            }
        }

        // Generate function body
        std::shared_ptr<TAC> body_code = nullptr;
        if (decl->body)
        {
            body_code = generate_block_stmt(decl->body);
        }

        // create TAC of func
        auto func_tac = tac_gen.do_func(func_sym, param_code, body_code);

        tac_gen.leave_scope();
        current_function = nullptr;
    }

    void ASTToTACGenerator::expand_struct_fields(const std::string& struct_name, const std::string& base_name,
                                                 std::function<void(const std::string&, DATA_TYPE)> handler)
    {
        auto struct_type = tac_gen.get_struct_type(struct_name);
        if (!struct_type)
        {
            std::cerr << "Error: Unknown struct type: " << struct_name << std::endl;
            return;
        }

        for (const auto &field : struct_type->struct_fields)
        {
            std::string field_name = base_name + "." + std::get<0>(field);
            DATA_TYPE field_type = std::get<1>(field);
            handler(field_name, field_type);
        }
    }

    void ASTToTACGenerator::extract_struct_fields(const std::shared_ptr<VarDecl> &field, std::vector<std::pair<std::string, DATA_TYPE>> &fields)
    {
        if (field->var_type && field->var_type->kind == TypeKind::ARRAY)
        {
            expand_array_elements(field->var_type, field->name,
                                  [&fields](const auto &name, DATA_TYPE dtype)
                                  {
                                      fields.push_back({name, dtype});
                                  });
            return;
        }

        if (field->var_type && field->var_type->kind == TypeKind::STRUCT)
        {
            expand_struct_fields(field->var_type->struct_name, field->name,
                                [&fields](const std::string& name, DATA_TYPE dtype)
                                {
                                    fields.push_back({name, dtype});
                                });
        }
    }

    std::shared_ptr<SYM> ASTToTACGenerator::find_array_first_element(const std::string &base_name)
    {
        std::string suffix = "";

        for (int dims = 1; dims <= 5; ++dims)
        {
            suffix += "[0]";
            auto elem = tac_gen.lookup_sym(base_name + suffix);
            if (elem && elem->type == SYM_TYPE::VAR)
            {
                return elem;
            }
            else
            {
                continue;
            }
        }
        return nullptr;
    }

    void ASTToTACGenerator::expand_array_elements(std::shared_ptr<Type> array_type,
                                                  const std::string &base_name,
                                                  std::function<void(const std::string &, DATA_TYPE)> handler)
    {
        if (!array_type || array_type->kind != TypeKind::ARRAY)
            return;

        // expand nested array dimensions!!
        std::vector<int> dimensions;
        std::shared_ptr<Type> current_type = array_type;

        while (current_type && current_type->kind == TypeKind::ARRAY)
        {
            dimensions.push_back(current_type->array_size);
            current_type = current_type->base_type;
        }
        //std::reverse(dimensions.begin(), dimensions.end());

        // current_type is now the base type of the arr
        std::shared_ptr<Type> base_type = current_type;

        std::function<void(size_t, const std::string &)> expand_recursive;
        expand_recursive = [&](size_t dim_index, const auto &current_name)
        {
            if (dim_index >= dimensions.size())
            {
                if (base_type && base_type->kind == TypeKind::STRUCT)
                {
                    expand_struct_fields(base_type->struct_name, current_name, handler);
                }
                else
                {
                    DATA_TYPE dtype = convert_type_to_data_type(base_type);
                    handler(current_name, dtype);
                }
                return;
            }

            // Expand current dimension
            int size = dimensions[dim_index];
            for (int i = 0; i < size; ++i)
            {
                std::string elem_name = current_name + "[" + std::to_string(i) + "]";
                expand_recursive(dim_index + 1, elem_name);
            }
        };

        expand_recursive(0, base_name);
    }

    void ASTToTACGenerator::generate_struct_decl(std::shared_ptr<StructDecl> decl)
    {
        if (!decl)
            return;

        std::vector<std::pair<std::string, DATA_TYPE>> fields;
        for (const auto &field : decl->fields)
        {
            if (field->var_type && field->var_type->kind == TypeKind::ARRAY)
            {
                expand_array_elements(field->var_type, field->name,
                                      [&fields](const std::string &name, DATA_TYPE dtype)
                                      {
                                          fields.push_back({name, dtype});
                                      });
                continue;
            }

            DATA_TYPE field_type = convert_type_to_data_type(field->var_type);
            if (field_type == DATA_TYPE::STRUCT)
            {
                extract_struct_fields(field, fields);
                continue;
            }
            fields.push_back({field->name, field_type});
        }

        auto struct_type = tac_gen.declare_struct_type(decl->name, fields);
    }

    // Statement Generation

    std::shared_ptr<TAC> ASTToTACGenerator::generate_statement(std::shared_ptr<Statement> stmt)
    {
        if (!stmt)
            return nullptr;

        switch (stmt->kind)
        {
        case ASTNodeKind::VAR_DECL:
            return generate_var_decl(std::dynamic_pointer_cast<VarDecl>(stmt));
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

    std::shared_ptr<TAC> ASTToTACGenerator::generate_expr_stmt(std::shared_ptr<ExprStmt> stmt)
    {
        if (!stmt || !stmt->expression)
            return nullptr;

        auto exp = generate_expression(stmt->expression);
        return exp ? exp->code : nullptr;
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_block_stmt(std::shared_ptr<BlockStmt> stmt)
    {
        if (!stmt)
            return nullptr;

        std::shared_ptr<TAC> result = nullptr;
        for (auto &statement : stmt->statements)
        {
            auto stmt_code = generate_statement(statement);
            result = tac_gen.join_tac(result, stmt_code);
        }

        return result;
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_if_stmt(std::shared_ptr<IfStmt> stmt)
    {
        if (!stmt)
            return nullptr;

        auto cond_exp = generate_expression(stmt->condition);
        auto then_code = generate_statement(stmt->then_branch);

        if (stmt->else_branch)
        {
            auto else_code = generate_statement(stmt->else_branch);
            return tac_gen.do_if_else(cond_exp, then_code, else_code);
        }
        else
        {
            return tac_gen.do_if(cond_exp, then_code);
        }
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_while_stmt(std::shared_ptr<WhileStmt> stmt)
    {
        if (!stmt)
            return nullptr;

        tac_gen.begin_while_loop();
        auto cond_exp = generate_expression(stmt->condition);
        auto body_code = generate_statement(stmt->body);
        return tac_gen.end_while_loop(cond_exp, body_code);
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_for_stmt(std::shared_ptr<ForStmt> stmt)
    {
        if (!stmt)
            return nullptr;

        tac_gen.begin_for_loop();

        auto init_code = generate_statement(stmt->init);
        auto cond_exp = generate_expression(stmt->condition);
        auto update_exp = generate_expression(stmt->update);
        auto update_code = update_exp ? update_exp->code : nullptr;
        auto body_code = generate_statement(stmt->body);

        return tac_gen.end_for_loop(init_code, cond_exp, update_code, body_code);
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_return_stmt(std::shared_ptr<ReturnStmt> stmt)
    {
        if (!stmt)
            return nullptr;

        if (stmt->return_value)
        {
            if (stmt->return_value->kind == ASTNodeKind::IDENTIFIER)
            {
                auto id_expr = std::dynamic_pointer_cast<IdentifierExpr>(stmt->return_value);
                if (id_expr)
                {
                    auto var = tac_gen.get_var(id_expr->name);
                    if (!var)
                    {
                        // This might be a struct variable (which is expanded into fields)
                        // cannot return a struct directly. In reality, a struct is usually returned by passing as a para.
                        std::cerr << "Warning: Returning struct value '" << id_expr->name
                                  << "' - TAC doesn't support struct returns, returning void" << std::endl;
                        return tac_gen.do_return(nullptr);
                    }
                }
            }

            auto ret_exp = generate_expression(stmt->return_value);
            return tac_gen.do_return(ret_exp);
        }
        else
        {
            return tac_gen.do_return(nullptr);
        }
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_break_stmt(std::shared_ptr<BreakStmt> stmt)
    {
        return tac_gen.do_break();
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_continue_stmt(std::shared_ptr<ContinueStmt> stmt)
    {
        return tac_gen.do_continue();
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_input_stmt(std::shared_ptr<InputStmt> stmt)
    {
        if (!stmt)
            return nullptr;

        auto var_sym = tac_gen.get_var(stmt->var_name);
        return tac_gen.do_input(var_sym);
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_output_stmt(std::shared_ptr<OutputStmt> stmt)
    {
        if (!stmt)
            return nullptr;

        auto exp = generate_expression(stmt->expression);
        auto output_tac = tac_gen.do_output(exp->place);
        return tac_gen.join_tac(exp->code, output_tac);
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_switch_stmt(std::shared_ptr<SwitchStmt> stmt)
    {
        if (!stmt)
            return nullptr;

        tac_gen.begin_switch();
        auto cond_exp = generate_expression(stmt->condition);
        auto body_code = generate_statement(stmt->body);
        return tac_gen.end_switch(cond_exp, body_code);
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_case_stmt(std::shared_ptr<CaseStmt> stmt)
    {
        if (!stmt)
            return nullptr;

        return tac_gen.do_case(stmt->value);
    }

    std::shared_ptr<TAC> ASTToTACGenerator::generate_default_stmt(std::shared_ptr<DefaultStmt> stmt)
    {
        return tac_gen.do_default();
    }

    // Expression Generation

    std::shared_ptr<EXP> ASTToTACGenerator::generate_expression(std::shared_ptr<Expression> expr)
    {
        if (!expr)
            return nullptr;

        switch (expr->kind)
        {
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

    std::shared_ptr<EXP> ASTToTACGenerator::generate_const_int(std::shared_ptr<ConstIntExpr> expr)
    {
        if (!expr)
            return nullptr;

        auto sym = tac_gen.mk_const(expr->value);
        auto exp = tac_gen.mk_exp(sym, nullptr);
        exp->data_type = DATA_TYPE::INT;
        return exp;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_const_char(std::shared_ptr<ConstCharExpr> expr)
    {
        if (!expr)
            return nullptr;

        auto sym = tac_gen.mk_const_char(expr->value);
        auto exp = tac_gen.mk_exp(sym, nullptr);
        exp->data_type = DATA_TYPE::CHAR;
        return exp;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_string_literal(std::shared_ptr<StringLiteralExpr> expr)
    {
        if (!expr)
            return nullptr;

        auto sym = tac_gen.mk_text(expr->value);
        auto exp = tac_gen.mk_exp(sym, nullptr);
        exp->data_type = DATA_TYPE::CHAR; // String is char*
        return exp;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_identifier(std::shared_ptr<IdentifierExpr> expr)
    {
        if (!expr)
            return nullptr;

        auto var = tac_gen.lookup_sym(expr->name);
        if (var && var->type == SYM_TYPE::VAR)
        {
            auto exp = tac_gen.mk_exp(var, nullptr);
            exp->data_type = var->data_type;
            return exp;
        }

        // not found, check if it's an array
        auto first_elem = find_array_first_element(expr->name);
        if (first_elem && first_elem->type == SYM_TYPE::VAR)
        {
            // Generate &arr[0] (or &arr[0][0], etc.) to get the address of the first element
            auto first_elem_exp = tac_gen.mk_exp(first_elem, nullptr);
            auto addr_result = tac_gen.do_address_of(first_elem_exp);
            return addr_result;
        }

        std::cerr << "Error: Variable not found: " << expr->name << std::endl;
        return nullptr;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_binary_op(std::shared_ptr<BinaryOpExpr> expr)
    {
        if (!expr)
            return nullptr;

        auto left_exp = generate_expression(expr->left);
        auto right_exp = generate_expression(expr->right);
        return tac_gen.do_bin(expr->op, left_exp, right_exp);
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_unary_op(std::shared_ptr<UnaryOpExpr> expr)
    {
        if (!expr)
            return nullptr;

        auto operand_exp = generate_expression(expr->operand);
        return tac_gen.do_un(expr->op, operand_exp);
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_assign(std::shared_ptr<AssignExpr> expr)
    {
        if (!expr)
            return nullptr;
        
        auto value_exp= generate_expression(expr->value);
        if(!value_exp)
            return nullptr;

        // For simple identifier assignment
        if (expr->target->kind == ASTNodeKind::IDENTIFIER)
        {
            auto id_expr = std::dynamic_pointer_cast<IdentifierExpr>(expr->target);

            // Special case: assigning a string literal to a char array
            // (this is not a c-style gramma..)
            if (expr->value->kind == ASTNodeKind::STRING_LITERAL)
            {
                auto string_literal = std::dynamic_pointer_cast<StringLiteralExpr>(expr->value);
                auto first_elem = find_array_first_element(id_expr->name);
                if (first_elem)
                {
                    std::shared_ptr<TAC> assign_tac = nullptr;
                    std::shared_ptr<SYM> last_var = nullptr;

                    // Assign each char to arr
                    for (size_t i = 0; i < string_literal->value.length(); ++i)
                    {
                        std::string elem_name = id_expr->name + "[" + std::to_string(i) + "]";
                        auto elem_var = tac_gen.get_var(elem_name);
                        if (!elem_var)
                        {
                            std::cerr << "Error: Array element not found: " << elem_name << std::endl;
                            break;
                        }
                        auto char_sym = tac_gen.mk_const_char(string_literal->value[i]);
                        auto char_exp = tac_gen.mk_exp(char_sym, nullptr);

                        auto elem_assign = tac_gen.do_assign(elem_var, char_exp);
                        assign_tac = tac_gen.join_tac(assign_tac, elem_assign);
                        last_var = elem_var;
                    }

                    // append '\0' (unsafe, for limited array boundary)
                    std::string null_elem_name = id_expr->name + "[" + std::to_string(string_literal->value.length()) + "]";
                    auto null_elem_var = tac_gen.get_var(null_elem_name);
                    if (null_elem_var)
                    {
                        auto null_sym = tac_gen.mk_const_char('\0');
                        auto null_exp = tac_gen.mk_exp(null_sym, nullptr);
                        auto null_assign = tac_gen.do_assign(null_elem_var, null_exp);
                        assign_tac = tac_gen.join_tac(assign_tac, null_assign);
                        last_var = null_elem_var;
                    }

                    return tac_gen.mk_exp(last_var, assign_tac);
                }
            }

            auto var = tac_gen.get_var(id_expr->name);
            auto assign_tac = tac_gen.do_assign(var, value_exp);
            return tac_gen.mk_exp(var, assign_tac);
        }

        if (expr->target->kind == ASTNodeKind::MEMBER_ACCESS)
        {
            auto member_expr = std::dynamic_pointer_cast<MemberAccessExpr>(expr->target);
            std::string field_var_name = member_expr->to_string();

            auto field_var = tac_gen.get_var(field_var_name);
            if (!field_var)
            {
                std::cerr << "Error: Field variable not found: " << field_var_name << std::endl;
                return nullptr;
            }
            auto assign_tac = tac_gen.do_assign(field_var, value_exp);

            // Return expression with the field variable as result
            return tac_gen.mk_exp(field_var, assign_tac);
        }

        // For array access assignment
        if (expr->target->kind == ASTNodeKind::ARRAY_ACCESS)
        {
            auto target_exp = generate_array_access(std::dynamic_pointer_cast<ArrayAccessExpr>(expr->target));
            if (!target_exp || !target_exp->place)
            {
                std::cerr << "Error: Failed to generate array access target" << std::endl;
                return nullptr;
            }

            // assign: target_exp = value_exp;
            auto assign_tac = tac_gen.do_assign(target_exp->place, value_exp);
            auto combined_tac = tac_gen.join_tac(target_exp->code, assign_tac);

            // assign_expr
            return tac_gen.mk_exp(target_exp->place, combined_tac);
        }

        // For dereference assignment: *ptr = value
        if (expr->target->kind == ASTNodeKind::DEREFERENCE)
        {
            auto deref_expr = std::dynamic_pointer_cast<DereferenceExpr>(expr->target);

            // Generate the pointer expression
            auto ptr_exp = generate_expression(deref_expr->operand);
            if (!ptr_exp || !ptr_exp->place)
            {
                std::cerr << "Error: Invalid pointer for dereference assignment" << std::endl;
                return nullptr;
            }

            // assign: *ptr = value
            auto assign_tac = tac_gen.do_pointer_assign(ptr_exp, value_exp);
            return tac_gen.mk_exp(value_exp->place, assign_tac);
        }

        std::cerr << "Warning: Complex assignment targets not yet fully supported" << std::endl;
        return nullptr;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_func_call(std::shared_ptr<FuncCallExpr> expr)
    {
        if (!expr)
            return nullptr;

        auto arg_list = expr_vector_to_list(expr->arguments);
        return tac_gen.do_call_ret(expr->func_name, arg_list);
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_array_access(std::shared_ptr<ArrayAccessExpr> expr)
    {
        if (!expr)
            return nullptr;

        // a[constant] -> a[constant], arr[i][j] -> dynamic computation

        if (expr->index->kind == ASTNodeKind::CONST_INT)
        {
            //check if each array access index is constant
            if (expr->all_constant_access())
            {
                std::string base_name = expr->to_string();

                if (!base_name.empty())
                {
                    auto elem_var = tac_gen.get_var(base_name);
                    if (!elem_var)
                    {
                        std::cerr << "Error: Array element variable not found: " << base_name << std::endl;
                        return nullptr;
                    }
                    auto result_exp = tac_gen.mk_exp(elem_var, nullptr);
                    result_exp->data_type = elem_var->data_type;
                    return result_exp;
                }
            }
        }

        //TODO: 多维数组的动态偏移值计算.

        // For non-constant indices, use runtime address computation
        // arr[i] is equivalent to *(arr + i)
        // The pointer arithmetic in do_bin will automatically scale by 4

        // Generate the base expression (should be a pointer or array name)
        auto base_exp = generate_expression(expr->array);
        if (!base_exp || !base_exp->place)
        {
            std::cerr << "Error: Failed to generate base array expression" << std::endl;
            return nullptr;
        }

        // Generate the index expression
        auto index_exp = generate_expression(expr->index);
        if (!index_exp || !index_exp->place)
        {
            std::cerr << "Error: Failed to generate index expression" << std::endl;
            return nullptr;
        }

        // Calculate address: base + index
        // The pointer arithmetic will automatically scale the index by 4
        auto addr_exp = tac_gen.do_bin(TAC_OP::ADD, base_exp, index_exp);

        // Dereference the address to get the value
        auto result_exp = tac_gen.do_dereference(addr_exp);

        return result_exp;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_member_access(std::shared_ptr<MemberAccessExpr> expr)
    {
        if (!expr)
            return nullptr;

        std::string field_var_name = expr->to_string();

        // Look up the flattened field variable (use lookup_sym to avoid error messages)
        auto field_var = tac_gen.lookup_sym(field_var_name);
        if (field_var && field_var->type == SYM_TYPE::VAR)
        {
            // Create expression with the field variable, no additional code needed
            auto result_exp = tac_gen.mk_exp(field_var, nullptr);
            result_exp->data_type = field_var->data_type;
            return result_exp;
        }

        // If variable is not found, check if it's an array
        auto first_elem = find_array_first_element(field_var_name);
        if (first_elem && first_elem->type == SYM_TYPE::VAR)
        {
            // This member refers to an array (any dimension)
            // Generate &arr[0] (or &arr[0][0], etc.) to get the address of the first element
            auto first_elem_exp = tac_gen.mk_exp(first_elem, nullptr);
            auto addr_result = tac_gen.do_address_of(first_elem_exp);
            return addr_result;
        }

        std::cerr << "Error: Field variable not found: " << field_var_name << std::endl;
        return nullptr;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_address_of(std::shared_ptr<AddressOfExpr> expr)
    {
        if (!expr)
            return nullptr;

        // Generate the operand expression
        auto operand_exp = generate_expression(expr->operand);
        if (!operand_exp || !operand_exp->place)
        {
            std::cerr << "Error: Invalid operand for address-of operation" << std::endl;
            return nullptr;
        }

        // Use TAC generator to create address-of operation
        return tac_gen.do_address_of(operand_exp);
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_dereference(std::shared_ptr<DereferenceExpr> expr)
    {
        if (!expr)
            return nullptr;

        // Generate the operand expression (should be a pointer)
        auto operand_exp = generate_expression(expr->operand);
        if (!operand_exp || !operand_exp->place)
        {
            std::cerr << "Error: Invalid operand for dereference operation" << std::endl;
            return nullptr;
        }

        // Use TAC generator to create dereference operation
        return tac_gen.do_dereference(operand_exp);
    }

    DATA_TYPE ASTToTACGenerator::convert_type_to_data_type(std::shared_ptr<Type> type)
    {
        if (!type)
            return DATA_TYPE::UNDEF;

        if (type->kind == TypeKind::BASIC)
        {
            return type->basic_type;
        }

        if (type->kind == TypeKind::STRUCT)
        {
            return DATA_TYPE::STRUCT;
        }

        if (type->kind == TypeKind::POINTER || type->kind == TypeKind::ARRAY)
        {
            return DATA_TYPE::INT;
        }

        if (type->kind == TypeKind::FUNCTION)
        {
            return convert_type_to_data_type(type->return_type);
        }

        return DATA_TYPE::UNDEF;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::expr_vector_to_list(
        const std::vector<std::shared_ptr<Expression>> &exprs)
    {

        if (exprs.empty())
            return nullptr;

        std::shared_ptr<EXP> result = nullptr;
        for (auto it = exprs.rbegin(); it != exprs.rend(); ++it)
        {
            auto exp = generate_expression(*it);
            if (exp)
            {
                exp->next = result;
                result = exp;
            }
        }

        return result;
    }

} // namespace twlm::ccpl::modules
