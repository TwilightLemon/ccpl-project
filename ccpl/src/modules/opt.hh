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
        BlockBuilder block_builder;
        
        void warning(const std::string& module,const std::string& msg)const;
        bool get_const_value(std::shared_ptr<SYM> sym, int& value)const;
        std::shared_ptr<SYM> make_const(int value)const;

        bool constant_folding(std::shared_ptr<TAC> tac);
        bool constant_propagation(std::shared_ptr<TAC> tac);
        bool copy_propagation(std::shared_ptr<TAC> tac);
        bool dead_code_elimination(std::shared_ptr<TAC> tac);
        void optimize_block(std::shared_ptr<BasicBlock> block);
    public:
        TACOptimizer(std::shared_ptr<TAC> first)
            : tac_first(first), block_builder(first) {}
        void optimize();
    };
}