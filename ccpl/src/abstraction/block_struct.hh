#pragma once
#include <string>
#include <variant>
#include <memory>
#include <vector>
#include "tac_struct.hh"

namespace twlm::ccpl::abstraction
{
    // 基础块结构
    struct BasicBlock
    {
        int id;                                      // 基础块ID
        std::shared_ptr<TAC> start;                  // 起始指令
        std::shared_ptr<TAC> end;                    // 结束指令
        std::vector<std::shared_ptr<BasicBlock>> predecessors;  // 前驱基础块
        std::vector<std::shared_ptr<BasicBlock>> successors;    // 后继基础块
        
        BasicBlock(int id, std::shared_ptr<TAC> start = nullptr)
            : id(id), start(start), end(nullptr) {}
    };
}