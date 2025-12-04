#include "tac.hh"
#include <sstream>
#include <iomanip>

using namespace twlm::ccpl::modules;

TACGenerator::TACGenerator() 
    : scope(SYM_SCOPE::GLOBAL), next_tmp(0), next_label(1), 
      current_var_type(DATA_TYPE::UNDEF), current_func(nullptr),
      tac_first(nullptr), tac_last(nullptr) {}

void TACGenerator::init() {
    scope = SYM_SCOPE::GLOBAL;
    next_tmp = 0;
    next_label = 1;
    current_var_type = DATA_TYPE::UNDEF;
    current_func = nullptr;
    sym_tab_global.clear();
    sym_tab_local.clear();
    tac_first = nullptr;
    tac_last = nullptr;
}

void TACGenerator::set_current_type(DATA_TYPE type) {
    current_var_type = type;
}

DATA_TYPE TACGenerator::get_current_type() const {
    return current_var_type;
}

void TACGenerator::complete() {
    std::shared_ptr<TAC> cur = nullptr;
    std::shared_ptr<TAC> prev = tac_last;
    
    while (prev != nullptr) {
        prev->next = cur;
        cur = prev;
        prev = prev->prev;
    }
    
    tac_first = cur;
}

std::shared_ptr<SYM> TACGenerator::lookup_sym(const std::string& name) {
    if (scope == SYM_SCOPE::LOCAL) {
        auto it = sym_tab_local.find(name);
        if (it != sym_tab_local.end()) {
            return it->second;
        }
    }
    
    auto it = sym_tab_global.find(name);
    if (it != sym_tab_global.end()) {
        return it->second;
    }
    
    return nullptr;
}

std::shared_ptr<SYM> TACGenerator::mk_var(const std::string& name, DATA_TYPE dtype) {
    auto sym = lookup_sym(name);
    
    if (sym != nullptr) {
        error("Variable already declared: " + name);
        return sym;
    }
    
    sym = std::make_shared<SYM>();
    sym->type = SYM_TYPE::VAR;
    sym->data_type = dtype;
    sym->name = name;
    sym->scope = scope;
    sym->offset = -1;
    
    if (scope == SYM_SCOPE::LOCAL) {
        sym_tab_local[name] = sym;
    } else {
        sym_tab_global[name] = sym;
    }
    
    return sym;
}

std::shared_ptr<SYM> TACGenerator::mk_tmp(DATA_TYPE dtype) {
    std::string name = "@t" + std::to_string(next_tmp++);
    auto sym = std::make_shared<SYM>();
    sym->type = SYM_TYPE::VAR;
    sym->data_type = dtype;
    sym->name = name;
    sym->scope = scope;
    sym->offset = -1;
    
    if (scope == SYM_SCOPE::LOCAL) {
        sym_tab_local[name] = sym;
    } else {
        sym_tab_global[name] = sym;
    }
    
    return sym;
}std::shared_ptr<SYM> TACGenerator::mk_const(int value) {
    std::string name = std::to_string(value);
    
    auto it = sym_tab_global.find(name);
    if (it != sym_tab_global.end()) {
        return it->second;
    }
    
    auto sym = std::make_shared<SYM>();
    sym->type = SYM_TYPE::CONST_INT;
    sym->data_type = DATA_TYPE::INT;
    sym->name = name;
    sym->value = value;
    sym->scope = SYM_SCOPE::GLOBAL;
    
    sym_tab_global[name] = sym;
    return sym;
}

std::shared_ptr<SYM> TACGenerator::mk_const_char(char value) {
    std::string name = "'" + std::string(1, value) + "'";
    
    auto it = sym_tab_global.find(name);
    if (it != sym_tab_global.end()) {
        return it->second;
    }
    
    auto sym = std::make_shared<SYM>();
    sym->type = SYM_TYPE::CONST_CHAR;
    sym->data_type = DATA_TYPE::CHAR;
    sym->name = name;
    sym->value = value;
    sym->scope = SYM_SCOPE::GLOBAL;
    
    sym_tab_global[name] = sym;
    return sym;
}

std::shared_ptr<SYM> TACGenerator::mk_text(const std::string& text) {
    auto it = sym_tab_global.find(text);
    if (it != sym_tab_global.end()) {
        return it->second;
    }
    
    auto sym = std::make_shared<SYM>();
    sym->type = SYM_TYPE::TEXT;
    sym->name = text;
    sym->value = text;
    sym->label = next_label++;
    sym->scope = SYM_SCOPE::GLOBAL;
    
    sym_tab_global[text] = sym;
    return sym;
}

std::shared_ptr<SYM> TACGenerator::mk_label(const std::string& name) {
    auto sym = std::make_shared<SYM>();
    sym->type = SYM_TYPE::LABEL;
    sym->name = name;
    sym->scope = scope;
    
    return sym;
}

std::shared_ptr<SYM> TACGenerator::get_var(const std::string& name) {
    auto sym = lookup_sym(name);
    
    if (sym == nullptr) {
        error("Variable not declared: " + name);
        return nullptr;
    }
    
    if (sym->type != SYM_TYPE::VAR) {
        error("Not a variable: " + name);
        return nullptr;
    }
    
    return sym;
}

std::shared_ptr<SYM> TACGenerator::declare_func(const std::string& name, DATA_TYPE return_type) {
    auto sym = sym_tab_global.find(name);
    
    if (sym != sym_tab_global.end()) {
        if (sym->second->type == SYM_TYPE::FUNC) {
            error("Function already declared: " + name);
            return sym->second;
        }
        error("Name already used: " + name);
        return nullptr;
    }
    
    auto func_sym = std::make_shared<SYM>();
    func_sym->type = SYM_TYPE::FUNC;
    func_sym->data_type = return_type;
    func_sym->return_type = return_type;
    func_sym->name = name;
    func_sym->scope = SYM_SCOPE::GLOBAL;
    
    sym_tab_global[name] = func_sym;
    current_func = func_sym;
    return func_sym;
}

std::shared_ptr<TAC> TACGenerator::mk_tac(TAC_OP op, 
                                           std::shared_ptr<SYM> a,
                                           std::shared_ptr<SYM> b,
                                           std::shared_ptr<SYM> c) {
    auto tac = std::make_shared<TAC>(op);
    tac->a = a;
    tac->b = b;
    tac->c = c;
    tac->prev = nullptr;
    tac->next = nullptr;
    
    return tac;
}

std::shared_ptr<TAC> TACGenerator::join_tac(std::shared_ptr<TAC> c1, std::shared_ptr<TAC> c2) {
    if (c1 == nullptr) return c2;
    if (c2 == nullptr) return c1;
    
    // Find the beginning of c2
    auto t = c2;
    while (t->prev != nullptr) {
        t = t->prev;
    }
    
    // Connect c1 to the beginning of c2
    t->prev = c1;
    return c2;
}

std::shared_ptr<TAC> TACGenerator::declare_var(const std::string& name, DATA_TYPE dtype, bool is_pointer, DATA_TYPE base_type) {
    auto var = mk_var(name, dtype);
    var->is_pointer = is_pointer;
    if (base_type!=DATA_TYPE::UNDEF)
    {
        var->base_type = base_type;
    }
    
    return mk_tac(TAC_OP::VAR, var);
}

std::shared_ptr<TAC> TACGenerator::declare_array(const std::string& name, std::shared_ptr<ArrayMetadata> metadata) {
    if (!metadata) {
        error("Array metadata is required for array declaration");
        return nullptr;
    }
    
    // Create array variable with metadata
    auto var = mk_var(name, DATA_TYPE::INT); // Arrays are represented by their base address (INT/pointer)
    var->is_array = true;
    var->array_metadata = metadata;
    
    return mk_tac(TAC_OP::VAR, var);
}

std::shared_ptr<TAC> TACGenerator::declare_struct_var(const std::string& name, const std::string& struct_type_name) {
    // Look up struct type
    auto struct_type = get_struct_type(struct_type_name);
    if (!struct_type || !struct_type->struct_metadata) {
        error("Unknown struct type: " + struct_type_name);
        return nullptr;
    }
    
    // Create struct variable with metadata
    auto var = mk_var(name, DATA_TYPE::STRUCT);
    var->struct_type_name = struct_type_name;
    var->struct_metadata = struct_type->struct_metadata;
    
    return mk_tac(TAC_OP::VAR, var);
}

std::shared_ptr<TAC> TACGenerator::declare_para(const std::string& name, DATA_TYPE dtype, bool is_pointer) {
    auto sym = mk_var(name, dtype);
    sym->is_pointer = is_pointer;
    
    // Add parameter type to current function
    if (current_func != nullptr) {
        current_func->param_types.push_back(dtype);
    }
    
    return mk_tac(TAC_OP::FORMAL, sym);
}

std::shared_ptr<TAC> TACGenerator::do_func(std::shared_ptr<SYM> func,
                                            std::shared_ptr<TAC> args,
                                            std::shared_ptr<TAC> code) {
    auto tlab = mk_tac(TAC_OP::LABEL, mk_label(func->name));
    auto tbegin = mk_tac(TAC_OP::BEGINFUNC);
    auto tend = mk_tac(TAC_OP::ENDFUNC);
    
    tbegin->prev = tlab;
    code = join_tac(args, code);
    tend->prev = join_tac(tbegin, code);
    
    // link 2 to the previous TAC
    if (tac_last != nullptr) {
        tlab->prev = tac_last;
    }
    tac_last = tend;
    
    return tend;
}

std::shared_ptr<TAC> TACGenerator::do_assign(std::shared_ptr<SYM> var, std::shared_ptr<EXP> exp) {
    if (var == nullptr || exp == nullptr) {
        error("Invalid assignment");
        return nullptr;
    }
    
    if (var->type != SYM_TYPE::VAR) {
        error("Assignment to non-variable");
        return nullptr;
    }
    
    // Type checking
    check_assignment_type(var, exp);
    
    // Propagate pointer flag from source to destination
    if (exp->place && exp->place->is_pointer) {
        var->is_pointer = true;
    }
    
    auto code = mk_tac(TAC_OP::COPY, var, exp->place);
    code->prev = exp->code;
    
    return code;
}

std::shared_ptr<TAC> TACGenerator::do_input(std::shared_ptr<SYM> var) {
    if (var == nullptr) {
        error("Invalid input");
        return nullptr;
    }
    
    if (var->type != SYM_TYPE::VAR) {
        error("Input to non-variable");
        return nullptr;
    }
    
    return mk_tac(TAC_OP::INPUT, var);
}

std::shared_ptr<TAC> TACGenerator::do_output(std::shared_ptr<SYM> sym) {
    if (sym == nullptr) {
        error("Invalid output");
        return nullptr;
    }
    
    return mk_tac(TAC_OP::OUTPUT, sym);
}

std::shared_ptr<TAC> TACGenerator::do_return(std::shared_ptr<EXP> exp) {
    if (exp == nullptr) {
        // Check void return
        if (current_func && current_func->return_type != DATA_TYPE::VOID) {
            warning("Non-void function should return a value");
        }
        return mk_tac(TAC_OP::RETURN);
    }
    
    // Type checking
    check_return_type(exp);
    
    auto tac = mk_tac(TAC_OP::RETURN, exp->place);
    tac->prev = exp->code;
    return tac;
}

std::shared_ptr<TAC> TACGenerator::do_if(std::shared_ptr<EXP> exp, std::shared_ptr<TAC> stmt) {
    auto label_name = "L" + std::to_string(next_label++);
    auto label = mk_tac(TAC_OP::LABEL, mk_label(label_name));
    auto code = mk_tac(TAC_OP::IFZ, label->a, exp->place);
    
    code->prev = exp->code;
    code = join_tac(code, stmt);
    label->prev = code;
    
    return label;
}

std::shared_ptr<TAC> TACGenerator::do_if_else(std::shared_ptr<EXP> exp,
                                               std::shared_ptr<TAC> stmt1,
                                               std::shared_ptr<TAC> stmt2) {
    auto label1 = mk_tac(TAC_OP::LABEL, mk_label("L" + std::to_string(next_label++)));
    auto label2 = mk_tac(TAC_OP::LABEL, mk_label("L" + std::to_string(next_label++)));
    
    auto code1 = mk_tac(TAC_OP::IFZ, label1->a, exp->place);
    auto code2 = mk_tac(TAC_OP::GOTO, label2->a);
    
    code1->prev = exp->code;
    code1 = join_tac(code1, stmt1);
    code2->prev = code1;
    label1->prev = code2;
    label1 = join_tac(label1, stmt2);
    label2->prev = label1;
    
    return label2;
}

std::shared_ptr<TAC> TACGenerator::do_while(std::shared_ptr<EXP> exp, std::shared_ptr<TAC> stmt) {
    // while (exp) stmt
    // =>
    // continue_label:
    //   ifz exp goto break_label
    //   stmt
    //   goto continue_label
    // break_label:
    auto loop=loop_stack.back();

    auto continue_label = mk_tac(TAC_OP::LABEL, loop.continue_label);
    auto break_label = mk_tac(TAC_OP::LABEL, loop.break_label);

    auto ifz = mk_tac(TAC_OP::IFZ, break_label->a, exp->place);
    auto goto_continue = mk_tac(TAC_OP::GOTO, continue_label->a);

    // Build: continue_label -> cond code -> ifz -> stmt -> goto -> break_label
    auto result = continue_label;
    result = join_tac(result, exp->code);
    ifz->prev = result;
    result = join_tac(ifz, stmt);
    goto_continue->prev = result;
    break_label->prev = goto_continue;
    
    return break_label;
}

void TACGenerator::begin_while_loop() {
    auto continue_label = mk_label("L" + std::to_string(next_label++));
    auto break_label = mk_label("L" + std::to_string(next_label++));
    enter_loop(break_label, continue_label);
}

std::shared_ptr<TAC> TACGenerator::end_while_loop(std::shared_ptr<EXP> exp, std::shared_ptr<TAC> stmt) {
    auto result = do_while(exp, stmt);
    leave_loop();
    return result;
}

void TACGenerator::begin_for_loop() {
    auto loop_start = mk_label("L" + std::to_string(next_label++));
    auto continue_label = mk_label("L" + std::to_string(next_label++));
    auto break_label = mk_label("L" + std::to_string(next_label++));
    enter_loop(break_label, continue_label, loop_start);
}

std::shared_ptr<TAC> TACGenerator::end_for_loop(std::shared_ptr<TAC> init,
                                                 std::shared_ptr<EXP> cond,
                                                 std::shared_ptr<TAC> update,
                                                 std::shared_ptr<TAC> body) {
    auto result = do_for(init, cond, update, body);
    leave_loop();
    return result;
}

void TACGenerator::begin_switch() {
    auto break_label = mk_label("L" + std::to_string(next_label++));
    auto default_label = mk_label("L" + std::to_string(next_label++));
    enter_switch(break_label, default_label);
}


void TACGenerator::leave_switch() {
    if (in_switch()) {
        switch_stack.pop_back();
        context_type_stack.pop_back();
    } else {
        error("Not in a switch context");
    }
}

std::shared_ptr<TAC> TACGenerator::do_case(int value){
    if(!in_switch()){
        error("case statement outside of switch");
        return nullptr;
    }
    auto& ctx = switch_stack.back();
    auto case_label = mk_label("L" + std::to_string(next_label++));
    ctx.case_labels[value] = case_label;
    return mk_tac(TAC_OP::LABEL, case_label);
}
std::shared_ptr<TAC> TACGenerator::do_default(){
    if(!in_switch()){
        error("default statement outside of switch");
        return nullptr;
    }
    auto& ctx = switch_stack.back();
    return mk_tac(TAC_OP::LABEL, ctx.default_label);
}

std::shared_ptr<TAC> TACGenerator::end_switch(std::shared_ptr<EXP> exp, std::shared_ptr<TAC> body) {
    if (!in_switch()) {
        error("Not in a switch context");
        return nullptr;
    }
    
    auto& ctx = switch_stack.back();
    auto switch_end = mk_tac(TAC_OP::LABEL, ctx.break_label);
    
    std::shared_ptr<TAC> case_jumps = nullptr;
    for (const auto& [case_value, case_label] : ctx.case_labels) {
        // 创建中间变量 tmp = exp - case_value
        auto const_sym = mk_const(case_value);
        auto temp = mk_tmp(exp->data_type);
        auto temp_decl = mk_tac(TAC_OP::VAR, temp);
        auto sub_tac = mk_tac(TAC_OP::SUB, temp, exp->place, const_sym);
        
        // 使用 IFZ 判断 tmp 是否为 0
        auto case_jump = mk_tac(TAC_OP::IFZ, case_label, temp);
        
        // 连接代码：case_jumps -> temp_decl -> sub_tac -> case_jump
        temp_decl->prev = case_jumps;
        sub_tac->prev = temp_decl;
        case_jump->prev = sub_tac;
        case_jumps = case_jump;
    }
    
    auto goto_default = mk_tac(TAC_OP::GOTO, ctx.default_label);
    goto_default->prev = case_jumps;
    case_jumps = goto_default;
    
    // Build final TAC: case_jumps -> body -> switch_end
    auto result = join_tac(case_jumps, body);
    switch_end->prev = result;
    
    leave_switch();
    return switch_end;
}

std::shared_ptr<TAC> TACGenerator::do_for(std::shared_ptr<TAC> init,
                                           std::shared_ptr<EXP> cond,
                                           std::shared_ptr<TAC> update,
                                           std::shared_ptr<TAC> body) {
    // for (init; cond; update) body
    // =>
    // init
    // loop_start:
    //   ifz cond goto break_label
    //   body
    // continue_label:
    //   update
    //   goto loop_start
    // break_label:
    auto loop=loop_stack.back();
    
    auto loop_start = mk_tac(TAC_OP::LABEL, loop.loop_start_label);
    auto continue_label = mk_tac(TAC_OP::LABEL, loop.continue_label);
    auto break_label = mk_tac(TAC_OP::LABEL, loop.break_label);
    
    auto ifz = mk_tac(TAC_OP::IFZ, loop.break_label, cond->place);
    auto goto_loop = mk_tac(TAC_OP::GOTO, loop.loop_start_label);
    
    auto result = join_tac(init, loop_start);
    result = join_tac(result, cond->code);
    ifz->prev = result;
    result = join_tac(ifz, body);
    continue_label->prev = result;
    result = join_tac(continue_label, update);
    goto_loop->prev = result;
    break_label->prev = goto_loop;
    
    return break_label;
}

std::shared_ptr<TAC> TACGenerator::do_call(const std::string& name, std::shared_ptr<EXP> arglist) {
    std::shared_ptr<TAC> code = nullptr;
    
    for (auto arg = arglist; arg != nullptr; arg = arg->next) {
        code = join_tac(code, arg->code);
    }
    
    while (arglist != nullptr) {
        auto temp = mk_tac(TAC_OP::ACTUAL, arglist->place);
        temp->prev = code;
        code = temp;
        arglist = arglist->next;
    }
    
    auto func_name = std::make_shared<SYM>();
    func_name->type = SYM_TYPE::FUNC;
    func_name->name = name;
    
    auto temp = mk_tac(TAC_OP::CALL, nullptr, func_name);
    temp->prev = code;
    code = temp;
    
    return code;
}

std::shared_ptr<EXP> TACGenerator::mk_exp(std::shared_ptr<SYM> place,
                                           std::shared_ptr<TAC> code,
                                           std::shared_ptr<EXP> next) {
    auto exp = std::make_shared<EXP>();
    exp->place = place;
    exp->code = code;
    exp->next = next;
    return exp;
}

std::shared_ptr<EXP> TACGenerator::do_bin(TAC_OP op, 
                                           std::shared_ptr<EXP> exp1, 
                                           std::shared_ptr<EXP> exp2) {
    // Handle pointer arithmetic: pointer +/- integer should scale by 4
    bool is_pointer_arithmetic = false;
    std::shared_ptr<EXP> pointer_exp = nullptr;
    std::shared_ptr<EXP> offset_exp = nullptr;
    
    if ((op == TAC_OP::ADD || op == TAC_OP::SUB) && exp1->place && exp2->place) {
        if (exp1->place->is_pointer && !exp2->place->is_pointer) {
            // pointer + integer or pointer - integer
            is_pointer_arithmetic = true;
            pointer_exp = exp1;
            offset_exp = exp2;
        } else if (op == TAC_OP::ADD && !exp1->place->is_pointer && exp2->place->is_pointer) {
            // integer + pointer (only valid for ADD, not SUB)
            is_pointer_arithmetic = true;
            pointer_exp = exp2;
            offset_exp = exp1;
        }
    }
    
    if (is_pointer_arithmetic) {
        // Scale the offset by 4 (size of int/pointer in bytes)
        auto four = mk_const(4);
        auto four_exp = mk_exp(four, nullptr);
        
        // scaled_offset = offset * 4
        auto scaled_temp = mk_tmp(DATA_TYPE::INT);
        auto scaled_decl = mk_tac(TAC_OP::VAR, scaled_temp);
        scaled_decl->prev = join_tac(pointer_exp->code, offset_exp->code);
        
        auto scale_tac = mk_tac(TAC_OP::MUL, scaled_temp, offset_exp->place, four);
        scale_tac->prev = scaled_decl;
        
        // result = pointer +/- scaled_offset
        auto result_temp = mk_tmp(DATA_TYPE::INT);
        result_temp->is_pointer = true; // Result is also a pointer
        auto result_decl = mk_tac(TAC_OP::VAR, result_temp);
        result_decl->prev = scale_tac;
        
        auto result_tac = mk_tac(op, result_temp, pointer_exp->place, scaled_temp);
        result_tac->prev = result_decl;
        
        auto result = mk_exp(result_temp, result_tac);
        result->data_type = DATA_TYPE::INT;
        return result;
    }
    
    // Normal binary operation
    // Infer result type
    DATA_TYPE result_type = infer_binary_type(exp1->data_type, exp2->data_type);
    
    auto temp = mk_tmp(result_type);
    auto temp_decl = mk_tac(TAC_OP::VAR, temp);
    temp_decl->prev = join_tac(exp1->code, exp2->code);
    
    auto ret = mk_tac(op, temp, exp1->place, exp2->place);
    ret->prev = temp_decl;
    
    auto exp = mk_exp(temp, ret);
    exp->data_type = result_type;
    return exp;
}

std::shared_ptr<EXP> TACGenerator::do_un(TAC_OP op, std::shared_ptr<EXP> exp) {
    // Unary operations preserve the type
    DATA_TYPE result_type = exp->data_type;
    
    auto temp = mk_tmp(result_type);
    auto temp_decl = mk_tac(TAC_OP::VAR, temp);
    temp_decl->prev = exp->code;
    
    auto ret = mk_tac(op, temp, exp->place);
    ret->prev = temp_decl;
    
    auto result = mk_exp(temp, ret);
    result->data_type = result_type;
    return result;
}

std::shared_ptr<EXP> TACGenerator::do_call_ret(const std::string& name, std::shared_ptr<EXP> arglist) {
    auto func_sym = lookup_sym(name);
    DATA_TYPE return_type = DATA_TYPE::INT;
    
    if (func_sym != nullptr && func_sym->type == SYM_TYPE::FUNC) {
        return_type = func_sym->return_type;
        
        int param_count = 0;
        for (auto arg = arglist; arg != nullptr; arg = arg->next) {
            if (param_count < func_sym->param_types.size()) {
                if (!check_type_compatibility(arg->data_type, func_sym->param_types[param_count])) {
                    warning("Type mismatch in function call argument " + std::to_string(param_count + 1));
                }
            }
            param_count++;
        }
        
        if (param_count != func_sym->param_types.size()) {
            warning("Argument count mismatch in function call to " + name);
        }
    } else {
        warning("Function not declared: " + name);
    }
    
    auto ret = mk_tmp(return_type);
    auto code = mk_tac(TAC_OP::VAR, ret);
    
    for (auto arg = arglist; arg != nullptr; arg = arg->next) {
        code = join_tac(code, arg->code);
    }
    
    while (arglist != nullptr) {
        auto temp = mk_tac(TAC_OP::ACTUAL, arglist->place);
        temp->prev = code;
        code = temp;
        arglist = arglist->next;
    }
    
    auto func_name = std::make_shared<SYM>();
    func_name->type = SYM_TYPE::FUNC;
    func_name->name = name;
    
    auto temp = mk_tac(TAC_OP::CALL, ret, func_name);
    temp->prev = code;
    code = temp;
    
    auto exp = mk_exp(ret, code);
    exp->data_type = return_type;
    return exp;
}

void TACGenerator::enter_scope() {
    scope = SYM_SCOPE::LOCAL;
    sym_tab_local.clear();
}

void TACGenerator::leave_scope() {
    scope = SYM_SCOPE::GLOBAL;
    sym_tab_local.clear();
}

void TACGenerator::enter_loop(std::shared_ptr<SYM> break_label, std::shared_ptr<SYM> continue_label,
                              std::shared_ptr<SYM> loop_start_label) {
    LoopContext ctx;
    ctx.break_label = break_label;
    ctx.continue_label = continue_label;
    ctx.loop_start_label = loop_start_label;
    loop_stack.push_back(ctx);

    context_type_stack.push_back(ContextType::LOOP);
}

void TACGenerator::enter_switch(std::shared_ptr<SYM> break_label, std::shared_ptr<SYM> default_label) {
    switch_stack.push_back({break_label, default_label});
    context_type_stack.push_back(ContextType::SWITCH);
}

void TACGenerator::leave_loop() {
    if (!loop_stack.empty()) {
        loop_stack.pop_back();
        context_type_stack.pop_back();
    }
}

bool TACGenerator::in_loop() const {
    return !loop_stack.empty() && context_type_stack.back() == ContextType::LOOP;
}

bool TACGenerator::in_switch() const {
    return !switch_stack.empty() && context_type_stack.back() == ContextType::SWITCH;
}

std::shared_ptr<TAC> TACGenerator::do_break() {
    if (in_loop()) {
        auto& ctx = loop_stack.back();
        return mk_tac(TAC_OP::GOTO, ctx.break_label);
    }
    if(in_switch()){
        auto& ctx = switch_stack.back();
        return mk_tac(TAC_OP::GOTO, ctx.break_label);
    }

    error("break statement outside of loop or switch");
    return nullptr;
}

std::shared_ptr<TAC> TACGenerator::do_continue() {
    if (!in_loop()) {
        error("continue statement outside of loop");
        return nullptr;
    }
    
    auto& ctx = loop_stack.back();
    return mk_tac(TAC_OP::GOTO, ctx.continue_label);
}

void TACGenerator::print_tac(std::ostream& os) {
    auto cur = tac_first;
    
    while (cur != nullptr) {
        os << cur->to_string() << std::endl;
        cur = cur->next;
    }
}

bool TACGenerator::check_type_compatibility(DATA_TYPE t1, DATA_TYPE t2) {
    if (t1 == DATA_TYPE::UNDEF || t2 == DATA_TYPE::UNDEF) {
        return true;  // Allow undefined types (error already reported)
    }
    
    // INT and CHAR are compatible (with implicit conversion)
    if ((t1 == DATA_TYPE::INT || t1 == DATA_TYPE::CHAR) &&
        (t2 == DATA_TYPE::INT || t2 == DATA_TYPE::CHAR)) {
        return true;
    }
    
    return t1 == t2;
}

DATA_TYPE TACGenerator::infer_binary_type(DATA_TYPE t1, DATA_TYPE t2) {
    if (t1 == DATA_TYPE::UNDEF || t2 == DATA_TYPE::UNDEF) {
        return DATA_TYPE::INT;  // Default to INT
    }
    
    // If either is INT, result is INT
    if (t1 == DATA_TYPE::INT || t2 == DATA_TYPE::INT) {
        return DATA_TYPE::INT;
    }
    
    // Both CHAR -> CHAR
    if (t1 == DATA_TYPE::CHAR && t2 == DATA_TYPE::CHAR) {
        return DATA_TYPE::CHAR;
    }
    
    return DATA_TYPE::INT;
}

void TACGenerator::check_assignment_type(std::shared_ptr<SYM> var, std::shared_ptr<EXP> exp) {
    if (!check_type_compatibility(var->data_type, exp->data_type)) {
        warning("Type mismatch in assignment: " + 
                data_type_to_string(var->data_type) + " = " + 
                data_type_to_string(exp->data_type));
    }
}

void TACGenerator::check_return_type(std::shared_ptr<EXP> exp) {
    if (current_func == nullptr) {
        return;
    }
    
    if (!check_type_compatibility(current_func->return_type, exp->data_type)) {
        warning("Return type mismatch: expected " + 
                data_type_to_string(current_func->return_type) + 
                ", got " + data_type_to_string(exp->data_type));
    }
}

void TACGenerator::error(const std::string& msg) {
    std::cerr << "TAC Error: " << msg << std::endl;
}

void TACGenerator::warning(const std::string& msg) {
    std::cerr << "TAC Warning: " << msg << std::endl;
}

void TACGenerator::print_symbol_table(std::ostream& os) {
    os << "\n=== Global Symbol Table ===" << std::endl;
    for (const auto& pair : sym_tab_global) {
        const auto& sym = pair.second;
        os << std::setw(6) << sym->name << " : ";
        
        switch (sym->type) {
            case SYM_TYPE::VAR:
                os << "VAR[" << data_type_to_string(sym->data_type) << "]";
                if (sym->offset >= 0) {
                    os << " @" << sym->offset;
                }
                break;
            case SYM_TYPE::FUNC:
                os << "FUNC[" << data_type_to_string(sym->return_type) << "](";
                for (size_t i = 0; i < sym->param_types.size(); ++i) {
                    if (i > 0) os << ", ";
                    os << data_type_to_string(sym->param_types[i]);
                }
                os << ")";
                break;
            case SYM_TYPE::CONST_INT:
                os << "CONST_INT = " << std::get<int>(sym->value);
                break;
            case SYM_TYPE::CONST_CHAR:
                os << "CONST_CHAR = '" << std::get<char>(sym->value) << "'";
                break;
            case SYM_TYPE::TEXT:
                os << "TEXT @L" << sym->label;
                break;
            default:
                os << "UNKNOWN";
        }
        os << std::endl;
    }
    
    //sym_tab_local不会打印，因为离开函数作用域后局部符号表会被清空
    os << std::endl;
}

void TACGenerator::link_tac(std::shared_ptr<TAC> tac) {
    if (!tac) return;
    
    // Link this TAC to the current tac_last
    if (tac_last != nullptr) {
        tac->prev = tac_last;
    }
    
    // Update tac_last to point to this new TAC
    tac_last = tac;
}

// ============ Struct Support ============

std::shared_ptr<SYM> TACGenerator::declare_struct_type(const std::string& name,
                                                       std::shared_ptr<StructTypeMetadata> metadata) {
    auto it = struct_types.find(name);
    if (it != struct_types.end()) {
        error("Struct type already declared: " + name);
        return it->second;
    }
    
    auto struct_sym = std::make_shared<SYM>();
    struct_sym->type = SYM_TYPE::STRUCT_TYPE;
    struct_sym->name = name;
    struct_sym->scope = SYM_SCOPE::GLOBAL;
    
    // Store the complete metadata instead of flattened fields
    struct_sym->struct_metadata = metadata;
    
    // Calculate size if not already done
    if (metadata) {
        metadata->calculate_size(struct_types);
    }
    
    struct_types[name] = struct_sym;
    return struct_sym;
}

std::shared_ptr<SYM> TACGenerator::get_struct_type(const std::string& name) {
    auto it = struct_types.find(name);
    if (it == struct_types.end()) {
        error("Struct type not declared: " + name);
        return nullptr;
    }
    return it->second;
}

// ============ Pointer Operations ============

std::shared_ptr<EXP> TACGenerator::do_address_of(std::shared_ptr<EXP> exp) {
    if (!exp || !exp->place) {
        error("Invalid expression for address-of operation");
        return nullptr;
    }
    
    // Address-of operation: &var
    // Result type is INT (pointer)
    auto temp = mk_tmp(DATA_TYPE::INT);
    temp->is_pointer = true; // Mark as pointer for pointer arithmetic
    auto temp_decl = mk_tac(TAC_OP::VAR, temp);
    temp_decl->prev = exp->code;
    
    auto addr_tac = mk_tac(TAC_OP::ADDR, temp, exp->place);
    addr_tac->prev = temp_decl;
    
    auto result = mk_exp(temp, addr_tac);
    result->data_type = DATA_TYPE::INT; // Pointer type represented as INT
    return result;
}

std::shared_ptr<EXP> TACGenerator::do_dereference(std::shared_ptr<EXP> exp) {
    if (!exp || !exp->place) {
        error("Invalid expression for dereference operation");
        return nullptr;
    }

    DATA_TYPE deref_type = DATA_TYPE::INT;
    if (exp->place->is_pointer && exp->place->base_type != DATA_TYPE::UNDEF) {
        deref_type = exp->place->base_type;
    }
    
    auto temp = mk_tmp(deref_type);
    auto temp_decl = mk_tac(TAC_OP::VAR, temp);
    temp_decl->prev = exp->code;
    
    auto deref_tac = mk_tac(TAC_OP::LOAD_PTR, temp, exp->place);
    deref_tac->prev = temp_decl;
    
    auto result = mk_exp(temp, deref_tac);
    result->data_type = deref_type;
    return result;
}

std::shared_ptr<TAC> TACGenerator::do_pointer_assign(std::shared_ptr<EXP> ptr_exp, 
                                                      std::shared_ptr<EXP> value_exp) {
    if (!ptr_exp || !ptr_exp->place || !value_exp || !value_exp->place) {
        error("Invalid pointer assignment");
        return nullptr;
    }
    
    // *ptr = value
    auto code = join_tac(ptr_exp->code, value_exp->code);
    auto store_tac = mk_tac(TAC_OP::STORE_PTR, ptr_exp->place, value_exp->place);
    store_tac->prev = code;
    
    return store_tac;
}
