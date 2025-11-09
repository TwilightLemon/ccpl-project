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
    
    class TACOptimizer
    {
    private:
        std::shared_ptr<TAC> tac_first;
        BlockBuilder block_builder;
        
        // 数据流分析结果：每个基本块的入口和出口
        std::unordered_map<std::shared_ptr<BasicBlock>, DataFlowInfo> block_in;
        std::unordered_map<std::shared_ptr<BasicBlock>, DataFlowInfo> block_out;
        
        void warning(const std::string& module,const std::string& msg)const;
        bool get_const_value(std::shared_ptr<SYM> sym, int& value)const;
        std::shared_ptr<SYM> make_const(int value)const;
        
        // 获取指令定义的变量
        std::shared_ptr<SYM> get_def(std::shared_ptr<TAC> tac) const;
        
        // 获取指令使用的变量
        std::vector<std::shared_ptr<SYM>> get_uses(std::shared_ptr<TAC> tac) const;
        
        // 数据流分析
        void compute_reaching_definitions(const std::vector<std::shared_ptr<BasicBlock>>& blocks);
        void compute_live_variables(const std::vector<std::shared_ptr<BasicBlock>>& blocks);
        void compute_constant_propagation(const std::vector<std::shared_ptr<BasicBlock>>& blocks);
        
        // 局部优化（基本块内）
        bool local_constant_folding(std::shared_ptr<TAC> tac);
        bool local_copy_propagation(std::shared_ptr<TAC> tac);
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
        bool is_loop_invariant(std::shared_ptr<TAC> tac, std::shared_ptr<BasicBlock> loop_header,
                               const std::unordered_set<std::shared_ptr<BasicBlock>>& loop_blocks) const;
        void find_loop_blocks(std::shared_ptr<BasicBlock> header,
                             std::unordered_set<std::shared_ptr<BasicBlock>>& loop_blocks) const;
        std::string get_expression_key(std::shared_ptr<TAC> tac) const;
        
    public:
        TACOptimizer(std::shared_ptr<TAC> first)
            : tac_first(first), block_builder(first) {}
        void optimize();
    };
}