#include "opt.hh"
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
using namespace twlm::ccpl::modules;

static void warning(const std::string& module,const std::string& msg) {
    std::cerr << "AST Opt[" << module << "] Warning: " << msg << std::endl;
}

// 辅助函数：将TAC指令转为字符串（简化版本）
static std::string tac_op_to_string(TAC_OP op) {
    switch (op) {
        case TAC_OP::ADD: return "ADD";
        case TAC_OP::SUB: return "SUB";
        case TAC_OP::MUL: return "MUL";
        case TAC_OP::DIV: return "DIV";
        case TAC_OP::EQ: return "EQ";
        case TAC_OP::NE: return "NE";
        case TAC_OP::LT: return "LT";
        case TAC_OP::LE: return "LE";
        case TAC_OP::GT: return "GT";
        case TAC_OP::GE: return "GE";
        case TAC_OP::NEG: return "NEG";
        case TAC_OP::COPY: return "COPY";
        case TAC_OP::GOTO: return "GOTO";
        case TAC_OP::IFZ: return "IFZ";
        case TAC_OP::BEGINFUNC: return "BEGINFUNC";
        case TAC_OP::ENDFUNC: return "ENDFUNC";
        case TAC_OP::LABEL: return "LABEL";
        case TAC_OP::VAR: return "VAR";
        case TAC_OP::FORMAL: return "FORMAL";
        case TAC_OP::ACTUAL: return "ACTUAL";
        case TAC_OP::CALL: return "CALL";
        case TAC_OP::RETURN: return "RETURN";
        case TAC_OP::INPUT: return "INPUT";
        case TAC_OP::OUTPUT: return "OUTPUT";
        default: return "UNDEF";
    }
}

static std::string sym_name(std::shared_ptr<SYM> s) {
    if (!s) return "NULL";
    return s->name;
}

static bool get_const_value(std::shared_ptr<SYM> sym, int& value) {
    if (!sym) return false;
    if (sym->type == SYM_TYPE::CONST_INT) {
        value = std::get<int>(sym->value);
        return true;
    }
    return false;
}

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
        // 二元运算
        if (current->op == TAC_OP::ADD || current->op == TAC_OP::SUB ||
            current->op == TAC_OP::MUL || current->op == TAC_OP::DIV) {
            
            int val_b, val_c;
            if (get_const_value(current->b, val_b) && 
                get_const_value(current->c, val_c)) {
                
                int result = 0;
                bool valid = true;

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
        // 比较运算
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
        
        // 如果是 COPY CONST，记录左值为常量
        if (current->op == TAC_OP::COPY && current->a && current->b) {
            int val;
            if (get_const_value(current->b, val)) {
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

// 判断一条指令是否是领导指令（基础块的起始）
bool TACOptimizer::is_leader(std::shared_ptr<TAC> tac, std::shared_ptr<TAC> prev) {
    if (!tac) return false;
    
    // ENDFUNC 永远不应该成为基础块的leader
    // 它应该是前一个基础块的结束指令
    if (tac->op == TAC_OP::ENDFUNC) return false;
    
    // 1. 程序的第一条指令
    if (tac == tac_first) return true;
    
    // 2. 标签指令（跳转目标，包括函数标签）
    // LABEL总是新基础块的开始
    if (tac->op == TAC_OP::LABEL) return true;
    
    // 3. 跳转指令的下一条指令
    if (prev) {
        // IFZ 后的下一条指令是新的基础块（fallthrough分支）
        if (prev->op == TAC_OP::IFZ) {
            return true;
        }
        
        // GOTO后的下一条指令是新的基础块（虽然通常不可达）
        if (prev->op == TAC_OP::GOTO) {
            return true;
        }
        
        // RETURN后的下一条指令（但ENDFUNC除外，已在上面处理）
        if (prev->op == TAC_OP::RETURN) {
            return true;
        }
        
        // ENDFUNC后的下一条指令（通常是新函数的LABEL）
        if (prev->op == TAC_OP::ENDFUNC) {
            return true;
        }
    }
    
    return false;
}

// 根据标签名查找基础块
std::shared_ptr<BasicBlock> TACOptimizer::find_block_by_label(std::shared_ptr<SYM> label) {
    if (!label) return nullptr;
    
    for (auto& block : basic_blocks) {
        if (block->start && block->start->op == TAC_OP::LABEL && 
            block->start->a && block->start->a->name == label->name) {
            return block;
        }
    }
    return nullptr;
}

// 构建基础块
void TACOptimizer::build_basic_blocks() {
    basic_blocks.clear();
    
    if (!tac_first) return;
    
    // 第一遍：标记所有领导指令
    std::unordered_set<std::shared_ptr<TAC>> leaders;
    auto current = tac_first;
    std::shared_ptr<TAC> prev = nullptr;
    
    while (current) {
        if (is_leader(current, prev)) {
            leaders.insert(current);
        }
        prev = current;
        current = current->next;
    }
    
    // 第二遍：构建基础块
    // 基础块从一个leader开始，一直到下一个leader之前（不包括下一个leader）
    int block_id = 0;
    current = tac_first;
    std::shared_ptr<BasicBlock> current_block = nullptr;
    prev = nullptr;
    
    while (current) {
        // 如果当前指令是领导指令，创建新的基础块
        if (leaders.find(current) != leaders.end()) {
            // 结束上一个基础块（设置end为前一条指令）
            if (current_block && prev) {
                current_block->end = prev;
            }
            
            // 创建新基础块
            current_block = std::make_shared<BasicBlock>(block_id++, current);
            basic_blocks.push_back(current_block);
        }
        
        prev = current;
        current = current->next;
    }
    
    // 结束最后一个基础块
    if (current_block && prev) {
        current_block->end = prev;
    }
}

// 构建控制流图（CFG）
void TACOptimizer::build_cfg() {
    if (basic_blocks.empty()) return;
    
    // 清空所有基础块的前驱和后继
    for (auto& block : basic_blocks) {
        block->predecessors.clear();
        block->successors.clear();
    }
    
    // 为每个基础块建立前驱和后继关系
    for (size_t i = 0; i < basic_blocks.size(); ++i) {
        auto block = basic_blocks[i];
        auto end_instr = block->end;
        
        if (!end_instr) continue;
        
        // 根据结束指令的类型确定后继
        switch (end_instr->op) {
            case TAC_OP::GOTO: {
                // 无条件跳转：只有一个后继（跳转目标）
                auto target = find_block_by_label(end_instr->a);
                if (target) {
                    block->successors.push_back(target);
                    target->predecessors.push_back(block);
                }
                break;
            }
            
            case TAC_OP::IFZ: {
                // 条件跳转：有两个后继（跳转目标和顺序执行）
                // 1. 跳转目标
                auto target = find_block_by_label(end_instr->a);
                if (target) {
                    block->successors.push_back(target);
                    target->predecessors.push_back(block);
                }
                
                // 2. 顺序执行的下一个基础块（fallthrough）
                if (i + 1 < basic_blocks.size()) {
                    auto next_block = basic_blocks[i + 1];
                    block->successors.push_back(next_block);
                    next_block->predecessors.push_back(block);
                }
                break;
            }
            
            case TAC_OP::RETURN:
            case TAC_OP::ENDFUNC:
                // 函数返回或结束：没有后继（函数出口）
                break;
            
            case TAC_OP::LABEL:
                // 如果基础块只有一个LABEL指令，它会顺序执行到下一个块
                // （这通常不应该发生，但为了健壮性处理）
                if (block->start == block->end && i + 1 < basic_blocks.size()) {
                    auto next_block = basic_blocks[i + 1];
                    block->successors.push_back(next_block);
                    next_block->predecessors.push_back(block);
                }
                break;
            
            default:
                // 其他指令：顺序执行到下一个基础块
                if (i + 1 < basic_blocks.size()) {
                    auto next_block = basic_blocks[i + 1];
                    // 检查下一个块是否是新函数的开始（避免跨函数连接）
                    if (next_block->start && next_block->start->op == TAC_OP::LABEL) {
                        // 检查当前块的最后一条指令之后是否紧接着ENDFUNC
                        auto check = end_instr->next;
                        while (check && check != next_block->start) {
                            if (check->op == TAC_OP::ENDFUNC) {
                                // 有ENDFUNC，不连接
                                return;
                            }
                            check = check->next;
                        }
                    }
                    block->successors.push_back(next_block);
                    next_block->predecessors.push_back(block);
                }
                break;
        }
    }
}

// 打印基础块信息
void TACOptimizer::print_basic_blocks(std::ostream &os) {
    os << "\n========== Basic Blocks ==========" << std::endl;
    os << "Total blocks: " << basic_blocks.size() << std::endl << std::endl;
    
    for (auto& block : basic_blocks) {
        os << "Block " << block->id << ":" << std::endl;
        
        // 打印前驱
        os << "  Predecessors: ";
        if (block->predecessors.empty()) {
            os << "none";
        } else {
            for (size_t i = 0; i < block->predecessors.size(); ++i) {
                if (i > 0) os << ", ";
                os << block->predecessors[i]->id;
            }
        }
        os << std::endl;
        
        // 打印后继
        os << "  Successors: ";
        if (block->successors.empty()) {
            os << "none";
        } else {
            for (size_t i = 0; i < block->successors.size(); ++i) {
                if (i > 0) os << ", ";
                os << block->successors[i]->id;
            }
        }
        os << std::endl;
        
        // 打印指令
        os << "  Instructions:" << std::endl;
        auto instr = block->start;
        while (instr) {
            os << "    " << tac_op_to_string(instr->op);
            if (instr->a) os << " " << sym_name(instr->a);
            if (instr->b) os << " " << sym_name(instr->b);
            if (instr->c) os << " " << sym_name(instr->c);
            os << std::endl;
            
            if (instr == block->end) break;
            instr = instr->next;
        }
        os << std::endl;
    }
    os << "==================================" << std::endl;
}

void TACOptimizer::optimize() {
    // 首先构建基础块和控制流图
    std::clog << "\n========== Building Basic Blocks ==========" << std::endl;
    build_basic_blocks();
    build_cfg();
    print_basic_blocks(std::clog);
    
    // 迭代优化直到不再有改变
    bool changed = true;
    int iteration = 0;
    const int MAX_ITERATIONS = 10; // 防止无限循环
    
    while (changed && iteration < MAX_ITERATIONS) {
        std::clog << "Optimization Iteration: " << iteration + 1 << std::endl;
        print_tac(std::clog);

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