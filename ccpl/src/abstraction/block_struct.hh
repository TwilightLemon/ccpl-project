#pragma once
#include <memory>
#include <vector>
#include "tac_struct.hh"
#include <unordered_map>
#include <unordered_set>

namespace twlm::ccpl::abstraction
{
    struct BasicBlock
    {
        int id;
        std::shared_ptr<TAC> start,end;
        std::vector<std::shared_ptr<BasicBlock>> predecessors,successors;
        
        BasicBlock(int id, std::shared_ptr<TAC> start = nullptr)
            : id(id), start(start), end(nullptr) {}
    };

    // 数据流分析结果
    struct DataFlowInfo
    {
        // 到达定值集合：每个变量 -> 可能的定值指令集合
        std::unordered_map<std::shared_ptr<SYM>, std::unordered_set<std::shared_ptr<TAC>>> reaching_defs;
        
        // 活跃变量集合
        std::unordered_set<std::shared_ptr<SYM>> live_vars;
        
        // 常量传播信息：变量 -> 常量值（如果确定是常量）
        std::unordered_map<std::shared_ptr<SYM>, int> constants;
    };
}