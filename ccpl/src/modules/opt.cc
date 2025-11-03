#include "opt.hh"
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
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

bool TACOptimizer::constant_folding(std::shared_ptr<TAC> tac)
{
    bool changed = false;
    auto current = tac;

    while (current != nullptr)
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
                        valid = false; // 除零错误，丢给虚拟机玩吧
                        warning("Constant Folding", "Division by zero!!!");
                    }
                    break;
                default:
                    valid = false;
                    break;
                }

                if (valid)
                {
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

bool TACOptimizer::constant_propagation(std::shared_ptr<TAC> tac)
{
    bool changed = false;
    std::unordered_map<std::shared_ptr<SYM>, int> const_map;

    auto current = tac;
    while (current != nullptr)
    {
        // 先替换使用处的变量为常量
        // 但是对于指针操作（ADDR, LOAD_PTR, STORE_PTR），不能进行常量传播
        bool is_pointer_op = (current->op == TAC_OP::ADDR || 
                              current->op == TAC_OP::LOAD_PTR || 
                              current->op == TAC_OP::STORE_PTR);
        
        // 替换 b 操作数（但不替换ADDR的操作数）
        if (current->b && current->b->type == SYM_TYPE::VAR && !is_pointer_op)
        {
            auto it = const_map.find(current->b);
            if (it != const_map.end())
            {
                current->b = make_const(it->second);
                changed = true;
            }
        }

        // 替换 c 操作数
        if (current->c && current->c->type == SYM_TYPE::VAR && !is_pointer_op)
        {
            auto it = const_map.find(current->c);
            if (it != const_map.end())
            {
                current->c = make_const(it->second);
                changed = true;
            }
        }

        // 对于 RETURN、OUTPUT、IFZ、ACTUAL 等指令，替换 a 操作数
        if (current->op == TAC_OP::RETURN ||
            current->op == TAC_OP::OUTPUT ||
            current->op == TAC_OP::IFZ ||
            current->op == TAC_OP::ACTUAL)
        {
            if (current->a && current->a->type == SYM_TYPE::VAR)
            {
                auto it = const_map.find(current->a);
                if (it != const_map.end())
                {
                    current->a = make_const(it->second);
                    changed = true;
                }
            }
        }


        // 如果是 COPY CONST，记录左值为常量
        if (current->op == TAC_OP::COPY && current->a && current->b)
        {
            int val;
            if (get_const_value(current->b, val))
            {
                if (current->a->type == SYM_TYPE::VAR)
                {
                    const_map[current->a] = val;
                }
            }
            else
            {
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
                  current->op == TAC_OP::CALL || current->op == TAC_OP::INPUT ||
                  current->op == TAC_OP::STORE_PTR || current->op == TAC_OP::LOAD_PTR))
        {
            const_map.erase(current->a);
        }
        // 对于 STORE_PTR，清除被存储变量的常量信息
        else if(current -> a && current->b && current->op ==TAC_OP::ADDR){
            const_map.erase(current->b);
        }

        current = current->next;
    }

    return changed;
}

bool TACOptimizer::copy_propagation(std::shared_ptr<TAC> tac)
{
    bool changed = false;

    // 建立变量到变量的映射表
    std::unordered_map<std::shared_ptr<SYM>, std::shared_ptr<SYM>> copy_map;

    auto current = tac;
    while (current != nullptr)
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
        // 但是对于指针操作（ADDR, LOAD_PTR, STORE_PTR），不能进行拷贝传播
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

bool TACOptimizer::dead_code_elimination(std::shared_ptr<TAC> tac)
{
    bool changed = false;

    // 收集所有被使用的变量
    std::unordered_set<std::shared_ptr<SYM>> used_vars;

    auto current = tac;
    while (current != nullptr)
    {
        // 记录作为操作数使用的变量
        if (current->b)
        {
            used_vars.insert(current->b);
        }
        if (current->c)
        {
            used_vars.insert(current->c);
        }

        // 特殊指令（如 OUTPUT, RETURN, IFZ）中的目标也被认为是使用
        if (current->op == TAC_OP::OUTPUT ||
            current->op == TAC_OP::RETURN ||
            current->op == TAC_OP::IFZ ||
            current->op == TAC_OP::ACTUAL)
        {
            if (current->a)
            {
                used_vars.insert(current->a);
            }
        }

        current = current->next;
    }

    // 删除未使用的赋值
    current = tac;
    while (current != nullptr)
    {
        auto next = current->next;

        if ((current->op == TAC_OP::COPY ||
             current->op == TAC_OP::ADD || current->op == TAC_OP::SUB ||
             current->op == TAC_OP::MUL || current->op == TAC_OP::DIV ||
             current->op == TAC_OP::NEG ||
             current->op == TAC_OP::EQ || current->op == TAC_OP::NE ||
             current->op == TAC_OP::LT || current->op == TAC_OP::LE ||
             current->op == TAC_OP::GT || current->op == TAC_OP::GE) ||
            current->op == TAC_OP::VAR &&
                current->a && current->a->type == SYM_TYPE::VAR)
        {

            if (used_vars.find(current->a) == used_vars.end())
            {
                // 删除该指令
                if (current->prev)
                {
                    current->prev->next = current->next;
                }
                if (current->next)
                {
                    current->next->prev = current->prev;
                }
                changed = true;
            }
        }

        current = next;
    }

    return changed;
}

void TACOptimizer::optimize_block(std::shared_ptr<BasicBlock> block)
{
    auto start = block->start;
    bool changed = true;
    int iter = 0;
    const int MAX_ITER = 10; //最多迭代次数

    while(changed && iter < MAX_ITER)
    {
        changed = false;
        iter++;

        if (constant_propagation(start))
        {
            changed = true;
        }
        if (constant_folding(start))
        {
            changed = true;
        }
        if (copy_propagation(start))
        {
            changed = true;
        }
        if (dead_code_elimination(start))
        {
            changed = true;
        }
    }
}

void TACOptimizer::optimize()
{
    block_builder.build();
    block_builder.print_basic_blocks(std::clog);
    auto blocks = block_builder.get_basic_blocks();

    for (auto &block : blocks)
    {
        optimize_block(block);
    }
}