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

        // 数据流分析结果：每个基本块的入口和出口
        std::unordered_map<std::shared_ptr<BasicBlock>, DataFlowInfo> block_in;
        std::unordered_map<std::shared_ptr<BasicBlock>, DataFlowInfo> block_out;

        void build_basic_blocks();
        void build_cfg();
        bool is_leader(std::shared_ptr<TAC> tac, std::shared_ptr<TAC> prev);
        std::shared_ptr<BasicBlock> find_block_by_label(std::shared_ptr<SYM> label);

        // 数据流分析
        void compute_reaching_definitions();
        void compute_live_variables();
        void compute_constant_propagation();

    public:
        BlockBuilder(std::shared_ptr<TAC> first)
            : tac_first(first) {}
        void build(){
            build_basic_blocks();
            build_cfg();
        }
        void compute_data_flow(){
            compute_reaching_definitions();
            compute_live_variables();
            compute_constant_propagation();
        }
        BlockList get_basic_blocks() const { return basic_blocks; }
        const auto& get_block_in() const { return block_in; }
        const auto& get_block_out() const { return block_out; }
        void print_basic_blocks(std::ostream &os = std::cout);
    };
}