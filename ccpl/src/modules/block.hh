#pragma once
#include <string>
#include <variant>
#include <memory>
#include <vector>
#include "abstraction/block_struct.hh"

namespace twlm::ccpl::modules
{
    using namespace twlm::ccpl::abstraction;
    using BlockList = std::vector<std::shared_ptr<BasicBlock>>;
    class BlockBuilder
    {
    private:
        std::shared_ptr<TAC> tac_first;
        BlockList basic_blocks;
        

        void build_basic_blocks();
        void build_cfg();
        bool is_leader(std::shared_ptr<TAC> tac, std::shared_ptr<TAC> prev);
        std::shared_ptr<BasicBlock> find_block_by_label(std::shared_ptr<SYM> label);

    public:
        BlockBuilder(std::shared_ptr<TAC> first)
            : tac_first(first) {}
        void build(){
            build_basic_blocks();
            build_cfg();
        }
        BlockList get_basic_blocks() const { return basic_blocks; }
        void print_basic_blocks(std::ostream &os = std::cout);
    };
}