#pragma once
#include <string>
#include <variant>
#include <memory>
#include <functional>
#include "abstraction/tac_struct.hh"

namespace twlm::ccpl::modules
{
    using namespace twlm::ccpl::abstraction;
    class TACOptimizer
    {
    private:
        std::shared_ptr<TAC> tac_first;
        std::function<void(std::ostream &os)> print_tac;

    public:
        TACOptimizer(std::shared_ptr<TAC> first, std::function<void(std::ostream &os)> print_func)
            : tac_first(first), print_tac(print_func) {}
        void optimize();
    };
}