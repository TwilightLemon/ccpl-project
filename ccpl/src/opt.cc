#include "opt.hh"

//常量折叠
void constant_folding(std::shared_ptr<TAC> tac) {
    auto current = tac;
    while (current != nullptr) {
        //检查是否为二元操作
        if (current->op == TAC_OP::ADD || current->op == TAC_OP::SUB ||
            current->op == TAC_OP::MUL || current->op == TAC_OP::DIV) {
            
            auto sym_b = current->b;
            auto sym_c = current->c;
            
            //只优化CONST INT
            if (sym_b && sym_b->type == SYM_TYPE::CONST_INT &&
                sym_c && sym_c->type == SYM_TYPE::CONST_INT) {

                int val_b = std::get<int>(sym_b->value);
                int val_c = std::get<int>(sym_c->value);
                int result = 0;
                
                //执行常量折叠
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
                            //处理除以零的情况
                            throw std::runtime_error("Division by zero in constant folding");
                        }
                        break;
                    default:
                        break;
                }
                
                //创建新的常量符号
                auto folded_const = std::make_shared<SYM>();
                folded_const->type = SYM_TYPE::CONST_INT;
                folded_const->value = result;
                
                //更新当前TAC指令
                current->op = TAC_OP::COPY;
                current->b = folded_const;
                current->c = nullptr;
            }
        }
        current = current->next;
    }
}

//常量传播
void constant_propagation(std::shared_ptr<TAC> tac) {
    
}

void TACOptimizer::optimize() {
    constant_folding(tac_first);
}