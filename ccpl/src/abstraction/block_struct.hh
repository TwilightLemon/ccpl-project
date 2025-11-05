#pragma once
#include <memory>
#include <vector>
#include "tac_struct.hh"

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
}