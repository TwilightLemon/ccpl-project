#include "block.hh"
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include <queue>
#include <climits>
using namespace twlm::ccpl::modules;

bool BlockBuilder::is_leader(std::shared_ptr<TAC> tac, std::shared_ptr<TAC> prev)
{
    if (!tac)
        return false;

    // ENDFUNC 是终止语句
    if (tac->op == TAC_OP::ENDFUNC)
        return false;

    if (tac == tac_first)
        return true;

    // LABEL 是入口语句
    if (tac->op == TAC_OP::LABEL)
        return true;

    // 检查上一条指令 prev -> tac
    if (prev)
    {
        // IFZ GOTO RETURN ENDFUNC 后的下一条指令是新的基础块
        if (prev->op == TAC_OP::IFZ || prev->op == TAC_OP::GOTO ||
            prev->op == TAC_OP::RETURN || prev->op == TAC_OP::ENDFUNC)
        {
            return true;
        }
    }
    return false;
}

void BlockBuilder::build_basic_blocks()
{
    basic_blocks.clear();
    if (!tac_first)
        return;

    // 先标记基础块的leader节点
    std::unordered_set<std::shared_ptr<TAC>> leaders;
    auto current = tac_first;
    std::shared_ptr<TAC> prev = nullptr;
    while (current)
    {
        if (is_leader(current, prev))
        {
            leaders.insert(current);
        }
        prev = current;
        current = current->next;
    }

    //划分基础块
    int block_id = 0;
    current = tac_first;
    std::shared_ptr<BasicBlock> current_block = nullptr;
    prev = nullptr;

    while (current)
    {
        if (leaders.find(current) != leaders.end())
        {
            // 结束上一个基础块（设置end为前一条指令）
            if (current_block && prev)
            {
                current_block->end = prev;
            }

            current_block = std::make_shared<BasicBlock>(block_id++, current);
            basic_blocks.push_back(current_block);
        }

        prev = current;
        current = current->next;
    }

    // 结束最后一个基础块
    if (current_block && prev)
    {
        current_block->end = prev;
    }
}

std::shared_ptr<BasicBlock> BlockBuilder::find_block_by_label(std::shared_ptr<SYM> label)
{
    if (!label)
        return nullptr;

    for (auto &block : basic_blocks)
    {
        if (block->start && block->start->op == TAC_OP::LABEL &&
            block->start->a && block->start->a->name == label->name)
        {
            return block;
        }
    }
    return nullptr;
}

void BlockBuilder::build_cfg()
{
    if (basic_blocks.empty())
        return;

    for (auto &block : basic_blocks)
    {
        block->predecessors.clear();
        block->successors.clear();
    }

    // 为每个基础块建立前驱和后继关系
    for (size_t i = 0; i < basic_blocks.size(); ++i)
    {
        auto block = basic_blocks[i];
        auto end_instr = block->end;

        if (!end_instr) throw std::runtime_error("Basic block has no end instruction");

        // 根据结束指令的类型确定后继
        switch (end_instr->op)
        {
        case TAC_OP::GOTO:
        {
            // 无条件跳转：只有一个后继（跳转目标）
            auto target = find_block_by_label(end_instr->a);
            if (target)
            {
                block->successors.push_back(target);
                target->predecessors.push_back(block);
            }
            break;
        }

        case TAC_OP::IFZ:
        {
            // 条件跳转：有两个后继（跳转目标和顺序执行）
            // 1. 跳转目标
            auto target = find_block_by_label(end_instr->a);
            if (target)
            {
                block->successors.push_back(target);
                target->predecessors.push_back(block);
            }

            // 2. 顺序执行的下一个基础块（fallthrough）
            if (i + 1 < basic_blocks.size())
            {
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
            if (block->start == block->end && i + 1 < basic_blocks.size())
            {
                auto next_block = basic_blocks[i + 1];
                block->successors.push_back(next_block);
                next_block->predecessors.push_back(block);
            }
            break;

        default:
            // 其他指令：顺序执行到下一个基础块
            if (i + 1 < basic_blocks.size())
            {
                auto next_block = basic_blocks[i + 1];
                // 检查下一个块是否是新函数的开始（避免跨函数连接）
                if (next_block->start && next_block->start->op == TAC_OP::LABEL)
                {
                    // 检查当前块的最后一条指令之后是否紧接着ENDFUNC
                    auto check = end_instr->next;
                    while (check && check != next_block->start)
                    {
                        if (check->op == TAC_OP::ENDFUNC)
                        {
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

void BlockBuilder::print_basic_blocks(std::ostream &os)
{
    os << "\n========== Basic Blocks ==========" << std::endl;
    os << "Total blocks: " << basic_blocks.size() << std::endl
       << std::endl;

    for (auto &block : basic_blocks)
    {
        os << "Block " << block->id << ":" << std::endl;

        os << "  Predecessors: ";
        if (block->predecessors.empty())
        {
            os << "none";
        }
        else
        {
            for (size_t i = 0; i < block->predecessors.size(); ++i)
            {
                if (i > 0)
                    os << ", ";
                os << block->predecessors[i]->id;
            }
        }
        os << std::endl;

        os << "  Successors: ";
        if (block->successors.empty())
        {
            os << "none";
        }
        else
        {
            for (size_t i = 0; i < block->successors.size(); ++i)
            {
                if (i > 0)
                    os << ", ";
                os << block->successors[i]->id;
            }
        }
        os << std::endl;

        os << "  Instructions:" << std::endl;
        auto instr = block->start;
        while (instr)
        {
            os << "    " << instr->to_string() << std::endl;

            if (instr == block->end)
                break;
            instr = instr->next;
        }
        os << std::endl;
    }
    os << "==================================" << std::endl;
}


// 到达定值分析：计算每个基本块入口和出口的到达定值集合
void BlockBuilder::compute_reaching_definitions()
{
    const auto& blocks=basic_blocks;
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
            auto def = tac->get_def();
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
                auto def = tac->get_def();
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
void BlockBuilder::compute_live_variables()
{
    const auto& blocks=basic_blocks;
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
                auto def = instr->get_def();
                if (def)
                {
                    new_in.erase(def);
                }
                
                // 添加使用的变量
                auto uses = instr->get_uses();
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
void BlockBuilder::compute_constant_propagation()
{
    const auto& blocks=basic_blocks;
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
            auto def = tac->get_def();
            
            if (def)
            {
                // 尝试计算定值的常量值
                int result = BOTTOM;
                bool is_const = false;
                
                if (tac->op == TAC_OP::COPY)
                {
                    int val;
                    if (tac->b->get_const_value(val))
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
                    
                    if (tac->b->get_const_value(val_b))
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
                    
                    if (tac->c->get_const_value(val_c))
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