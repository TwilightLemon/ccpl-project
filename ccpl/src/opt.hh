#pragma once
#include <string>
#include <variant>
#include <memory>
#include "tac.hh"

class TACOptimizer{
private:
    std::shared_ptr<TAC> tac_first;
public:
    TACOptimizer(std::shared_ptr<TAC> first)
        : tac_first(first) {}
    void optimize();
};