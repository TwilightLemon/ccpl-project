#pragma once
#include <string>
#include <variant>
#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>
#include <unordered_set>
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
        std::shared_ptr<SYM> make_const(int value)const;
        
        // 局部优化（基本块内）
        bool local_constant_folding(std::shared_ptr<TAC> tac,std::shared_ptr<TAC> end);
        bool local_copy_propagation(std::shared_ptr<TAC> tac,std::shared_ptr<TAC> end);
        bool local_chain_folding(std::shared_ptr<TAC> tac,std::shared_ptr<TAC> end);
        void optimize_block_local(std::shared_ptr<BasicBlock> block);
        
        // 全局优化（基于数据流分析）
        bool global_constant_propagation(const std::vector<std::shared_ptr<BasicBlock>>& blocks);
        bool global_dead_code_elimination(const std::vector<std::shared_ptr<BasicBlock>>& blocks);
        
        // 高级优化
        bool common_subexpression_elimination(std::shared_ptr<BasicBlock> block);
        bool loop_invariant_code_motion(std::shared_ptr<BasicBlock> loop_header);
        bool simplify_control_flow(std::shared_ptr<TAC> tac_start);
        bool eliminate_unreachable_code(std::vector<std::shared_ptr<BasicBlock>>& blocks);
        bool eliminate_unused_var_declarations(std::shared_ptr<TAC> tac_start);
        
        // 辅助函数
        bool is_loop_header(std::shared_ptr<BasicBlock> block) const;
        void find_loop_blocks(std::shared_ptr<BasicBlock> header,
                             std::unordered_set<std::shared_ptr<BasicBlock>>& loop_blocks) const;
        std::string get_expression_key(std::shared_ptr<TAC> tac) const;
        
    public:
        TACOptimizer(std::shared_ptr<TAC> first)
            : tac_first(first), block_builder(first) {}
        void optimize();
    };
}