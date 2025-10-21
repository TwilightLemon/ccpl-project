#pragma once
#include <string>
#include <variant>
#include <memory>
#include <functional>
#include <vector>
#include "abstraction/block_struct.hh"
#include "block.hh"

namespace twlm::ccpl::modules
{
    using namespace twlm::ccpl::abstraction;
    
    class TACOptimizer
    {
    private:
        std::shared_ptr<TAC> tac_first;
        std::function<void(std::ostream &os)> print_tac;
        BlockBuilder block_builder;
    public:
        TACOptimizer(std::shared_ptr<TAC> first, std::function<void(std::ostream &os)> print_func)
            : tac_first(first), print_tac(print_func), block_builder(first) {}
        void optimize();
    };
}