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
        _program = program;

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

        // Handle array declaration - no expansion, just record metadata
        if (decl->var_type && decl->var_type->kind == TypeKind::ARRAY)
        {
            // Create array metadata
            auto metadata = create_array_metadata(decl->name, decl->var_type);
            record_array_metadata(decl->name, decl->var_type);
            
            // Declare array as a single VAR with metadata
            auto var_tac = tac_gen.declare_array(decl->name, metadata);
            return var_tac;
        }

        // Handle struct declaration - no expansion, just record metadata
        if (decl->var_type && decl->var_type->kind == TypeKind::STRUCT)
        {
            if (decl->init_value)
            {
                std::cerr << "Warning: Struct initialization not yet supported" << std::endl;
            }
            
            // Declare struct as a single VAR with metadata
            auto var_tac = tac_gen.declare_struct_var(decl->name, decl->var_type->struct_name);
            return var_tac;
        }

        // Handle pointer and basic types
        bool is_pointer = (decl->var_type && decl->var_type->kind == TypeKind::POINTER);
        DATA_TYPE dtype = convert_type_to_data_type(decl->var_type);
        DATA_TYPE base_type = DATA_TYPE::UNDEF;
        if(is_pointer){
            base_type = convert_type_to_data_type(decl->var_type->base_type);
        }
        auto var_tac = tac_gen.declare_var(decl->name, dtype, is_pointer, base_type);

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
                                     [this, &param_code](const std::string &field_name, DATA_TYPE field_type)
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

    void ASTToTACGenerator::expand_struct_fields(const std::string &struct_name, const std::string &base_name,
                                                 std::function<void(const std::string &, DATA_TYPE)> handler)
    {
        auto struct_type = tac_gen.get_struct_type(struct_name);
        if (!struct_type || !struct_type->struct_metadata)
        {
            std::cerr << "Error: Unknown struct type: " << struct_name << std::endl;
            return;
        }

        // Recursively expand struct fields using metadata
        for (const auto &field_meta : struct_type->struct_metadata->fields)
        {
            std::string field_name = base_name + "." + field_meta.name;

            // Handle different field types
            if (field_meta.type)
            {
                if (field_meta.type->kind == TypeKind::ARRAY)
                {
                    // Record array metadata for this field
                    record_array_metadata(field_name, field_meta.type);

                    // Recursively expand array elements
                    expand_array_elements(field_meta.type, field_name, handler);
                }
                else if (field_meta.type->kind == TypeKind::STRUCT)
                {
                    // Recursively expand nested struct
                    expand_struct_fields(field_meta.type->struct_name, field_name, handler);
                }
                else
                {
                    // Basic type - add directly
                    DATA_TYPE dtype = convert_type_to_data_type(field_meta.type);
                    handler(field_name, dtype);
                }
            }
        }
    }

    std::shared_ptr<SYM> ASTToTACGenerator::find_array_first_element(const std::string &base_name)
    {
        std::clog<<"find_array_first_element: "<<base_name<<std::endl;
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
        std::reverse(dimensions.begin(), dimensions.end());

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

        // Create struct metadata with complete type information (no expansion)
        auto metadata = std::make_shared<StructTypeMetadata>(decl->name);

        for (const auto &field : decl->fields)
        {
            // Store complete type information including arrays, nested structs, etc.
            StructFieldMetadata field_meta(field->name, field->var_type);
            metadata->fields.push_back(field_meta);
        }

        // Register struct type with metadata (no expansion at this point)
        auto struct_type = tac_gen.declare_struct_type(decl->name, metadata);
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

        auto value_exp = generate_expression(expr->value);
        if (!value_exp)
            return nullptr;

        // For simple identifier assignment
        if (expr->target->kind == ASTNodeKind::IDENTIFIER)
        {
            auto id_expr = std::dynamic_pointer_cast<IdentifierExpr>(expr->target);
            auto var = tac_gen.get_var(id_expr->name);
            auto assign_tac = tac_gen.do_assign(var, value_exp);
            return tac_gen.mk_exp(var, assign_tac);
        }

        if (expr->target->kind == ASTNodeKind::MEMBER_ACCESS)
        {
            auto member_expr = std::dynamic_pointer_cast<MemberAccessExpr>(expr->target);

            // Generate the address of the member field
            auto addr_exp = generate_member_address(member_expr);
            if (!addr_exp || !addr_exp->place)
            {
                std::cerr << "Error: Failed to generate member address for assignment" << std::endl;
                return nullptr;
            }

            // Use pointer assignment: *addr = value
            auto assign_tac = tac_gen.do_pointer_assign(addr_exp, value_exp);
            return tac_gen.mk_exp(value_exp->place, assign_tac);
        }

        // For array access assignment
        if (expr->target->kind == ASTNodeKind::ARRAY_ACCESS)
        {
            auto array_access_expr = std::dynamic_pointer_cast<ArrayAccessExpr>(expr->target);

            // Generate the address of the array element (without dereferencing)
            auto addr_exp = generate_array_address(array_access_expr);
            if (!addr_exp || !addr_exp->place)
            {
                std::cerr << "Error: Failed to generate array address for assignment" << std::endl;
                return nullptr;
            }

            // Use pointer assignment: *addr = value
            auto assign_tac = tac_gen.do_pointer_assign(addr_exp, value_exp);
            return tac_gen.mk_exp(value_exp->place, assign_tac);
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

        //calculate array address
        auto addr_exp = generate_array_address(expr);
        if (!addr_exp || !addr_exp->place)
        {
            std::cerr << "Error: Failed to calculate array address" << std::endl;
            return nullptr;
        }

        // *addr
        auto result_exp = tac_gen.do_dereference(addr_exp);

        // Set the correct data type
        if (result_exp && result_exp->place)
        {
            // For array access, get base type from metadata
            std::shared_ptr<Expression> current = expr;

            while (current && current->kind == ASTNodeKind::ARRAY_ACCESS)
            {
                auto arr_access = std::dynamic_pointer_cast<ArrayAccessExpr>(current);
                current = arr_access->array;
            }
            
            // Try static lookup first, then infer if needed
            std::string base_array_name = current->to_string();
            auto metadata = get_array_metadata(base_array_name);
            if (!metadata)
            {
                metadata = infer_array_metadata_from_access(current, base_array_name);
            }
            
            if (metadata)
            {
                result_exp->place->data_type = metadata->base_type;
                result_exp->data_type = metadata->base_type;
            }
        }

        return result_exp;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_member_access(std::shared_ptr<MemberAccessExpr> expr)
    {
        if (!expr)
            return nullptr;

        // Calculate address of the member, then dereference it
        auto member_addr = generate_member_address(expr);
        if (!member_addr || !member_addr->place)
        {
            std::cerr << "Error: Failed to calculate member address" << std::endl;
            return nullptr;
        }

        // Get struct type to determine field type
        std::string struct_type_name;
        DATA_TYPE field_type = DATA_TYPE::INT; // Default
        
        if (expr->object->kind == ASTNodeKind::IDENTIFIER)
        {
            auto id = std::dynamic_pointer_cast<IdentifierExpr>(expr->object);
            auto base_var = tac_gen.get_var(id->name);
            if (base_var)
            {
                struct_type_name = base_var->struct_type_name;
                auto struct_type = tac_gen.get_struct_type(struct_type_name);
                if (struct_type && struct_type->struct_metadata)
                {
                    auto field_meta = struct_type->struct_metadata->get_field(expr->member_name);
                    if (field_meta && field_meta->type)
                    {
                        field_type = convert_type_to_data_type(field_meta->type);
                    }
                }
            }
        }

        // Dereference to get the value
        auto result = tac_gen.do_dereference(member_addr);
        
        // Set correct data type
        if (result && result->place)
        {
            result->place->data_type = field_type;
            result->data_type = field_type;
        }
        
        return result;
    }
    
    std::shared_ptr<EXP> ASTToTACGenerator::generate_member_address(std::shared_ptr<MemberAccessExpr> expr)
    {
        if (!expr)
            return nullptr;

        // First, get the address of the object (base)
        std::shared_ptr<EXP> base_addr;
        std::string struct_type_name;
        
        if (expr->object->kind == ASTNodeKind::IDENTIFIER)
        {
            // Simple case: obj.field
            auto id = std::dynamic_pointer_cast<IdentifierExpr>(expr->object);
            auto base_var = tac_gen.get_var(id->name);
            if (!base_var)
            {
                std::cerr << "Error: Struct variable not found: " << id->name << std::endl;
                return nullptr;
            }
            
            // Get address of base struct
            auto base_exp = tac_gen.mk_exp(base_var, nullptr);
            base_addr = tac_gen.do_address_of(base_exp);
            
            struct_type_name = base_var->struct_type_name;
        }
        else if (expr->object->kind == ASTNodeKind::ARRAY_ACCESS)
        {
            // Array element access: arr[i].field or a1[0].field
            auto arr_access = std::dynamic_pointer_cast<ArrayAccessExpr>(expr->object);
            base_addr = generate_array_address(arr_access);
            
            // Determine struct type from array metadata
            // Find the base array to get its metadata
            std::shared_ptr<Expression> base_expr = arr_access->array;
            while (base_expr->kind == ASTNodeKind::ARRAY_ACCESS)
            {
                base_expr = std::dynamic_pointer_cast<ArrayAccessExpr>(base_expr)->array;
            }
            
            // Get the base array variable
            if (base_expr->kind == ASTNodeKind::IDENTIFIER)
            {
                auto id = std::dynamic_pointer_cast<IdentifierExpr>(base_expr);
                auto var = tac_gen.get_var(id->name);
                if (var && var->is_array && var->array_metadata)
                {
                    if (var->array_metadata->base_type == DATA_TYPE::STRUCT)
                    {
                        struct_type_name = var->array_metadata->struct_type_name;
                    }
                    else
                    {
                        std::cerr << "Error: Array elements are not struct type" << std::endl;
                        return nullptr;
                    }
                }
                else
                {
                    std::cerr << "Error: Array variable not found or not an array: " << id->name << std::endl;
                    return nullptr;
                }
            }
            else
            {
                std::cerr << "Error: Complex array base in member access not supported" << std::endl;
                return nullptr;
            }
        }
        else if (expr->object->kind == ASTNodeKind::MEMBER_ACCESS)
        {
            // Nested member access: obj.field1.field2
            auto nested = std::dynamic_pointer_cast<MemberAccessExpr>(expr->object);
            base_addr = generate_member_address(nested);
            
            // Determine struct type - TODO: track this properly
            std::cerr << "Warning: Nested member access not fully implemented yet" << std::endl;
            return nullptr;
        }
        else
        {
            std::cerr << "Error: Unsupported member access base type" << std::endl;
            return nullptr;
        }
        
        if (!base_addr || !base_addr->place)
        {
            std::cerr << "Error: Failed to get base address for member access" << std::endl;
            return nullptr;
        }
        
        // Look up struct type metadata
        auto struct_type = tac_gen.get_struct_type(struct_type_name);
        if (!struct_type || !struct_type->struct_metadata)
        {
            std::cerr << "Error: Unknown struct type: " << struct_type_name << std::endl;
            return nullptr;
        }
        
        // Get field metadata
        auto field_meta = struct_type->struct_metadata->get_field(expr->member_name);
        if (!field_meta)
        {
            std::cerr << "Error: Field not found: " << expr->member_name 
                      << " in struct " << struct_type_name << std::endl;
            return nullptr;
        }
        
        // Calculate field address: base_addr + field_offset
        // NOTE: field_offset is already in words, don't use do_bin which would scale it by 4
        if (field_meta->offset == 0)
        {
            return base_addr;
        }
        else
        {
            auto offset_const = tac_gen.mk_const(field_meta->offset);
            auto offset_exp = tac_gen.mk_exp(offset_const, nullptr);
            offset_exp->data_type = DATA_TYPE::INT;
            
            // Generate direct ADD without pointer arithmetic scaling
            auto result_temp = tac_gen.mk_tmp(DATA_TYPE::INT);
            result_temp->is_pointer = true; // Result is a pointer
            auto result_decl = tac_gen.mk_tac(TAC_OP::VAR, result_temp);
            result_decl->prev = base_addr->code;
            
            auto add_tac = tac_gen.mk_tac(TAC_OP::ADD, result_temp, base_addr->place, offset_const);
            add_tac->prev = result_decl;
            
            auto result = tac_gen.mk_exp(result_temp, add_tac);
            result->data_type = DATA_TYPE::INT;
            return result;
        }
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_address_of(std::shared_ptr<AddressOfExpr> expr)
    {
        if (!expr)
            return nullptr;

        // for array access: &a[x][y]
        // We want the address of the array element, not the address of a temporary variable
        if (expr->operand->kind == ASTNodeKind::ARRAY_ACCESS)
        {
            auto array_access = std::dynamic_pointer_cast<ArrayAccessExpr>(expr->operand);
            return generate_array_address(array_access);
        }
        
        // for member access: &obj.field
        if (expr->operand->kind == ASTNodeKind::MEMBER_ACCESS)
        {
            auto member_access = std::dynamic_pointer_cast<MemberAccessExpr>(expr->operand);
            return generate_member_address(member_access);
        }

        auto operand_exp = generate_expression(expr->operand);
        if (!operand_exp || !operand_exp->place)
        {
            std::cerr << "Error: Invalid operand for address-of operation" << std::endl;
            return nullptr;
        }
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

    std::shared_ptr<ArrayMetadata> ASTToTACGenerator::create_array_metadata(const std::string &name, std::shared_ptr<Type> array_type)
    {
        if (!array_type || array_type->kind != TypeKind::ARRAY)
            return nullptr;

        // Extract all dimensions from nested array types
        std::vector<int> dimensions;
        std::shared_ptr<Type> current_type = array_type;

        while (current_type && current_type->kind == TypeKind::ARRAY)
        {
            dimensions.push_back(current_type->array_size);
            current_type = current_type->base_type;
        }

        // Reverse to get correct order (outer to inner)
        // For char a[5][10], Type structure is: Array<10, Array<5, char>>
        // After collection we have [10, 5], need to reverse to [5, 10]
        std::reverse(dimensions.begin(), dimensions.end());

        // Get the base type (current_type is now the element type)
        DATA_TYPE base_data_type = convert_type_to_data_type(current_type);
        std::string struct_type_name;

        // Determine element size (for now, everything is 4 bytes/words in TAC)
        int element_size = 4;
        if (base_data_type == DATA_TYPE::STRUCT)
        {
            struct_type_name = current_type->struct_name;
            auto struct_meta = tac_gen.get_struct_type(struct_type_name)->struct_metadata;
            element_size = struct_meta->total_size;
        }

        // Create metadata with base type and struct name
        auto metadata = std::make_shared<ArrayMetadata>(name, dimensions, base_data_type, element_size, struct_type_name);

        // Debug output
        std::cout << "Created array metadata: " << metadata->to_string()
                  << " (stride[0]=" << metadata->get_stride(0)
                  << ", base_type=" << static_cast<int>(base_data_type)
                  << ", unit size=" << metadata->element_size;
        if (!struct_type_name.empty())
        {
            std::cout << ", struct_type=" << struct_type_name;
        }
        std::cout << ")" << std::endl;

        return metadata;
    }

    void ASTToTACGenerator::record_array_metadata(const std::string &name, std::shared_ptr<Type> array_type)
    {
        auto metadata = create_array_metadata(name, array_type);
        if (metadata)
        {
            array_metadata_map[name] = metadata;
        }
    }

    std::shared_ptr<ArrayMetadata> ASTToTACGenerator::get_array_metadata(const std::string &name) const
    {
        auto it = array_metadata_map.find(name);
        if (it != array_metadata_map.end())
        {
            return it->second;
        }
        return nullptr;
    }

    std::shared_ptr<ArrayMetadata> ASTToTACGenerator::infer_array_metadata_from_access(
        std::shared_ptr<Expression> array_expr, const std::string& fallback_name)
    {
        // Try to infer array type from the expression
        std::shared_ptr<Type> array_type = nullptr;
        
        if (array_expr->kind == ASTNodeKind::IDENTIFIER)
        {
            // Simple identifier - look up in symbol table
            auto id = std::dynamic_pointer_cast<IdentifierExpr>(array_expr);
            auto var = tac_gen.get_var(id->name);
            if (var && var->is_array && var->array_metadata)
            {
                return var->array_metadata;
            }
        }
        else if (array_expr->kind == ASTNodeKind::ARRAY_ACCESS)
        {
            // Nested array access: a[i][j] - need to get the base and track through
            auto arr_access = std::dynamic_pointer_cast<ArrayAccessExpr>(array_expr);
            // The result type of array access is the element type
            if (arr_access->expr_type && arr_access->expr_type->kind == TypeKind::ARRAY)
            {
                array_type = arr_access->expr_type;
            }
        }
        else if (array_expr->kind == ASTNodeKind::MEMBER_ACCESS)
        {
            // Member access: obj.field where field is an array
            auto member = std::dynamic_pointer_cast<MemberAccessExpr>(array_expr);
            
            // Get the struct type
            std::string struct_type_name;
            if (member->object->kind == ASTNodeKind::IDENTIFIER)
            {
                auto id = std::dynamic_pointer_cast<IdentifierExpr>(member->object);
                auto var = tac_gen.get_var(id->name);
                if (var && var->data_type == DATA_TYPE::STRUCT)
                {
                    struct_type_name = var->struct_type_name;
                }
            }
            else if (member->object->kind == ASTNodeKind::ARRAY_ACCESS)
            {
                // a[i].field - need to infer struct type from array
                auto arr_access = std::dynamic_pointer_cast<ArrayAccessExpr>(member->object);
                
                // First, try to get the base array variable
                std::shared_ptr<Expression> base_expr = arr_access->array;
                while (base_expr->kind == ASTNodeKind::ARRAY_ACCESS)
                {
                    base_expr = std::dynamic_pointer_cast<ArrayAccessExpr>(base_expr)->array;
                }
                
                if (base_expr->kind == ASTNodeKind::IDENTIFIER)
                {
                    auto id = std::dynamic_pointer_cast<IdentifierExpr>(base_expr);
                    auto var = tac_gen.get_var(id->name);
                    if (var && var->is_array && var->array_metadata)
                    {
                        // Get struct type from var's metadata
                        if (var->array_metadata->base_type == DATA_TYPE::STRUCT && 
                            !var->array_metadata->struct_type_name.empty())
                        {
                            struct_type_name = var->array_metadata->struct_type_name;
                        }
                    }
                }
            }
            
            if (!struct_type_name.empty())
            {
                // Look up the field in struct metadata
                auto struct_type = tac_gen.get_struct_type(struct_type_name);
                if (struct_type && struct_type->struct_metadata)
                {
                    auto field = struct_type->struct_metadata->get_field(member->member_name);
                    if (field && field->type && field->type->kind == TypeKind::ARRAY)
                    {
                        array_type = field->type;
                    }
                }
            }
        }
        
        // If we found the type, create metadata
        if (array_type && array_type->kind == TypeKind::ARRAY)
        {
            return create_array_metadata(fallback_name, array_type);
        }
        
        return nullptr;
    }

    std::shared_ptr<EXP> ASTToTACGenerator::generate_array_address(std::shared_ptr<ArrayAccessExpr> expr)
    {
        if (!expr)
            return nullptr;

        // Collect all array access expressions from inner to outer
        std::vector<std::shared_ptr<ArrayAccessExpr>> access_chain;
        std::vector<int> constant_indices; // For constant access optimization
        std::shared_ptr<Expression> current = expr;

        while (current && current->kind == ASTNodeKind::ARRAY_ACCESS)
        {
            auto arr_access = std::dynamic_pointer_cast<ArrayAccessExpr>(current);
            access_chain.push_back(arr_access);
            
            // Collect constant indices for optimization
            if (arr_access->index->kind == ASTNodeKind::CONST_INT)
            {
                auto const_idx = std::dynamic_pointer_cast<ConstIntExpr>(arr_access->index);
                constant_indices.push_back(const_idx->value);
            }
            
            current = arr_access->array;
        }

        // Reverse to get outer-to-inner order
        std::reverse(access_chain.begin(), access_chain.end());
        std::reverse(constant_indices.begin(), constant_indices.end());

        // Get base array name (could be a simple identifier or a member access like "a1[i].a")
        std::string base_array_name = current->to_string();
        
        // For pointer array access: p[i] -> *(p + i)
        if (current->kind == ASTNodeKind::IDENTIFIER)
        {
            auto id = std::dynamic_pointer_cast<IdentifierExpr>(current);
            auto var_sym = tac_gen.get_var(id->name);
            if (var_sym && var_sym->is_pointer)
            {
                auto ptr_exp = generate_expression(expr->array);
                if (!ptr_exp || !ptr_exp->place)
                {
                    std::cerr << "Error: Failed to generate pointer expression" << std::endl;
                    return nullptr;
                }

                auto index_exp = generate_expression(expr->index);
                if (!index_exp || !index_exp->place)
                {
                    std::cerr << "Error: Failed to generate index expression" << std::endl;
                    return nullptr;
                }

                // Calculate and return address: p + i
                return tac_gen.do_bin(TAC_OP::ADD, ptr_exp, index_exp);
            }
        }
        
        // Look up or infer metadata for the base array
        auto metadata = get_array_metadata(base_array_name);
        if (!metadata)
        {
            // Try to infer from the expression type
            metadata = infer_array_metadata_from_access(current, base_array_name);
            if (!metadata)
            {
                std::cerr << "Error: No metadata found for array: " << base_array_name << std::endl;
                return nullptr;
            }
        }

        // For nested access (like a1[0].a), we need to calculate the base address differently
        std::shared_ptr<EXP> base_addr;
        
        if (current->kind == ASTNodeKind::MEMBER_ACCESS)
        {
            // This is member access like a1[0].a - get the address of the member
            auto member = std::dynamic_pointer_cast<MemberAccessExpr>(current);
            base_addr = generate_member_address(member);
            if (!base_addr)
            {
                std::cerr << "Error: Failed to get member address for: " << base_array_name << std::endl;
                return nullptr;
            }
        }
        else if (current->kind == ASTNodeKind::ARRAY_ACCESS)
        {
            // This is nested array access like a[i][j] accessing a[i]
            auto arr = std::dynamic_pointer_cast<ArrayAccessExpr>(current);
            base_addr = generate_array_address(arr);
            if (!base_addr)
            {
                std::cerr << "Error: Failed to get array base address for: " << base_array_name << std::endl;
                return nullptr;
            }
        }
        else
        {
            // Simple identifier - get the variable
            auto base_var = tac_gen.get_var(base_array_name);
            if (!base_var)
            {
                std::cerr << "Error: Array variable not found: " << base_array_name << std::endl;
                return nullptr;
            }
            
            // Get address of the base variable
            auto base_exp = tac_gen.mk_exp(base_var, nullptr);
            base_addr = tac_gen.do_address_of(base_exp);
        }

        // OPTIMIZATION: Constant folding for array indices
        // Even if not all indices are constant, we can still fold the constant ones
        bool has_constant_part = (constant_indices.size() == access_chain.size() && 
                                  constant_indices.size() > 0);
        
        if (has_constant_part && expr->all_constant_access())
        {
            // All indices are constant - full compile-time calculation
            int element_offset = 0;
            for (size_t dim = 0; dim < constant_indices.size(); ++dim)
            {
                int stride = metadata->get_stride(dim);
                element_offset += constant_indices[dim] * stride;
            }
            
            int compile_time_offset = element_offset * metadata->element_size;
            
            std::clog << "Compile-time constant array access: " << base_array_name 
                      << " element_offset=" << element_offset 
                      << " word_offset=" << compile_time_offset 
                      << " (element_size=" << metadata->element_size << ")" << std::endl;
            
            if (compile_time_offset == 0)
            {
                return base_addr;
            }
            else
            {
                auto offset_const = tac_gen.mk_const(compile_time_offset);
                auto result_temp = tac_gen.mk_tmp(DATA_TYPE::INT);
                result_temp->is_pointer = true;
                auto result_decl = tac_gen.mk_tac(TAC_OP::VAR, result_temp);
                result_decl->prev = base_addr->code;
                
                auto add_tac = tac_gen.mk_tac(TAC_OP::ADD, result_temp, base_addr->place, offset_const);
                add_tac->prev = result_decl;
                
                auto result = tac_gen.mk_exp(result_temp, add_tac);
                result->data_type = DATA_TYPE::INT;
                return result;
            }
        }

        // DYNAMIC ADDRESS CALCULATION with partial constant folding
        // Calculate total offset in elements: offset = i0*stride0 + i1*stride1 + i2*stride2 + ...
        std::shared_ptr<EXP> element_offset = nullptr;
        int constant_element_offset = 0;  // Accumulate compile-time constant parts

        for (size_t dim = 0; dim < access_chain.size(); ++dim)
        {
            // Check if this index is a constant
            if (dim < constant_indices.size() && 
                access_chain[dim]->index->kind == ASTNodeKind::CONST_INT)
            {
                // Constant index - accumulate at compile time
                int stride = metadata->get_stride(dim);
                constant_element_offset += constant_indices[dim] * stride;
                continue;  // Skip TAC generation for this dimension
            }

            // Dynamic index - generate TAC
            auto index_exp = generate_expression(access_chain[dim]->index);
            if (!index_exp || !index_exp->place)
            {
                std::cerr << "Error: Failed to generate index for dimension " << dim << std::endl;
                return nullptr;
            }

            // Get stride for this dimension
            int stride = metadata->get_stride(dim);

            // Calculate: index * stride
            std::shared_ptr<EXP> scaled_index;
            if (stride == 1)
            {
                scaled_index = index_exp;
            }
            else
            {
                auto stride_sym = tac_gen.mk_const(stride);
                auto stride_exp = tac_gen.mk_exp(stride_sym, nullptr);
                stride_exp->data_type = DATA_TYPE::INT;
                scaled_index = tac_gen.do_bin(TAC_OP::MUL, index_exp, stride_exp);
            }

            // Add to total element offset
            if (!element_offset)
            {
                element_offset = scaled_index;
            }
            else
            {
                element_offset = tac_gen.do_bin(TAC_OP::ADD, element_offset, scaled_index);
            }
        }

        // Convert element offset to word offset by multiplying with element_size
        // Handle both dynamic and constant parts
        std::shared_ptr<EXP> dynamic_word_offset = nullptr;
        int constant_word_offset = constant_element_offset * metadata->element_size;
        
        if (element_offset)
        {
            // We have dynamic part
            if (metadata->element_size == 1)
            {
                dynamic_word_offset = element_offset;
            }
            else
            {
                auto size_const = tac_gen.mk_const(metadata->element_size);
                auto size_exp = tac_gen.mk_exp(size_const, nullptr);
                size_exp->data_type = DATA_TYPE::INT;
                dynamic_word_offset = tac_gen.do_bin(TAC_OP::MUL, element_offset, size_exp);
            }
        }

        // Calculate final address: base_addr + dynamic_offset + constant_offset
        auto result_temp = tac_gen.mk_tmp(DATA_TYPE::INT);
        result_temp->is_pointer = true;
        auto result_decl = tac_gen.mk_tac(TAC_OP::VAR, result_temp);
        
        if (dynamic_word_offset && constant_word_offset != 0)
        {
            // Both dynamic and constant parts
            result_decl->prev = tac_gen.join_tac(base_addr->code, dynamic_word_offset->code);
            
            // temp1 = base_addr + dynamic_offset
            auto temp1 = tac_gen.mk_tmp(DATA_TYPE::INT);
            temp1->is_pointer = true;
            auto temp1_decl = tac_gen.mk_tac(TAC_OP::VAR, temp1);
            temp1_decl->prev = result_decl;
            
            auto add1_tac = tac_gen.mk_tac(TAC_OP::ADD, temp1, base_addr->place, dynamic_word_offset->place);
            add1_tac->prev = temp1_decl;
            
            // result = temp1 + constant_offset
            auto const_offset = tac_gen.mk_const(constant_word_offset);
            auto add2_tac = tac_gen.mk_tac(TAC_OP::ADD, result_temp, temp1, const_offset);
            add2_tac->prev = add1_tac;
            
            auto final_result = tac_gen.mk_exp(result_temp, add2_tac);
            final_result->data_type = DATA_TYPE::INT;
            return final_result;
        }
        else if (dynamic_word_offset)
        {
            // Only dynamic part
            result_decl->prev = tac_gen.join_tac(base_addr->code, dynamic_word_offset->code);
            auto add_tac = tac_gen.mk_tac(TAC_OP::ADD, result_temp, base_addr->place, dynamic_word_offset->place);
            add_tac->prev = result_decl;
            
            auto final_result = tac_gen.mk_exp(result_temp, add_tac);
            final_result->data_type = DATA_TYPE::INT;
            return final_result;
        }
        else if (constant_word_offset != 0)
        {
            // Only constant part
            result_decl->prev = base_addr->code;
            auto const_offset = tac_gen.mk_const(constant_word_offset);
            auto add_tac = tac_gen.mk_tac(TAC_OP::ADD, result_temp, base_addr->place, const_offset);
            add_tac->prev = result_decl;
            
            auto final_result = tac_gen.mk_exp(result_temp, add_tac);
            final_result->data_type = DATA_TYPE::INT;
            return final_result;
        }
        else
        {
            // No offset at all
            return base_addr;
        }
    }

} // namespace twlm::ccpl::modules
