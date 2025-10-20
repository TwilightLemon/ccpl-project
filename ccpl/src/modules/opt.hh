#pragma once
#include <string>
#include <variant>
#include <memory>
#include "abstraction/tac_struct.hh"

namespace twlm::ccpl::modules
{
    using namespace twlm::ccpl::abstraction;
    class TACOptimizer
    {
    private:
        std::shared_ptr<TAC> tac_first;

    public:
        TACOptimizer(std::shared_ptr<TAC> first)
            : tac_first(first) {}
        void optimize();
    };
}