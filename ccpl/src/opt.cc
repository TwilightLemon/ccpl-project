#include "opt.hh"
#include <unordered_map>
#include <unordered_set>

static void warning(const std::string& module,const std::string& msg) {
    std::cerr << "AST Opt[" << module << "] Warning: " << msg << std::endl;
}

// 获取符号的常量值（如果是常量）
static bool get_const_value(std::shared_ptr<SYM> sym, int& value) {
    if (!sym) return false;
    if (sym->type == SYM_TYPE::CONST_INT) {
        value = std::get<int>(sym->value);
        return true;
    }
    return false;
}

// 创建常量符号
static std::shared_ptr<SYM> make_const(int value) {
    auto const_sym = std::make_shared<SYM>();
    const_sym->type = SYM_TYPE::CONST_INT;
    const_sym->value = value;
    const_sym->data_type = DATA_TYPE::INT;
    return const_sym;
}

//常量折叠：计算编译期可确定的常量表达式
bool constant_folding(std::shared_ptr<TAC> tac) {
    bool changed = false;
    auto current = tac;
    
    while (current != nullptr) {
        // 处理二元运算
        if (current->op == TAC_OP::ADD || current->op == TAC_OP::SUB ||
            current->op == TAC_OP::MUL || current->op == TAC_OP::DIV) {
            
            int val_b, val_c;
            if (get_const_value(current->b, val_b) && 
                get_const_value(current->c, val_c)) {
                
                int result = 0;
                bool valid = true;
                
                // 执行常量折叠
                switch (current->op) {
                    case TAC_OP::ADD:
                        result = val_b + val_c;
                        break;
                    case TAC_OP::SUB:
                        result = val_b - val_c;
                        break;
                    case TAC_OP::MUL:
                        result = val_b * val_c;
                        break;
                    case TAC_OP::DIV:
                        if (val_c != 0) {
                            result = val_b / val_c;
                        } else {
                            valid = false; // 除零错误，丢给虚拟机玩吧
                            warning("Constant Folding","Division by zero!!!");
                        }
                        break;
                    default:
                        valid = false;
                        break;
                }
                
                if (valid) {
                    // 转换为赋值指令
                    current->op = TAC_OP::COPY;
                    current->b = make_const(result);
                    current->c = nullptr;
                    changed = true;
                }
            }
        }
        // 处理比较运算
        else if (current->op == TAC_OP::EQ || current->op == TAC_OP::NE ||
                 current->op == TAC_OP::LT || current->op == TAC_OP::LE ||
                 current->op == TAC_OP::GT || current->op == TAC_OP::GE) {
            
            int val_b, val_c;
            if (get_const_value(current->b, val_b) && 
                get_const_value(current->c, val_c)) {
                
                int result = 0;
                
                switch (current->op) {
                    case TAC_OP::EQ:
                        result = (val_b == val_c) ? 1 : 0;
                        break;
                    case TAC_OP::NE:
                        result = (val_b != val_c) ? 1 : 0;
                        break;
                    case TAC_OP::LT:
                        result = (val_b < val_c) ? 1 : 0;
                        break;
                    case TAC_OP::LE:
                        result = (val_b <= val_c) ? 1 : 0;
                        break;
                    case TAC_OP::GT:
                        result = (val_b > val_c) ? 1 : 0;
                        break;
                    case TAC_OP::GE:
                        result = (val_b >= val_c) ? 1 : 0;
                        break;
                    default:
                        break;
                }
                
                current->op = TAC_OP::COPY;
                current->b = make_const(result);
                current->c = nullptr;
                changed = true;
            }
        }
        // 处理一元运算（NEG）
        else if (current->op == TAC_OP::NEG) {
            int val_b;
            if (get_const_value(current->b, val_b)) {
                current->op = TAC_OP::COPY;
                current->b = make_const(-val_b);
                current->c = nullptr;
                changed = true;
            }
        }
        
        current = current->next;
    }
    
    return changed;
}

//常量传播：将已知常量值传播到使用处
bool constant_propagation(std::shared_ptr<TAC> tac) {
    bool changed = false;
    
    // 建立变量到常量值的映射表
    std::unordered_map<std::shared_ptr<SYM>, int> const_map;
    
    auto current = tac;
    while (current != nullptr) {
        // 先替换使用处的变量为常量（在记录新定义之前）
        // 替换 b 操作数
        if (current->b && current->b->type == SYM_TYPE::VAR) {
            auto it = const_map.find(current->b);
            if (it != const_map.end()) {
                current->b = make_const(it->second);
                changed = true;
            }
        }
        
        // 替换 c 操作数
        if (current->c && current->c->type == SYM_TYPE::VAR) {
            auto it = const_map.find(current->c);
            if (it != const_map.end()) {
                current->c = make_const(it->second);
                changed = true;
            }
        }
        
        // 对于 RETURN、OUTPUT、IFZ、ACTUAL 等指令，替换 a 操作数
        if (current->op == TAC_OP::RETURN || 
            current->op == TAC_OP::OUTPUT ||
            current->op == TAC_OP::IFZ ||
            current->op == TAC_OP::ACTUAL) {
            if (current->a && current->a->type == SYM_TYPE::VAR) {
                auto it = const_map.find(current->a);
                if (it != const_map.end()) {
                    current->a = make_const(it->second);
                    changed = true;
                }
            }
        }
        
        // 如果是 COPY 指令，且右值是常量，记录左值为常量
        if (current->op == TAC_OP::COPY && current->a && current->b) {
            int val;
            if (get_const_value(current->b, val)) {
                // 只传播临时变量和普通变量
                if (current->a->type == SYM_TYPE::VAR) {
                    const_map[current->a] = val;
                }
            } else {
                // 如果赋值的右值不是常量，移除左值的常量信息
                const_map.erase(current->a);
            }
        }
        // 如果变量被重新赋值（作为目标），清除其常量信息
        else if (current->a && 
                 (current->op == TAC_OP::ADD || current->op == TAC_OP::SUB ||
                  current->op == TAC_OP::MUL || current->op == TAC_OP::DIV ||
                  current->op == TAC_OP::NEG ||
                  current->op == TAC_OP::EQ || current->op == TAC_OP::NE ||
                  current->op == TAC_OP::LT || current->op == TAC_OP::LE ||
                  current->op == TAC_OP::GT || current->op == TAC_OP::GE ||
                  current->op == TAC_OP::CALL || current->op == TAC_OP::INPUT)) {
            const_map.erase(current->a);
        }
        
        current = current->next;
    }
    
    return changed;
}

// 拷贝传播：将 a = b 形式的赋值中，用 b 替换后续对 a 的使用
bool copy_propagation(std::shared_ptr<TAC> tac) {
    bool changed = false;
    
    // 建立变量到变量的映射表
    std::unordered_map<std::shared_ptr<SYM>, std::shared_ptr<SYM>> copy_map;
    
    auto current = tac;
    while (current != nullptr) {
        // 处理简单的拷贝赋值 a = b（b不是常量）
        if (current->op == TAC_OP::COPY && current->a && current->b && 
            current->b->type == SYM_TYPE::VAR) {
            copy_map[current->a] = current->b;
        }
        // 如果变量被重新赋值，清除其拷贝信息
        else if (current->a) {
            copy_map.erase(current->a);
        }
        
        // 替换使用处

        if (current->b && current->b->type == SYM_TYPE::VAR) {
            auto it = copy_map.find(current->b);
            if (it != copy_map.end()) {
                current->b = it->second;
                changed = true;
            }
        }
        
        if (current->c && current->c->type == SYM_TYPE::VAR) {
            auto it = copy_map.find(current->c);
            if (it != copy_map.end()) {
                current->c = it->second;
                changed = true;
            }
        }
        
        current = current->next;
    }
    
    return changed;
}

// 死代码消除：删除结果未被使用的赋值语句
bool dead_code_elimination(std::shared_ptr<TAC> tac) {
    bool changed = false;
    
    // 收集所有被使用的变量
    std::unordered_set<std::shared_ptr<SYM>> used_vars;
    
    auto current = tac;
    while (current != nullptr) {
        // 记录作为操作数使用的变量
        if (current->b) {
            used_vars.insert(current->b);
        }
        if (current->c) {
            used_vars.insert(current->c);
        }
        
        // 特殊指令（如 OUTPUT, RETURN, IFZ）中的目标也被认为是使用
        if (current->op == TAC_OP::OUTPUT || 
            current->op == TAC_OP::RETURN ||
            current->op == TAC_OP::IFZ ||
            current->op == TAC_OP::ACTUAL) {
            if (current->a) {
                used_vars.insert(current->a);
            }
        }
        
        current = current->next;
    }
    
    // 删除未使用的赋值
    current = tac;
    while (current != nullptr) {
        auto next = current->next;
        
        // 如果是简单赋值且目标变量未被使用，可以删除
        if ((current->op == TAC_OP::COPY || 
             current->op == TAC_OP::ADD || current->op == TAC_OP::SUB ||
             current->op == TAC_OP::MUL || current->op == TAC_OP::DIV ||
             current->op == TAC_OP::NEG ||
             current->op == TAC_OP::EQ || current->op == TAC_OP::NE ||
             current->op == TAC_OP::LT || current->op == TAC_OP::LE ||
             current->op == TAC_OP::GT || current->op == TAC_OP::GE) ||
             current->op==TAC_OP::VAR &&
            current->a && current->a->type == SYM_TYPE::VAR) {
            
            if (used_vars.find(current->a) == used_vars.end()) {
                // 删除该指令
                if (current->prev) {
                    current->prev->next = current->next;
                }
                if (current->next) {
                    current->next->prev = current->prev;
                }
                changed = true;
            }
        }
        
        current = next;
    }
    
    return changed;
}

void TACOptimizer::optimize() {
    // 迭代优化直到不再有改变
    bool changed = true;
    int iteration = 0;
    const int MAX_ITERATIONS = 10; // 防止无限循环
    
    while (changed && iteration < MAX_ITERATIONS) {
        std::clog << "Optimization Iteration: " << iteration + 1 << std::endl;
        tac_gen.print_tac(std::clog);

        changed = false;
        iteration++;
        
        // 1. 常量传播
        if (constant_propagation(tac_first)) {
            changed = true;
        }
        
        // 2. 常量折叠
        if (constant_folding(tac_first)) {
            changed = true;
        }
        
        // 3. 拷贝传播
        if (copy_propagation(tac_first)) {
            changed = true;
        }
        
        // 4. 死代码消除
        if (dead_code_elimination(tac_first)) {
            changed = true;
        }
        std::clog <<std::endl;
    }
}