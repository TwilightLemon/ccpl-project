#include "block.hh"
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
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
