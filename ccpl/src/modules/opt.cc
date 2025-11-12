#include "opt.hh"
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include <queue>
#include <climits>
using namespace twlm::ccpl::modules;

void TACOptimizer::warning(const std::string &module, const std::string &msg) const
{
    std::cerr << "AST Opt[" << module << "] Warning: " << msg << std::endl;
}

bool TACOptimizer::get_const_value(std::shared_ptr<SYM> sym, int &value) const
{
    if (!sym)
        return false;
    if (sym->type == SYM_TYPE::CONST_INT && sym->data_type == DATA_TYPE::INT)
    {
        value = std::get<int>(sym->value);
        return true;
    }else if(sym->type == SYM_TYPE::CONST_CHAR && sym->data_type == DATA_TYPE::CHAR){
        value = std::get<char>(sym->value);
        return true;
    }
    return false;
}

std::shared_ptr<SYM> TACOptimizer::make_const(int value) const
{
    auto const_sym = std::make_shared<SYM>();
    const_sym->type = SYM_TYPE::CONST_INT;
    const_sym->value = value;
    const_sym->data_type = DATA_TYPE::INT;
    return const_sym;
}

// 获取指令定义的变量
std::shared_ptr<SYM> TACOptimizer::get_def(std::shared_ptr<TAC> tac) const
{
    if (!tac || !tac->a)
        return nullptr;
        
    // 这些指令会定义变量 a
    if (tac->op == TAC_OP::COPY || tac->op == TAC_OP::ADD || tac->op == TAC_OP::SUB ||
        tac->op == TAC_OP::MUL || tac->op == TAC_OP::DIV || tac->op == TAC_OP::NEG ||
        tac->op == TAC_OP::EQ || tac->op == TAC_OP::NE || tac->op == TAC_OP::LT ||
        tac->op == TAC_OP::LE || tac->op == TAC_OP::GT || tac->op == TAC_OP::GE ||
        tac->op == TAC_OP::CALL || tac->op == TAC_OP::INPUT ||
        tac->op == TAC_OP::ADDR || tac->op == TAC_OP::LOAD_PTR)
    {
        if (tac->a->type == SYM_TYPE::VAR)
            return tac->a;
    }
    
    return nullptr;
}

// 获取指令使用的变量
std::vector<std::shared_ptr<SYM>> TACOptimizer::get_uses(std::shared_ptr<TAC> tac) const
{
    std::vector<std::shared_ptr<SYM>> uses;
    if (!tac)
        return uses;
    
    // b 操作数
    if (tac->b && tac->b->type == SYM_TYPE::VAR)
    {
        uses.push_back(tac->b);
    }
    
    // c 操作数
    if (tac->c && tac->c->type == SYM_TYPE::VAR)
    {
        uses.push_back(tac->c);
    }
    
    // 特殊指令中的 a 操作数也是使用
    if (tac->op == TAC_OP::RETURN || tac->op == TAC_OP::OUTPUT ||
        tac->op == TAC_OP::IFZ || tac->op == TAC_OP::ACTUAL ||
        tac->op == TAC_OP::STORE_PTR)
    {
        if (tac->a && tac->a->type == SYM_TYPE::VAR)
        {
            uses.push_back(tac->a);
        }
    }
    
    return uses;
}

// 到达定值分析：计算每个基本块入口和出口的到达定值集合
void TACOptimizer::compute_reaching_definitions(const std::vector<std::shared_ptr<BasicBlock>>& blocks)
{
    // 初始化
    for (auto& block : blocks)
    {
        block_in[block].reaching_defs.clear();
        block_out[block].reaching_defs.clear();
    }
    
    // 收集所有变量的所有定值点
    std::unordered_map<std::shared_ptr<SYM>, std::unordered_set<std::shared_ptr<TAC>>> all_defs;
    for (auto& block : blocks)
    {
        auto tac = block->start;
        while (tac)
        {
            auto def = get_def(tac);
            if (def)
            {
                all_defs[def].insert(tac);
            }
            if (tac == block->end)
                break;
            tac = tac->next;
        }
    }
    
    // 迭代求解不动点
    bool changed = true;
    while (changed)
    {
        changed = false;
        
        for (auto& block : blocks)
        {
            // IN[B] = ∪ OUT[P] for all predecessors P
            std::unordered_map<std::shared_ptr<SYM>, std::unordered_set<std::shared_ptr<TAC>>> new_in;
            for (auto& pred : block->predecessors)
            {
                for (auto& [var, defs] : block_out[pred].reaching_defs)
                {
                    new_in[var].insert(defs.begin(), defs.end());
                }
            }
            
            // 检查 IN 是否改变
            if (new_in != block_in[block].reaching_defs)
            {
                block_in[block].reaching_defs = new_in;
                changed = true;
            }
            
            // OUT[B] = GEN[B] ∪ (IN[B] - KILL[B])
            auto new_out = block_in[block].reaching_defs;
            
            // 遍历块中的指令，更新 OUT
            auto tac = block->start;
            while (tac)
            {
                auto def = get_def(tac);
                if (def)
                {
                    // KILL：删除该变量的所有其他定值
                    new_out[def].clear();
                    // GEN：添加当前定值
                    new_out[def].insert(tac);
                }
                
                if (tac == block->end)
                    break;
                tac = tac->next;
            }
            
            // 检查 OUT 是否改变
            if (new_out != block_out[block].reaching_defs)
            {
                block_out[block].reaching_defs = new_out;
                changed = true;
            }
        }
    }
}

// 活跃变量分析：计算每个基本块入口和出口的活跃变量集合
void TACOptimizer::compute_live_variables(const std::vector<std::shared_ptr<BasicBlock>>& blocks)
{
    // 初始化
    for (auto& block : blocks)
    {
        block_in[block].live_vars.clear();
        block_out[block].live_vars.clear();
    }
    
    // 迭代求解不动点（逆向数据流）
    bool changed = true;
    while (changed)
    {
        changed = false;
        
        // 逆序遍历基本块
        for (auto it = blocks.rbegin(); it != blocks.rend(); ++it)
        {
            auto& block = *it;
            
            // OUT[B] = ∪ IN[S] for all successors S
            std::unordered_set<std::shared_ptr<SYM>> new_out;
            for (auto& succ : block->successors)
            {
                new_out.insert(block_in[succ].live_vars.begin(), 
                              block_in[succ].live_vars.end());
            }
            
            if (new_out != block_out[block].live_vars)
            {
                block_out[block].live_vars = new_out;
                changed = true;
            }
            
            // IN[B] = USE[B] ∪ (OUT[B] - DEF[B])
            auto new_in = block_out[block].live_vars;
            
            // 逆序遍历块中的指令
            std::vector<std::shared_ptr<TAC>> instructions;
            auto tac = block->start;
            while (tac)
            {
                instructions.push_back(tac);
                if (tac == block->end)
                    break;
                tac = tac->next;
            }
            
            for (auto it = instructions.rbegin(); it != instructions.rend(); ++it)
            {
                auto& instr = *it;
                
                // 移除定值变量
                auto def = get_def(instr);
                if (def)
                {
                    new_in.erase(def);
                }
                
                // 添加使用的变量
                auto uses = get_uses(instr);
                for (auto& use : uses)
                {
                    new_in.insert(use);
                }
            }
            
            if (new_in != block_in[block].live_vars)
            {
                block_in[block].live_vars = new_in;
                changed = true;
            }
        }
    }
}

// 常量传播分析：确定每个点上哪些变量是常量
void TACOptimizer::compute_constant_propagation(const std::vector<std::shared_ptr<BasicBlock>>& blocks)
{
    // 初始化：所有变量都是 TOP（未知）
    for (auto& block : blocks)
    {
        block_in[block].constants.clear();
        block_out[block].constants.clear();
    }
    
    std::queue<std::shared_ptr<BasicBlock>> worklist;
    std::unordered_set<std::shared_ptr<BasicBlock>> in_worklist;
    
    for (auto& block : blocks)
    {
        worklist.push(block);
        in_worklist.insert(block);
    }
    
    // 特殊值：用 INT_MIN 表示 BOTTOM（非常量）
    const int BOTTOM = INT_MIN;
    
    while (!worklist.empty())
    {
        auto block = worklist.front();
        worklist.pop();
        in_worklist.erase(block);
        
        // IN[B] = meet of OUT[P] for all predecessors P
        std::unordered_map<std::shared_ptr<SYM>, int> new_in;
        
        bool first = true;
        for (auto& pred : block->predecessors)
        {
            if (first)
            {
                new_in = block_out[pred].constants;
                first = false;
            }
            else
            {
                // Meet 操作：如果所有前驱的值相同则保持，否则为 BOTTOM
                std::unordered_set<std::shared_ptr<SYM>> all_vars;
                for (auto& [var, _] : new_in)
                    all_vars.insert(var);
                for (auto& [var, _] : block_out[pred].constants)
                    all_vars.insert(var);
                
                for (auto& var : all_vars)
                {
                    bool in_new = new_in.find(var) != new_in.end();
                    bool in_pred = block_out[pred].constants.find(var) != block_out[pred].constants.end();
                    
                    if (in_new && in_pred)
                    {
                        if (new_in[var] != block_out[pred].constants[var])
                        {
                            new_in[var] = BOTTOM; // 不同的常量值
                        }
                    }
                    else if (in_pred)
                    {
                        new_in[var] = block_out[pred].constants[var];
                    }
                    // 如果只在 new_in 中，保持不变
                }
            }
        }
        
        block_in[block].constants = new_in;
        
        // OUT[B] = transfer(IN[B])
        auto new_out = new_in;
        
        auto tac = block->start;
        while (tac)
        {
            auto def = get_def(tac);
            
            if (def)
            {
                // 尝试计算定值的常量值
                int result = BOTTOM;
                bool is_const = false;
                
                if (tac->op == TAC_OP::COPY)
                {
                    int val;
                    if (get_const_value(tac->b, val))
                    {
                        result = val;
                        is_const = true;
                    }
                    else if (tac->b && tac->b->type == SYM_TYPE::VAR)
                    {
                        auto it = new_out.find(tac->b);
                        if (it != new_out.end() && it->second != BOTTOM)
                        {
                            result = it->second;
                            is_const = true;
                        }
                    }
                }
                else if (tac->op == TAC_OP::ADD || tac->op == TAC_OP::SUB ||
                         tac->op == TAC_OP::MUL || tac->op == TAC_OP::DIV)
                {
                    int val_b, val_c;
                    bool has_b = false, has_c = false;
                    
                    if (get_const_value(tac->b, val_b))
                    {
                        has_b = true;
                    }
                    else if (tac->b && tac->b->type == SYM_TYPE::VAR)
                    {
                        auto it = new_out.find(tac->b);
                        if (it != new_out.end() && it->second != BOTTOM)
                        {
                            val_b = it->second;
                            has_b = true;
                        }
                    }
                    
                    if (get_const_value(tac->c, val_c))
                    {
                        has_c = true;
                    }
                    else if (tac->c && tac->c->type == SYM_TYPE::VAR)
                    {
                        auto it = new_out.find(tac->c);
                        if (it != new_out.end() && it->second != BOTTOM)
                        {
                            val_c = it->second;
                            has_c = true;
                        }
                    }
                    
                    if (has_b && has_c)
                    {
                        switch (tac->op)
                        {
                        case TAC_OP::ADD:
                            result = val_b + val_c;
                            is_const = true;
                            break;
                        case TAC_OP::SUB:
                            result = val_b - val_c;
                            is_const = true;
                            break;
                        case TAC_OP::MUL:
                            result = val_b * val_c;
                            is_const = true;
                            break;
                        case TAC_OP::DIV:
                            if (val_c != 0)
                            {
                                result = val_b / val_c;
                                is_const = true;
                            }
                            break;
                        default:
                            break;
                        }
                    }
                }
                
                if (is_const)
                {
                    new_out[def] = result;
                }
                else
                {
                    new_out[def] = BOTTOM; // 非常量
                }
            }
            
            if (tac == block->end)
                break;
            tac = tac->next;
        }
        
        // 如果 OUT 改变，将后继加入工作列表
        if (new_out != block_out[block].constants)
        {
            block_out[block].constants = new_out;
            
            for (auto& succ : block->successors)
            {
                if (in_worklist.find(succ) == in_worklist.end())
                {
                    worklist.push(succ);
                    in_worklist.insert(succ);
                }
            }
        }
    }
}

// 局部常量折叠（只处理立即数常量）
bool TACOptimizer::local_constant_folding(std::shared_ptr<TAC> tac,std::shared_ptr<TAC> end)
{
    bool changed = false;
    auto current = tac;

    while (current != end->next)
    {
        // 二元运算
        if (current->op == TAC_OP::ADD || current->op == TAC_OP::SUB ||
            current->op == TAC_OP::MUL || current->op == TAC_OP::DIV)
        {
            int val_b, val_c;
            if (get_const_value(current->b, val_b) &&
                get_const_value(current->c, val_c))
            {
                int result = 0;
                bool valid = true;

                switch (current->op)
                {
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
                    if (val_c != 0)
                    {
                        result = val_b / val_c;
                    }
                    else
                    {
                        valid = false;
                        warning("Constant Folding", "Division by zero!!!");
                    }
                    break;
                default:
                    valid = false;
                    break;
                }

                if (valid)
                {
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
                 current->op == TAC_OP::GT || current->op == TAC_OP::GE)
        {
            int val_b, val_c;
            if (get_const_value(current->b, val_b) &&
                get_const_value(current->c, val_c))
            {
                int result = 0;

                switch (current->op)
                {
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
        else if (current->op == TAC_OP::NEG)
        {
            int val_b;
            if (get_const_value(current->b, val_b))
            {
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

// 局部拷贝传播
bool TACOptimizer::local_copy_propagation(std::shared_ptr<TAC> tac,std::shared_ptr<TAC> end)
{
    bool changed = false;
    std::unordered_map<std::shared_ptr<SYM>, std::shared_ptr<SYM>> copy_map;

    auto current = tac;
    while (current != end->next)
    {
        // 处理简单的拷贝赋值 a = b（b不是常量）
        if (current->op == TAC_OP::COPY && current->a && current->b &&
            current->b->type == SYM_TYPE::VAR)
        {
            copy_map[current->a] = current->b;
        }
        // 如果变量被重新赋值，清除其拷贝信息
        else if (current->a)
        {
            copy_map.erase(current->a);
        }

        // 替换使用处
        bool is_pointer_op = (current->op == TAC_OP::ADDR || 
                              current->op == TAC_OP::LOAD_PTR || 
                              current->op == TAC_OP::STORE_PTR);

        if (current->b && current->b->type == SYM_TYPE::VAR && !is_pointer_op)
        {
            auto it = copy_map.find(current->b);
            if (it != copy_map.end())
            {
                current->b = it->second;
                changed = true;
            }
        }

        if (current->c && current->c->type == SYM_TYPE::VAR && !is_pointer_op)
        {
            auto it = copy_map.find(current->c);
            if (it != copy_map.end())
            {
                current->c = it->second;
                changed = true;
            }
        }

        current = current->next;
    }

    return changed;
}

// 局部优化（基本块内）
void TACOptimizer::optimize_block_local(std::shared_ptr<BasicBlock> block)
{
    // 安全检查：确保块有有效的指令
    if (!block->start || !block->end)
    {
        return;
    }
    
    auto start = block->start;
    auto end = block->end;
    bool changed = true;
    int iter = 0;
    const int MAX_ITER = 20;

    while(changed && iter < MAX_ITER)
    {
        changed = false;
        iter++;

        if (local_constant_folding(start, end))
        {
            changed = true;
        }
        if (local_copy_propagation(start, end))
        {
            changed = true;
        }
    }
}

// 全局常量传播（基于数据流分析）
bool TACOptimizer::global_constant_propagation(const std::vector<std::shared_ptr<BasicBlock>>& blocks)
{
    bool changed = false;
    const int BOTTOM = INT_MIN;
    
    for (auto& block : blocks)
    {
        auto& in_constants = block_in[block].constants;
        auto current_constants = in_constants;
        
        auto tac = block->start;
        while (tac)
        {
            // 替换使用的变量为常量
            bool is_pointer_op = (tac->op == TAC_OP::ADDR || 
                                  tac->op == TAC_OP::LOAD_PTR || 
                                  tac->op == TAC_OP::STORE_PTR);
            
            if (tac->b && tac->b->type == SYM_TYPE::VAR && !is_pointer_op)
            {
                auto it = current_constants.find(tac->b);
                if (it != current_constants.end() && it->second != BOTTOM)
                {
                    tac->b = make_const(it->second);
                    changed = true;
                }
            }
            
            if (tac->c && tac->c->type == SYM_TYPE::VAR && !is_pointer_op)
            {
                auto it = current_constants.find(tac->c);
                if (it != current_constants.end() && it->second != BOTTOM)
                {
                    tac->c = make_const(it->second);
                    changed = true;
                }
            }
            
            // 特殊指令
            if (tac->op == TAC_OP::RETURN || tac->op == TAC_OP::OUTPUT ||
                tac->op == TAC_OP::IFZ || tac->op == TAC_OP::ACTUAL)
            {
                if (tac->a && tac->a->type == SYM_TYPE::VAR)
                {
                    auto it = current_constants.find(tac->a);
                    if (it != current_constants.end() && it->second != BOTTOM)
                    {
                        tac->a = make_const(it->second);
                        changed = true;
                    }
                }
            }
            
            // 更新当前常量状态
            auto def = get_def(tac);
            if (def)
            {
                // 简化计算
                if (tac->op == TAC_OP::COPY)
                {
                    int val;
                    if (get_const_value(tac->b, val))
                    {
                        current_constants[def] = val;
                    }
                    else
                    {
                        current_constants[def] = BOTTOM;
                    }
                }
                else
                {
                    current_constants[def] = BOTTOM;
                }
            }
            
            if (tac == block->end)
                break;
            tac = tac->next;
        }
    }
    
    return changed;
}

// 全局死代码消除（基于活跃变量分析）
bool TACOptimizer::global_dead_code_elimination(const std::vector<std::shared_ptr<BasicBlock>>& blocks)
{
    bool changed = false;
    
    for (auto& block : blocks)
    {
        auto& out_live = block_out[block].live_vars;
        auto current_live = out_live;
        
        // 收集块内指令
        std::vector<std::shared_ptr<TAC>> instructions;
        auto tac = block->start;
        while (tac)
        {
            instructions.push_back(tac);
            if (tac == block->end)
                break;
            tac = tac->next;
        }
        
        // 逆序遍历，删除死代码
        for (auto it = instructions.rbegin(); it != instructions.rend(); ++it)
        {
            auto& instr = *it;
            auto def = get_def(instr);
            
            // 如果定义的变量不活跃，且指令没有副作用，可以删除
            if (def && current_live.find(def) == current_live.end())
            {
                // 检查是否有副作用（CALL, INPUT 等不能删除）
                if (instr->op != TAC_OP::CALL && instr->op != TAC_OP::INPUT &&
                    instr->op != TAC_OP::LOAD_PTR && instr->op != TAC_OP::STORE_PTR)
                {
                    // 保存指针后删除该指令
                    auto instr_prev = instr->prev;
                    auto instr_next = instr->next;
                    
                    if (instr_prev)
                    {
                        instr_prev->next = instr_next;
                    }
                    if (instr_next)
                    {
                        instr_next->prev = instr_prev;
                    }
                    
                    // 更新块的起始或结束指针
                    if (instr == block->start)
                    {
                        block->start = instr_next;
                    }
                    if (instr == block->end)
                    {
                        block->end = instr_prev;
                    }
                    
                    // 清空被删除指令的链接
                    instr->prev = nullptr;
                    instr->next = nullptr;
                    
                    changed = true;
                    continue;
                }
            }
            
            // 更新活跃变量集合
            if (def)
            {
                current_live.erase(def);
            }
            
            auto uses = get_uses(instr);
            for (auto& use : uses)
            {
                current_live.insert(use);
            }
        }
    }
    
    return changed;
}

// 判断是否是循环头
bool TACOptimizer::is_loop_header(std::shared_ptr<BasicBlock> block) const
{
    // 循环头的特征：有前驱在它之后（回边）
    for (const auto& pred : block->predecessors)
    {
        if (pred->id >= block->id)  // 简单判断：后继块指向前面的块
        {
            return true;
        }
    }
    return false;
}

// 查找循环体中的所有基本块
void TACOptimizer::find_loop_blocks(std::shared_ptr<BasicBlock> header,
                                    std::unordered_set<std::shared_ptr<BasicBlock>>& loop_blocks) const
{
    loop_blocks.clear();
    loop_blocks.insert(header);
    
    // 使用工作列表算法，从回边的源节点开始反向遍历
    std::queue<std::shared_ptr<BasicBlock>> worklist;
    
    for (const auto& pred : header->predecessors)
    {
        if (pred->id >= header->id)  // 回边
        {
            worklist.push(pred);
            loop_blocks.insert(pred);
        }
    }
    
    while (!worklist.empty())
    {
        auto block = worklist.front();
        worklist.pop();
        
        for (const auto& pred : block->predecessors)
        {
            if (loop_blocks.find(pred) == loop_blocks.end())
            {
                loop_blocks.insert(pred);
                worklist.push(pred);
            }
        }
    }
}

// 获取表达式的唯一键（用于CSE）
std::string TACOptimizer::get_expression_key(std::shared_ptr<TAC> tac) const
{
    if (!tac)
        return "";
    
    std::string key;
    
    switch (tac->op)
    {
    case TAC_OP::ADD:
    case TAC_OP::MUL:  // 交换律
        {
            std::string b_str = tac->b ? tac->b->to_string() : "";
            std::string c_str = tac->c ? tac->c->to_string() : "";
            if (b_str > c_str)
                std::swap(b_str, c_str);
            key = "ADD_MUL:" + b_str + "," + c_str;
        }
        break;
    case TAC_OP::SUB:
        key = "SUB:" + (tac->b ? tac->b->to_string() : "") + "," + 
              (tac->c ? tac->c->to_string() : "");
        break;
    case TAC_OP::DIV:
        key = "DIV:" + (tac->b ? tac->b->to_string() : "") + "," + 
              (tac->c ? tac->c->to_string() : "");
        break;
    case TAC_OP::LT:
    case TAC_OP::LE:
    case TAC_OP::GT:
    case TAC_OP::GE:
    case TAC_OP::EQ:
    case TAC_OP::NE:
        key = std::to_string(static_cast<int>(tac->op)) + ":" +
              (tac->b ? tac->b->to_string() : "") + "," + 
              (tac->c ? tac->c->to_string() : "");
        break;
    default:
        return "";
    }
    
    return key;
}

// 公共子表达式消除（基本块内）
bool TACOptimizer::common_subexpression_elimination(std::shared_ptr<BasicBlock> block)
{
    bool changed = false;
    
    // 表达式 -> 结果变量的映射
    std::unordered_map<std::string, std::shared_ptr<SYM>> expr_map;
    
    auto tac = block->start;
    while (tac)
    {
        std::string key = get_expression_key(tac);
        
        if (!key.empty() && tac->a && tac->a->type == SYM_TYPE::VAR)
        {
            auto it = expr_map.find(key);
            if (it != expr_map.end())
            {
                // 找到相同的表达式，替换为拷贝
                tac->op = TAC_OP::COPY;
                tac->b = it->second;    //it->second 是临时变量的符号
                tac->c = nullptr;
                changed = true;
            }
            else
            {
                // 记录新表达式
                expr_map[key] = tac->a;
            }
        }
        
        // 如果有变量被重新定值，使相关表达式失效
        auto def = get_def(tac);
        if (def)
        {
            // 移除包含该变量的表达式
            std::vector<std::string> to_remove;
            for (auto& [expr_key, var] : expr_map)
            {
                if (expr_key.find(def->to_string()) != std::string::npos)
                {
                    to_remove.push_back(expr_key);
                }
            }
            for (const auto& key : to_remove)
            {
                expr_map.erase(key);
            }
        }
        
        if (tac == block->end)
            break;
        tac = tac->next;
    }
    
    return changed;
}

// 循环不变量外提
bool TACOptimizer::loop_invariant_code_motion(std::shared_ptr<BasicBlock> loop_header)
{
    bool changed = false;

    std::unordered_set<std::shared_ptr<BasicBlock>> loop_blocks;
    find_loop_blocks(loop_header, loop_blocks);

    if (loop_blocks.size() <= 1)
        return false;

    std::vector<std::shared_ptr<TAC>> loop_instructions;
    std::unordered_map<std::shared_ptr<TAC>, std::shared_ptr<BasicBlock>> instr_block;
    std::unordered_map<std::string, std::vector<std::shared_ptr<TAC>>> defs_in_loop;
    std::unordered_map<std::string, std::shared_ptr<TAC>> var_decl_in_loop;

    auto record_instruction = [&](std::shared_ptr<BasicBlock> block, std::shared_ptr<TAC> tac) {
        instr_block[tac] = block;
        loop_instructions.push_back(tac);

        if (tac->op == TAC_OP::VAR && tac->a && tac->a->type == SYM_TYPE::VAR)
        {
            var_decl_in_loop[tac->a->name] = tac;
        }

        auto def = get_def(tac);
        if (def)
        {
            defs_in_loop[def->name].push_back(tac);
        }
    };

    for (auto& block : loop_blocks)
    {
        if (!block || block == loop_header || !block->start || !block->end)
            continue;

        auto tac = block->start;
        while (tac)
        {
            record_instruction(block, tac);
            if (tac == block->end)
                break;
            tac = tac->next;
        }
    }

    if (loop_instructions.empty())
        return false;

    auto is_supported_op = [](TAC_OP op) {
        switch (op)
        {
        case TAC_OP::ADD:
        case TAC_OP::SUB:
        case TAC_OP::MUL:
        case TAC_OP::DIV:
        case TAC_OP::LT:
        case TAC_OP::LE:
        case TAC_OP::GT:
        case TAC_OP::GE:
        case TAC_OP::EQ:
        case TAC_OP::NE:
        case TAC_OP::COPY:
            return true;
        default:
            return false;
        }
    };

    auto operand_ready = [&](const std::shared_ptr<SYM>& sym,
                             const std::unordered_set<std::shared_ptr<TAC>>& movable) -> bool {
        if (!sym)
            return true;

        if (sym->type == SYM_TYPE::CONST_INT || sym->type == SYM_TYPE::CONST_CHAR)
            return true;

        if (sym->type != SYM_TYPE::VAR)
            return false;

        auto it = defs_in_loop.find(sym->name);
        if (it == defs_in_loop.end())
            return true; // 定义在循环外

        for (auto& defining_instr : it->second)
        {
            if (movable.find(defining_instr) == movable.end())
                return false;
        }

        return true;
    };

    std::unordered_set<std::shared_ptr<TAC>> movable;
    bool progress = true;
    while (progress)
    {
        progress = false;

        for (auto& instr : loop_instructions)
        {
            if (movable.find(instr) != movable.end())
                continue;

            if (!is_supported_op(instr->op))
                continue;

            auto def = get_def(instr);
            if (!def)
                continue;

            auto def_it = defs_in_loop.find(def->name);
            if (def_it == defs_in_loop.end())
                continue;

            if (def_it->second.size() != 1)
                continue; // 在循环中有多次定值，不能外提

            if (!operand_ready(instr->b, movable))
                continue;

            if (!operand_ready(instr->c, movable))
                continue;

            movable.insert(instr);
            progress = true;
        }
    }

    if (movable.empty())
        return false;

    std::shared_ptr<BasicBlock> preheader = nullptr;
    for (auto& pred : loop_header->predecessors)
    {
        if (loop_blocks.find(pred) == loop_blocks.end())
        {
            preheader = pred;
            break;
        }
    }

    if (!preheader || !preheader->end)
        return false;

    std::vector<std::shared_ptr<TAC>> ordered_to_move;
    ordered_to_move.reserve(movable.size());
    for (auto& instr : loop_instructions)
    {
        if (movable.find(instr) != movable.end())
            ordered_to_move.push_back(instr);
    }

    std::unordered_set<std::string> moved_var_decls;

    auto remove_from_block = [&](std::shared_ptr<TAC> node) {
        if (!node)
            return;

        auto block_it = instr_block.find(node);
        if (block_it == instr_block.end())
            return;

        auto block = block_it->second;
        if (!block)
            return;

        if (node == block->start)
            block->start = node->next;
        if (node == block->end)
            block->end = node->prev;

        if (node->prev)
            node->prev->next = node->next;
        if (node->next)
            node->next->prev = node->prev;

        node->prev = nullptr;
        node->next = nullptr;

        if (!block->start)
            block->start = block->end;
    };

    auto insert_before = [&](std::shared_ptr<TAC> node, std::shared_ptr<TAC> position) {
        if (!node || !position)
            return;

        node->prev = position->prev;
        node->next = position;

        if (position->prev)
            position->prev->next = node;
        position->prev = node;

        if (preheader->start == position)
            preheader->start = node;

        instr_block[node] = preheader;
    };

    auto insertion_point = preheader->end;

    for (auto& instr : ordered_to_move)
    {
        auto def = get_def(instr);
        if (def)
        {
            auto decl_it = var_decl_in_loop.find(def->name);
            if (decl_it != var_decl_in_loop.end() &&
                moved_var_decls.insert(def->name).second)
            {
                auto decl = decl_it->second;
                remove_from_block(decl);
                insert_before(decl, insertion_point);
                changed = true;
            }
        }

        remove_from_block(instr);
        insert_before(instr, insertion_point);
        changed = true;
    }

    if (changed)
    {
        std::clog << "    Loop invariant code motion: found "
                  << ordered_to_move.size() << " invariant instructions" << std::endl;
    }

    return changed;
}

// 控制流简化：处理常量条件的分支
bool TACOptimizer::simplify_control_flow(std::shared_ptr<TAC> tac_start)
{
    bool changed = false;
    auto current = tac_start;
    
    while (current)
    {
        // 查找 IFZ 指令
        if (current->op == TAC_OP::IFZ && current->b)
        {
            int cond_value;
            if (get_const_value(current->b, cond_value))
            {
                // 条件是常量
                if (cond_value == 0)
                {
                    // ifz 0 goto L -> 总是跳转，转换为 goto L
                    current->op = TAC_OP::GOTO;
                    current->b = nullptr;
                    current->c = nullptr;
                    changed = true;
                    std::clog << "    Simplified: ifz 0 -> goto " << current->a->to_string() << std::endl;
                }
                else
                {
                    // ifz <非0> goto L -> 永不跳转，删除此指令
                    std::clog << "    Simplified: ifz " << cond_value << " -> removed (never jumps)" << std::endl;
                    
                    auto next = current->next;
                    auto prev = current->prev;
                    
                    // 删除该指令
                    if (prev)
                        prev->next = next;
                    if (next)
                        next->prev = prev;
                    
                    // 清空链接
                    current->prev = nullptr;
                    current->next = nullptr;
                    
                    current = next;
                    changed = true;
                    continue;
                }
            }
        }
        // 查找连续的 GOTO 指令
        else if (current->op == TAC_OP::GOTO && current->next && 
                 current->next->op == TAC_OP::LABEL)
        {
            // goto L; label L; -> 可以删除 goto
            if (current->a && current->next->a && 
                current->a->name == current->next->a->name)
            {
                std::clog << "    Removed redundant goto to next label" << std::endl;
                
                auto next = current->next;
                auto prev = current->prev;
                
                if (prev)
                    prev->next = next;
                if (next)
                    next->prev = prev;
                
                // 清空链接
                current->prev = nullptr;
                current->next = nullptr;
                
                current = next;
                changed = true;
                continue;
            }
        }
        
        current = current->next;
    }
    
    return changed;
}

// 不可达代码消除：删除永远不会执行的代码块
bool TACOptimizer::eliminate_unreachable_code(std::vector<std::shared_ptr<BasicBlock>>& blocks)
{
    bool changed = false;
    
    if (blocks.empty())
        return false;
    
    // 使用可达性分析
    std::unordered_set<std::shared_ptr<BasicBlock>> reachable;
    std::queue<std::shared_ptr<BasicBlock>> worklist;
    
    // 从第一个块开始（程序入口，多函数由于不启用优化，可认为block 0为入口）
    worklist.push(blocks[0]);
    reachable.insert(blocks[0]);
    
    while (!worklist.empty())
    {
        auto block = worklist.front();
        worklist.pop();
        
        for (auto& succ : block->successors)
        {
            if (reachable.find(succ) == reachable.end())
            {
                reachable.insert(succ);
                worklist.push(succ);
            }
        }
    }
    
    // 删除不可达的块
    std::vector<std::shared_ptr<BasicBlock>> new_blocks;
    for (auto& block : blocks)
    {
        if (reachable.find(block) != reachable.end())
        {
            new_blocks.push_back(block);
        }
        else
        {
            std::clog << "    Removing unreachable block " << block->id << std::endl;
            
            // 从 TAC 链表中删除该块的所有指令
            auto tac = block->start;
            while (tac)
            {
                auto next = (tac == block->end) ? nullptr : tac->next;
                auto prev = tac->prev;
                auto curr_next = tac->next;
                
                if (prev)
                    prev->next = curr_next;
                if (curr_next)
                    curr_next->prev = prev;
                
                // 清空被删除指令的链接
                tac->prev = nullptr;
                tac->next = nullptr;
                
                tac = next;
                if (!next)
                    break;
            }
            
            changed = true;
        }
    }
    
    if (changed)
    {
        blocks = new_blocks;
    }
    
    return changed;
}

// 消除未使用的变量声明（该方法不适用于多函数，因为没有考虑函数作用域和参数传递）
bool TACOptimizer::eliminate_unused_var_declarations(std::shared_ptr<TAC> tac_start)
{
    bool changed = false;
    
    // 收集所有被使用的变量（除了声明）
    std::unordered_set<std::string> used_vars;
    
    auto current = tac_start;
    while (current)
    {
        // 跳过 VAR 声明，收集其他地方使用的变量
        if (current->op != TAC_OP::VAR)
        {
            if (current->a && current->a->type == SYM_TYPE::VAR)
                used_vars.insert(current->a->name);
            if (current->b && current->b->type == SYM_TYPE::VAR)
                used_vars.insert(current->b->name);
            if (current->c && current->c->type == SYM_TYPE::VAR)
                used_vars.insert(current->c->name);
        }
        
        current = current->next;
    }
    
    // 删除未使用的 VAR 声明
    current = tac_start;
    while (current)
    {
        auto next = current->next;
        
        if (current->op == TAC_OP::VAR && current->a && current->a->type == SYM_TYPE::VAR)
        {
            // 检查该变量是否被使用
            if (used_vars.find(current->a->name) == used_vars.end())
            {
                std::clog << "    Removing unused var declaration: " << current->a->name << std::endl;
                
                auto prev = current->prev;
                auto curr_next = current->next;
                
                // 删除该声明
                if (prev)
                    prev->next = curr_next;
                if (curr_next)
                    curr_next->prev = prev;
                
                // 清空链接
                current->prev = nullptr;
                current->next = nullptr;
                
                changed = true;
            }
        }
        
        current = next;
    }
    
    return changed;
}

void TACOptimizer::optimize()
{
    // 构建控制流图
    block_builder.build();
    block_builder.print_basic_blocks(std::clog);
    auto blocks = block_builder.get_basic_blocks();
    
    if (blocks.empty())
        return;
    
    // 多轮迭代优化
    bool global_changed = true;
    int global_iter = 0;
    const int MAX_GLOBAL_ITER = 20;
    
    while (global_changed && global_iter < MAX_GLOBAL_ITER)
    {
        global_changed = false;
        global_iter++;
        
        std::clog << "\n=== Optimization Pass " << global_iter << " ===" << std::endl;
        
        // 1. 公共子表达式消除（每个基本块内）
        for (auto& block : blocks)
        {
            if (common_subexpression_elimination(block))
            {
                global_changed = true;
                std::clog << "  - CSE applied in block " << block->id << std::endl;
            }
        }
        
        // 2. 循环不变量外提
        for (auto& block : blocks)
        {
            if (is_loop_header(block))
            {
                if (loop_invariant_code_motion(block))
                {
                    global_changed = true;
                    std::clog << "  - LICM applied for loop at block " << block->id << std::endl;
                }
            }
        }

        // 3. 局部优化（基本块内）
        for (auto& block : blocks)
        {
            optimize_block_local(block);
        }
        
        // 4. 数据流分析
        compute_reaching_definitions(blocks);
        compute_live_variables(blocks);
        compute_constant_propagation(blocks);
        
        // 5. 全局优化
        if (global_constant_propagation(blocks))
        {
            global_changed = true;
            std::clog << "  - Global constant propagation applied" << std::endl;
        }
        
        // 再次进行局部常量折叠（处理新产生的常量）
        for (auto& block : blocks)
        {
            if (local_constant_folding(block->start,block->end))
            {
                global_changed = true;
            }
        }
        
        //消除未使用的变量声明
        if (global_dead_code_elimination(blocks))
        {
            global_changed = true;
            std::clog << "  - Dead code elimination applied" << std::endl;

            block_builder.build();
            blocks = block_builder.get_basic_blocks();
        }
        
        // 6. 控制流简化
        if (simplify_control_flow(tac_first))
        {
            global_changed = true;
            std::clog << "  - Control flow simplification applied" << std::endl;

            block_builder.build();
            blocks = block_builder.get_basic_blocks();
        }
        
        // 7. 不可达代码消除
        if (eliminate_unreachable_code(blocks))
        {
            global_changed = true;
            std::clog << "  - Unreachable code elimination applied" << std::endl;

            block_builder.build();
            blocks = block_builder.get_basic_blocks();
        }
    }
    
    // 最后一轮清理：删除未使用的变量声明（包括临时变量）
    if (eliminate_unused_var_declarations(tac_first))
    {
        std::clog << "  - Unused variable declarations eliminated" << std::endl;
    }
    
    std::clog << "\n=== Optimization completed in " << global_iter << " passes ===" << std::endl;
}