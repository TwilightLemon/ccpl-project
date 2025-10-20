#include "tac.hh"
#include <sstream>
#include <iomanip>

// Global TAC generator instance
TACGenerator tac_gen;

TACGenerator::TACGenerator() 
    : scope(SYM_SCOPE::GLOBAL), next_tmp(0), next_label(1), 
      tac_first(nullptr), tac_last(nullptr) {}

void TACGenerator::init() {
    scope = SYM_SCOPE::GLOBAL;
    next_tmp = 0;
    next_label = 1;
    sym_tab_global.clear();
    sym_tab_local.clear();
    tac_first = nullptr;
    tac_last = nullptr;
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

std::shared_ptr<SYM> TACGenerator::mk_var(const std::string& name) {
    auto sym = lookup_sym(name);
    
    if (sym != nullptr) {
        error("Variable already declared: " + name);
        return sym;
    }
    
    sym = std::make_shared<SYM>();
    sym->type = SYM_TYPE::VAR;
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

std::shared_ptr<SYM> TACGenerator::mk_tmp() {
    std::string name = "t" + std::to_string(next_tmp++);
    auto sym = std::make_shared<SYM>();
    sym->type = SYM_TYPE::VAR;
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

std::shared_ptr<SYM> TACGenerator::mk_const(int value) {
    std::string name = std::to_string(value);
    
    auto it = sym_tab_global.find(name);
    if (it != sym_tab_global.end()) {
        return it->second;
    }
    
    auto sym = std::make_shared<SYM>();
    sym->type = SYM_TYPE::INT;
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
    sym->type = SYM_TYPE::CHAR;
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

std::shared_ptr<SYM> TACGenerator::declare_func(const std::string& name) {
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
    func_sym->name = name;
    func_sym->scope = SYM_SCOPE::GLOBAL;
    
    sym_tab_global[name] = func_sym;
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

std::shared_ptr<TAC> TACGenerator::declare_var(const std::string& name) {
    return mk_tac(TAC_OP::VAR, mk_var(name));
}

std::shared_ptr<TAC> TACGenerator::declare_para(const std::string& name) {
    return mk_tac(TAC_OP::FORMAL, mk_var(name));
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
    
    // Update tac_last
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
        return mk_tac(TAC_OP::RETURN);
    }
    
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
    auto label1_name = "L" + std::to_string(next_label++);
    auto label2_name = "L" + std::to_string(next_label++);
    
    auto label1 = mk_tac(TAC_OP::LABEL, mk_label(label1_name));
    auto label2 = mk_tac(TAC_OP::LABEL, mk_label(label2_name));
    
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
    auto label_name = "L" + std::to_string(next_label++);
    auto label = mk_tac(TAC_OP::LABEL, mk_label(label_name));
    auto code = mk_tac(TAC_OP::GOTO, label->a);
    
    code->prev = stmt;
    
    return join_tac(label, do_if(exp, code));
}

std::shared_ptr<TAC> TACGenerator::do_for(std::shared_ptr<TAC> init,
                                           std::shared_ptr<EXP> cond,
                                           std::shared_ptr<TAC> update,
                                           std::shared_ptr<TAC> body) {
    // for (init; cond; update) body
    // =>
    // init
    // label:
    //   ifz cond goto end
    //   body
    //   update
    //   goto label
    // end:
    
    auto label_name = "L" + std::to_string(next_label++);
    auto end_label_name = "L" + std::to_string(next_label++);
    
    auto label = mk_tac(TAC_OP::LABEL, mk_label(label_name));
    auto end_label = mk_tac(TAC_OP::LABEL, mk_label(end_label_name));
    
    auto ifz = mk_tac(TAC_OP::IFZ, end_label->a, cond->place);
    auto goto_label = mk_tac(TAC_OP::GOTO, label->a);
    
    // Build: init -> label -> cond code -> ifz -> body -> update -> goto -> end
    auto result = join_tac(init, label);
    result = join_tac(result, cond->code);
    ifz->prev = result;
    result = join_tac(ifz, body);
    result = join_tac(result, update);
    goto_label->prev = result;
    end_label->prev = goto_label;
    
    return end_label;
}

std::shared_ptr<TAC> TACGenerator::do_call(const std::string& name, std::shared_ptr<EXP> arglist) {
    std::shared_ptr<TAC> code = nullptr;
    
    // Generate code for all arguments
    for (auto arg = arglist; arg != nullptr; arg = arg->next) {
        code = join_tac(code, arg->code);
    }
    
    // Generate ACTUAL instructions in reverse order
    while (arglist != nullptr) {
        auto temp = mk_tac(TAC_OP::ACTUAL, arglist->place);
        temp->prev = code;
        code = temp;
        arglist = arglist->next;
    }
    
    // Generate CALL instruction
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
    auto temp = mk_tmp();
    auto temp_decl = mk_tac(TAC_OP::VAR, temp);
    temp_decl->prev = join_tac(exp1->code, exp2->code);
    
    auto ret = mk_tac(op, temp, exp1->place, exp2->place);
    ret->prev = temp_decl;
    
    return mk_exp(temp, ret);
}

std::shared_ptr<EXP> TACGenerator::do_un(TAC_OP op, std::shared_ptr<EXP> exp) {
    auto temp = mk_tmp();
    auto temp_decl = mk_tac(TAC_OP::VAR, temp);
    temp_decl->prev = exp->code;
    
    auto ret = mk_tac(op, temp, exp->place);
    ret->prev = temp_decl;
    
    return mk_exp(temp, ret);
}

std::shared_ptr<EXP> TACGenerator::do_call_ret(const std::string& name, std::shared_ptr<EXP> arglist) {
    auto ret = mk_tmp();
    auto code = mk_tac(TAC_OP::VAR, ret);
    
    // Generate code for all arguments
    for (auto arg = arglist; arg != nullptr; arg = arg->next) {
        code = join_tac(code, arg->code);
    }
    
    // Generate ACTUAL instructions
    while (arglist != nullptr) {
        auto temp = mk_tac(TAC_OP::ACTUAL, arglist->place);
        temp->prev = code;
        code = temp;
        arglist = arglist->next;
    }
    
    // Generate CALL instruction with return value
    auto func_name = std::make_shared<SYM>();
    func_name->type = SYM_TYPE::FUNC;
    func_name->name = name;
    
    auto temp = mk_tac(TAC_OP::CALL, ret, func_name);
    temp->prev = code;
    code = temp;
    
    return mk_exp(ret, code);
}

void TACGenerator::enter_scope() {
    scope = SYM_SCOPE::LOCAL;
    sym_tab_local.clear();
}

void TACGenerator::leave_scope() {
    scope = SYM_SCOPE::GLOBAL;
    sym_tab_local.clear();
}

std::string TACGenerator::sym_to_string(std::shared_ptr<SYM> s) {
    if (s == nullptr) return "NULL";
    
    switch (s->type) {
        case SYM_TYPE::VAR:
        case SYM_TYPE::FUNC:
        case SYM_TYPE::LABEL:
            return s->name;
            
        case SYM_TYPE::TEXT: {
            std::ostringstream oss;
            oss << "L" << s->label;
            return oss.str();
        }
        
        case SYM_TYPE::INT:
            if (std::holds_alternative<int>(s->value)) {
                return std::to_string(std::get<int>(s->value));
            }
            return s->name;
            
        case SYM_TYPE::CHAR:
            if (std::holds_alternative<char>(s->value)) {
                return "'" + std::string(1, std::get<char>(s->value)) + "'";
            }
            return s->name;
            
        default:
            return "?";
    }
}

std::string TACGenerator::tac_to_string(std::shared_ptr<TAC> t) {
    if (t == nullptr) return "";
    
    std::ostringstream oss;
    
    switch (t->op) {
        case TAC_OP::ADD:
            oss << sym_to_string(t->a) << " = " << sym_to_string(t->b) << " + " << sym_to_string(t->c);
            break;
        case TAC_OP::SUB:
            oss << sym_to_string(t->a) << " = " << sym_to_string(t->b) << " - " << sym_to_string(t->c);
            break;
        case TAC_OP::MUL:
            oss << sym_to_string(t->a) << " = " << sym_to_string(t->b) << " * " << sym_to_string(t->c);
            break;
        case TAC_OP::DIV:
            oss << sym_to_string(t->a) << " = " << sym_to_string(t->b) << " / " << sym_to_string(t->c);
            break;
        case TAC_OP::EQ:
            oss << sym_to_string(t->a) << " = (" << sym_to_string(t->b) << " == " << sym_to_string(t->c) << ")";
            break;
        case TAC_OP::NE:
            oss << sym_to_string(t->a) << " = (" << sym_to_string(t->b) << " != " << sym_to_string(t->c) << ")";
            break;
        case TAC_OP::LT:
            oss << sym_to_string(t->a) << " = (" << sym_to_string(t->b) << " < " << sym_to_string(t->c) << ")";
            break;
        case TAC_OP::LE:
            oss << sym_to_string(t->a) << " = (" << sym_to_string(t->b) << " <= " << sym_to_string(t->c) << ")";
            break;
        case TAC_OP::GT:
            oss << sym_to_string(t->a) << " = (" << sym_to_string(t->b) << " > " << sym_to_string(t->c) << ")";
            break;
        case TAC_OP::GE:
            oss << sym_to_string(t->a) << " = (" << sym_to_string(t->b) << " >= " << sym_to_string(t->c) << ")";
            break;
        case TAC_OP::NEG:
            oss << sym_to_string(t->a) << " = -" << sym_to_string(t->b);
            break;
        case TAC_OP::COPY:
            oss << sym_to_string(t->a) << " = " << sym_to_string(t->b);
            break;
        case TAC_OP::GOTO:
            oss << "goto " << sym_to_string(t->a);
            break;
        case TAC_OP::IFZ:
            oss << "ifz " << sym_to_string(t->b) << " goto " << sym_to_string(t->a);
            break;
        case TAC_OP::LABEL:
            oss << "label " << sym_to_string(t->a);
            break;
        case TAC_OP::VAR:
            oss << "var " << sym_to_string(t->a);
            break;
        case TAC_OP::FORMAL:
            oss << "formal " << sym_to_string(t->a);
            break;
        case TAC_OP::ACTUAL:
            oss << "actual " << sym_to_string(t->a);
            break;
        case TAC_OP::CALL:
            if (t->a != nullptr) {
                oss << sym_to_string(t->a) << " = call " << sym_to_string(t->b);
            } else {
                oss << "call " << sym_to_string(t->b);
            }
            break;
        case TAC_OP::RETURN:
            if (t->a != nullptr) {
                oss << "return " << sym_to_string(t->a);
            } else {
                oss << "return";
            }
            break;
        case TAC_OP::INPUT:
            oss << "input " << sym_to_string(t->a);
            break;
        case TAC_OP::OUTPUT:
            oss << "output " << sym_to_string(t->a);
            break;
        case TAC_OP::BEGINFUNC:
            oss << "begin";
            break;
        case TAC_OP::ENDFUNC:
            oss << "end";
            break;
        default:
            oss << "undef";
            break;
    }
    
    return oss.str();
}

void TACGenerator::print_tac(std::ostream& os) {
    auto cur = tac_first;
    int line = 1;
    
    while (cur != nullptr) {
        os << tac_to_string(cur) << std::endl;
        cur = cur->next;
    }
}

void TACGenerator::error(const std::string& msg) {
    std::cerr << "TAC Error: " << msg << std::endl;
}
