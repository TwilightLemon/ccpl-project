# TAC 优化器实现指南

## 目录
1. [概述](#概述)
2. [基础概念](#基础概念)
3. [控制流图构建](#控制流图构建)
4. [数据流分析](#数据流分析)
5. [优化算法](#优化算法)
6. [实现细节](#实现细节)
7. [测试与验证](#测试与验证)

---

## 概述

本文档介绍了 CCPL 编译器中 TAC（三地址码）优化器的设计与实现。优化器采用多轮迭代的方式，结合局部优化和全局优化技术，对中间代码进行优化，以提高生成代码的执行效率。

### 主要优化技术

- **公共子表达式消除（CSE, Common Subexpression Elimination）**
- **循环不变量外提（LICM, Loop Invariant Code Motion）**
- **常量折叠（Constant Folding）**
- **常量传播（Constant Propagation）**
- **拷贝传播（Copy Propagation）**
- **死代码消除（DCE, Dead Code Elimination）**
- **控制流简化（Control Flow Simplification）**
- **不可达代码消除（Unreachable Code Elimination）**

---

## 基础概念

### 2.1 三地址码（TAC）

三地址码是一种中间表示形式，每条指令最多包含三个地址（操作数）。例如：

```
a = b + c      // 二元运算
x = -y         // 一元运算
if x < 10 goto L1  // 条件跳转
goto L2        // 无条件跳转
```

在我们的实现中，TAC 结构定义为：

```cpp
struct TAC {
    TAC_OP op;              // 操作码
    std::shared_ptr<SYM> a; // 目标操作数或跳转标签
    std::shared_ptr<SYM> b; // 源操作数1
    std::shared_ptr<SYM> c; // 源操作数2
    std::shared_ptr<TAC> prev, next; // 双向链表
};
```

### 2.2 基本块（Basic Block）

基本块是程序中的一段顺序执行的代码，具有以下特点：

- **入口唯一**：只有第一条指令可以被执行到
- **出口唯一**：只有最后一条指令可以将控制转移出去
- **顺序执行**：中间的指令按顺序执行，没有跳转

基本块的划分规则：

1. 程序的第一条指令是一个 leader
2. 跳转指令的目标（标签）是一个 leader
3. 跳转指令的下一条指令是一个 leader

### 2.3 控制流图（CFG, Control Flow Graph）

控制流图是程序的图形表示，其中：

- **节点**：表示基本块
- **边**：表示控制流的转移关系

```cpp
struct BasicBlock {
    int id;
    std::shared_ptr<TAC> start, end;  // 基本块的起始和结束指令
    std::vector<std::shared_ptr<BasicBlock>> predecessors;  // 前驱
    std::vector<std::shared_ptr<BasicBlock>> successors;    // 后继
};
```

---

## 控制流图构建

### 3.1 基本块划分

**算法步骤**：

1. **标记 leaders**：遍历 TAC 指令序列，标记所有 leader 指令
2. **划分基本块**：从每个 leader 开始，直到下一个 leader 之前，形成一个基本块

```cpp
bool BlockBuilder::is_leader(std::shared_ptr<TAC> tac, std::shared_ptr<TAC> prev) {
    if (!tac) return false;
    if (tac->op == TAC_OP::ENDFUNC) return false;
    
    // 程序开始
    if (tac == tac_first) return true;
    
    // 标签是入口
    if (tac->op == TAC_OP::LABEL) return true;
    
    // 跳转指令后的下一条指令
    if (prev && (prev->op == TAC_OP::IFZ || prev->op == TAC_OP::GOTO ||
                 prev->op == TAC_OP::RETURN || prev->op == TAC_OP::ENDFUNC)) {
        return true;
    }
    
    return false;
}
```

### 3.2 控制流图构建

**后继关系的确定**：

- **无条件跳转 `goto L`**：唯一后继是标签 L 对应的基本块
- **条件跳转 `ifz x goto L`**：两个后继
  - 跳转目标：标签 L 对应的基本块
  - fall-through：顺序执行的下一个基本块
- **其他指令**：后继是顺序执行的下一个基本块
- **return/end**：无后继（出口节点）

```cpp
void BlockBuilder::build_cfg() {
    for (size_t i = 0; i < basic_blocks.size(); ++i) {
        auto block = basic_blocks[i];
        auto end_instr = block->end;
        
        switch (end_instr->op) {
        case TAC_OP::GOTO:
            // 添加跳转目标为后继
            auto target = find_block_by_label(end_instr->a);
            if (target) {
                block->successors.push_back(target);
                target->predecessors.push_back(block);
            }
            break;
            
        case TAC_OP::IFZ:
            // 添加跳转目标
            auto target = find_block_by_label(end_instr->a);
            if (target) {
                block->successors.push_back(target);
                target->predecessors.push_back(block);
            }
            // 添加下一个基本块（fall-through）
            if (i + 1 < basic_blocks.size()) {
                auto next = basic_blocks[i + 1];
                block->successors.push_back(next);
                next->predecessors.push_back(block);
            }
            break;
            
        // ... 其他情况
        }
    }
}
```

---

## 数据流分析

数据流分析是优化的基础，用于收集程序中变量的定义和使用信息。

### 4.1 到达定值分析（Reaching Definitions）

**目的**：确定每个程序点上，哪些变量的定值可能到达该点。

**数学定义**：

- **GEN[B]**：基本块 B 中生成的定值
- **KILL[B]**：基本块 B 中杀死的定值（同一变量的其他定值）
- **IN[B]**：基本块 B 入口处的到达定值
- **OUT[B]**：基本块 B 出口处的到达定值

**数据流方程**：

```
IN[B] = ∪ OUT[P] for all predecessors P of B
OUT[B] = GEN[B] ∪ (IN[B] - KILL[B])
```

**实现**：

```cpp
void TACOptimizer::compute_reaching_definitions(
    const std::vector<std::shared_ptr<BasicBlock>>& blocks) {
    
    // 迭代求解不动点
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (auto& block : blocks) {
            // IN[B] = ∪ OUT[P]
            std::unordered_map<std::shared_ptr<SYM>, 
                              std::unordered_set<std::shared_ptr<TAC>>> new_in;
            for (auto& pred : block->predecessors) {
                for (auto& [var, defs] : block_out[pred].reaching_defs) {
                    new_in[var].insert(defs.begin(), defs.end());
                }
            }
            
            if (new_in != block_in[block].reaching_defs) {
                block_in[block].reaching_defs = new_in;
                changed = true;
            }
            
            // OUT[B] = GEN[B] ∪ (IN[B] - KILL[B])
            auto new_out = block_in[block].reaching_defs;
            auto tac = block->start;
            while (tac) {
                auto def = get_def(tac);
                if (def) {
                    // KILL：删除该变量的所有其他定值
                    new_out[def].clear();
                    // GEN：添加当前定值
                    new_out[def].insert(tac);
                }
                if (tac == block->end) break;
                tac = tac->next;
            }
            
            if (new_out != block_out[block].reaching_defs) {
                block_out[block].reaching_defs = new_out;
                changed = true;
            }
        }
    }
}
```

### 4.2 活跃变量分析（Live Variables）

**目的**：确定每个程序点上，哪些变量在后续会被使用。

**特点**：这是一个**逆向数据流分析**（从程序出口向入口分析）。

**数学定义**：

- **USE[B]**：基本块 B 中使用的变量（在定值之前）
- **DEF[B]**：基本块 B 中定值的变量
- **IN[B]**：基本块 B 入口处的活跃变量
- **OUT[B]**：基本块 B 出口处的活跃变量

**数据流方程**：

```
OUT[B] = ∪ IN[S] for all successors S of B
IN[B] = USE[B] ∪ (OUT[B] - DEF[B])
```

**实现**：

```cpp
void TACOptimizer::compute_live_variables(
    const std::vector<std::shared_ptr<BasicBlock>>& blocks) {
    
    bool changed = true;
    while (changed) {
        changed = false;
        
        // 逆序遍历基本块
        for (auto it = blocks.rbegin(); it != blocks.rend(); ++it) {
            auto& block = *it;
            
            // OUT[B] = ∪ IN[S]
            std::unordered_set<std::shared_ptr<SYM>> new_out;
            for (auto& succ : block->successors) {
                new_out.insert(block_in[succ].live_vars.begin(),
                              block_in[succ].live_vars.end());
            }
            
            if (new_out != block_out[block].live_vars) {
                block_out[block].live_vars = new_out;
                changed = true;
            }
            
            // IN[B] = USE[B] ∪ (OUT[B] - DEF[B])
            auto new_in = block_out[block].live_vars;
            
            // 逆序遍历块中的指令
            std::vector<std::shared_ptr<TAC>> instructions;
            auto tac = block->start;
            while (tac) {
                instructions.push_back(tac);
                if (tac == block->end) break;
                tac = tac->next;
            }
            
            for (auto it = instructions.rbegin(); it != instructions.rend(); ++it) {
                auto& instr = *it;
                
                // 移除定值变量
                auto def = get_def(instr);
                if (def) new_in.erase(def);
                
                // 添加使用的变量
                auto uses = get_uses(instr);
                for (auto& use : uses) {
                    new_in.insert(use);
                }
            }
            
            if (new_in != block_in[block].live_vars) {
                block_in[block].live_vars = new_in;
                changed = true;
            }
        }
    }
}
```

### 4.3 常量传播分析（Constant Propagation）

**目的**：确定每个程序点上，哪些变量具有确定的常量值。

**格值（Lattice）**：

```
        TOP (未知)
       /    |    \
      1     2     3  ... (常量值)
       \    |    /
       BOTTOM (非常量)
```

**Meet 操作**：

- `TOP ⊓ c = c`（第一次赋值）
- `c ⊓ c = c`（相同常量）
- `c1 ⊓ c2 = BOTTOM`（不同常量，c1 ≠ c2）
- `BOTTOM ⊓ x = BOTTOM`（已知非常量）

**实现**：

```cpp
void TACOptimizer::compute_constant_propagation(
    const std::vector<std::shared_ptr<BasicBlock>>& blocks) {
    
    const int BOTTOM = INT_MIN;
    
    std::queue<std::shared_ptr<BasicBlock>> worklist;
    std::unordered_set<std::shared_ptr<BasicBlock>> in_worklist;
    
    // 初始化工作列表
    for (auto& block : blocks) {
        worklist.push(block);
        in_worklist.insert(block);
    }
    
    while (!worklist.empty()) {
        auto block = worklist.front();
        worklist.pop();
        in_worklist.erase(block);
        
        // IN[B] = meet of OUT[P]
        std::unordered_map<std::shared_ptr<SYM>, int> new_in;
        bool first = true;
        
        for (auto& pred : block->predecessors) {
            if (first) {
                new_in = block_out[pred].constants;
                first = false;
            } else {
                // Meet 操作
                std::unordered_set<std::shared_ptr<SYM>> all_vars;
                for (auto& [var, _] : new_in) all_vars.insert(var);
                for (auto& [var, _] : block_out[pred].constants) all_vars.insert(var);
                
                for (auto& var : all_vars) {
                    bool in_new = new_in.find(var) != new_in.end();
                    bool in_pred = block_out[pred].constants.find(var) != block_out[pred].constants.end();
                    
                    if (in_new && in_pred) {
                        if (new_in[var] != block_out[pred].constants[var]) {
                            new_in[var] = BOTTOM;  // 不同常量
                        }
                    } else if (in_pred) {
                        new_in[var] = block_out[pred].constants[var];
                    }
                }
            }
        }
        
        block_in[block].constants = new_in;
        
        // OUT[B] = transfer(IN[B])
        auto new_out = new_in;
        auto tac = block->start;
        
        while (tac) {
            auto def = get_def(tac);
            if (def) {
                // 尝试计算常量值
                if (tac->op == TAC_OP::COPY) {
                    int val;
                    if (get_const_value(tac->b, val)) {
                        new_out[def] = val;
                    } else {
                        new_out[def] = BOTTOM;
                    }
                } else if (/* 二元运算 */) {
                    // 如果操作数都是常量，计算结果
                    // ...
                } else {
                    new_out[def] = BOTTOM;
                }
            }
            
            if (tac == block->end) break;
            tac = tac->next;
        }
        
        // 如果 OUT 改变，将后继加入工作列表
        if (new_out != block_out[block].constants) {
            block_out[block].constants = new_out;
            for (auto& succ : block->successors) {
                if (in_worklist.find(succ) == in_worklist.end()) {
                    worklist.push(succ);
                    in_worklist.insert(succ);
                }
            }
        }
    }
}
```

---

## 优化算法

### 5.1 常量折叠（Constant Folding）

**目的**：在编译时计算常量表达式的值。

**示例**：

```
x = 3 + 5   →   x = 8
y = 10 * 2  →   y = 20
```

**实现**：

```cpp
bool TACOptimizer::local_constant_folding(std::shared_ptr<TAC> tac) {
    bool changed = false;
    auto current = tac;
    
    while (current) {
        if (current->op == TAC_OP::ADD || current->op == TAC_OP::SUB ||
            current->op == TAC_OP::MUL || current->op == TAC_OP::DIV) {
            
            int val_b, val_c;
            if (get_const_value(current->b, val_b) &&
                get_const_value(current->c, val_c)) {
                
                int result;
                switch (current->op) {
                case TAC_OP::ADD: result = val_b + val_c; break;
                case TAC_OP::SUB: result = val_b - val_c; break;
                case TAC_OP::MUL: result = val_b * val_c; break;
                case TAC_OP::DIV: 
                    if (val_c != 0) result = val_b / val_c;
                    else continue;  // 除零错误
                    break;
                }
                
                // 转换为 COPY 指令
                current->op = TAC_OP::COPY;
                current->b = make_const(result);
                current->c = nullptr;
                changed = true;
            }
        }
        current = current->next;
    }
    
    return changed;
}
```

### 5.2 常量传播（Constant Propagation）

**目的**：将变量的常量值传播到使用点。

**局部常量传播**：仅在基本块内传播。

**全局常量传播**：基于数据流分析，跨基本块传播。

**示例**：

```
a = 5
b = a + 3    →    b = 5 + 3    →    b = 8
```

**实现**：

```cpp
bool TACOptimizer::global_constant_propagation(
    const std::vector<std::shared_ptr<BasicBlock>>& blocks) {
    
    bool changed = false;
    const int BOTTOM = INT_MIN;
    
    for (auto& block : blocks) {
        auto& in_constants = block_in[block].constants;
        auto current_constants = in_constants;
        
        auto tac = block->start;
        while (tac) {
            // 替换使用的变量为常量
            if (tac->b && tac->b->type == SYM_TYPE::VAR) {
                auto it = current_constants.find(tac->b);
                if (it != current_constants.end() && it->second != BOTTOM) {
                    tac->b = make_const(it->second);
                    changed = true;
                }
            }
            
            if (tac->c && tac->c->type == SYM_TYPE::VAR) {
                auto it = current_constants.find(tac->c);
                if (it != current_constants.end() && it->second != BOTTOM) {
                    tac->c = make_const(it->second);
                    changed = true;
                }
            }
            
            // 更新常量状态
            auto def = get_def(tac);
            if (def) {
                // ... 更新 current_constants
            }
            
            if (tac == block->end) break;
            tac = tac->next;
        }
    }
    
    return changed;
}
```

### 5.3 拷贝传播（Copy Propagation）

**目的**：消除不必要的拷贝操作。

**示例**：

```
x = y
z = x + 1    →    z = y + 1
```

**实现**：

```cpp
bool TACOptimizer::local_copy_propagation(std::shared_ptr<TAC> tac) {
    bool changed = false;
    std::unordered_map<std::shared_ptr<SYM>, std::shared_ptr<SYM>> copy_map;
    
    auto current = tac;
    while (current) {
        // 记录拷贝关系：a = b
        if (current->op == TAC_OP::COPY && current->a && current->b &&
            current->b->type == SYM_TYPE::VAR) {
            copy_map[current->a] = current->b;
        } else if (current->a) {
            copy_map.erase(current->a);  // 变量被重新赋值
        }
        
        // 替换使用点
        if (current->b && current->b->type == SYM_TYPE::VAR) {
            auto it = copy_map.find(current->b);
            if (it != copy_map.end()) {
                current->b = it->second;
                changed = true;
            }
        }
        
        current = current->next;
    }
    
    return changed;
}
```

### 5.4 死代码消除（Dead Code Elimination）

**目的**：删除对程序输出无影响的代码。

**判断标准**：

- 变量被定值，但后续没有被使用
- 指令没有副作用（不是 I/O、函数调用等）

**基于活跃变量分析**：

```cpp
bool TACOptimizer::global_dead_code_elimination(
    const std::vector<std::shared_ptr<BasicBlock>>& blocks) {
    
    bool changed = false;
    
    for (auto& block : blocks) {
        auto& out_live = block_out[block].live_vars;
        auto current_live = out_live;
        
        // 收集块内指令
        std::vector<std::shared_ptr<TAC>> instructions;
        auto tac = block->start;
        while (tac) {
            instructions.push_back(tac);
            if (tac == block->end) break;
            tac = tac->next;
        }
        
        // 逆序遍历，删除死代码
        for (auto it = instructions.rbegin(); it != instructions.rend(); ++it) {
            auto& instr = *it;
            auto def = get_def(instr);
            
            // 如果定值变量不活跃，且无副作用，删除
            if (def && current_live.find(def) == current_live.end()) {
                if (instr->op != TAC_OP::CALL && instr->op != TAC_OP::INPUT) {
                    // 删除指令
                    if (instr->prev) instr->prev->next = instr->next;
                    if (instr->next) instr->next->prev = instr->prev;
                    changed = true;
                    continue;
                }
            }
            
            // 更新活跃变量集合
            if (def) current_live.erase(def);
            auto uses = get_uses(instr);
            for (auto& use : uses) current_live.insert(use);
        }
    }
    
    return changed;
}
```

### 5.5 公共子表达式消除（CSE）

**目的**：避免重复计算相同的表达式。

**示例**：

```
t1 = b * c
x = a + t1
t2 = b * c      // 重复计算
y = t2 - d

→

t1 = b * c
x = a + t1
t2 = t1         // 使用已计算的值
y = t2 - d
```

**实现**：

```cpp
bool TACOptimizer::common_subexpression_elimination(std::shared_ptr<BasicBlock> block) {
    bool changed = false;
    
    // 表达式键 -> 结果变量
    std::unordered_map<std::string, std::shared_ptr<SYM>> expr_map;
    
    auto tac = block->start;
    while (tac) {
        std::string key = get_expression_key(tac);
        
        if (!key.empty() && tac->a && tac->a->type == SYM_TYPE::VAR) {
            auto it = expr_map.find(key);
            if (it != expr_map.end()) {
                // 找到相同表达式，替换为拷贝
                tac->op = TAC_OP::COPY;
                tac->b = it->second;
                tac->c = nullptr;
                changed = true;
            } else {
                expr_map[key] = tac->a;
            }
        }
        
        // 变量被重新定值时，使相关表达式失效
        auto def = get_def(tac);
        if (def) {
            std::vector<std::string> to_remove;
            for (auto& [expr_key, var] : expr_map) {
                if (expr_key.find(def->to_string()) != std::string::npos) {
                    to_remove.push_back(expr_key);
                }
            }
            for (auto& key : to_remove) expr_map.erase(key);
        }
        
        if (tac == block->end) break;
        tac = tac->next;
    }
    
    return changed;
}
```

**表达式键的生成**：

```cpp
std::string TACOptimizer::get_expression_key(std::shared_ptr<TAC> tac) const {
    std::string key;
    
    switch (tac->op) {
    case TAC_OP::ADD:
    case TAC_OP::MUL:  // 满足交换律
        {
            std::string b_str = tac->b ? tac->b->to_string() : "";
            std::string c_str = tac->c ? tac->c->to_string() : "";
            if (b_str > c_str) std::swap(b_str, c_str);
            key = "ADD_MUL:" + b_str + "," + c_str;
        }
        break;
    case TAC_OP::SUB:
        key = "SUB:" + (tac->b ? tac->b->to_string() : "") + "," +
              (tac->c ? tac->c->to_string() : "");
        break;
    // ... 其他运算
    }
    
    return key;
}
```

### 5.6 循环不变量外提（LICM）

**目的**：将循环中不变的计算移到循环外，减少重复计算。

**循环不变量的定义**：

一个表达式是循环不变的，当且仅当：

1. 所有操作数都是常量
2. 或所有操作数在循环中未被修改

**示例**：

```
while (i < 10) {
    x = a + b;    // a 和 b 在循环中不变
    y = x * i;
    i = i + 1;
}

→

x = a + b;        // 外提到循环前
while (i < 10) {
    y = x * i;
    i = i + 1;
}
```

**实现步骤**：

1. **识别循环**：通过回边识别循环头
2. **查找循环体**：从回边源节点反向遍历
3. **识别不变量**：检查操作数是否在循环内被修改
4. **外提指令**：移动到循环头的前驱块

```cpp
bool TACOptimizer::loop_invariant_code_motion(std::shared_ptr<BasicBlock> loop_header) {
    bool changed = false;
    
    // 找到循环体的所有块
    std::unordered_set<std::shared_ptr<BasicBlock>> loop_blocks;
    find_loop_blocks(loop_header, loop_blocks);
    
    // 收集可外提的指令及其变量声明
    std::vector<std::pair<std::shared_ptr<TAC>, std::shared_ptr<TAC>>> invariant_pairs;
    
    for (auto& block : loop_blocks) {
        if (block == loop_header) continue;
        
        auto tac = block->start;
        while (tac) {
            if (is_loop_invariant(tac, loop_header, loop_blocks)) {
                auto def = get_def(tac);
                if (def) {
                    // 查找 var 声明
                    std::shared_ptr<TAC> var_decl = nullptr;
                    auto search = tac->prev;
                    while (search) {
                        if (search->op == TAC_OP::VAR &&
                            search->a && search->a->name == def->name) {
                            var_decl = search;
                            break;
                        }
                        search = search->prev;
                    }
                    
                    invariant_pairs.push_back({tac, var_decl});
                }
            }
            
            if (tac == block->end) break;
            tac = tac->next;
        }
    }
    
    // 外提到循环头的前驱
    if (!invariant_pairs.empty()) {
        std::shared_ptr<BasicBlock> preheader = nullptr;
        for (auto& pred : loop_header->predecessors) {
            if (loop_blocks.find(pred) == loop_blocks.end()) {
                preheader = pred;
                break;
            }
        }
        
        if (preheader) {
            for (auto& [inv_tac, var_decl] : invariant_pairs) {
                // 移除并插入到 preheader 末尾
                // ... (详见完整实现)
                changed = true;
            }
        }
    }
    
    return changed;
}
```

**判断循环不变量**：

```cpp
bool TACOptimizer::is_loop_invariant(
    std::shared_ptr<TAC> tac,
    std::shared_ptr<BasicBlock> loop_header,
    const std::unordered_set<std::shared_ptr<BasicBlock>>& loop_blocks) const {
    
    // 只考虑算术运算
    if (tac->op != TAC_OP::ADD && tac->op != TAC_OP::SUB &&
        tac->op != TAC_OP::MUL && tac->op != TAC_OP::DIV) {
        return false;
    }
    
    auto check_operand = [&](std::shared_ptr<SYM> sym) -> bool {
        if (!sym) return true;
        
        // 常量是不变的
        if (sym->type == SYM_TYPE::CONST_INT || sym->type == SYM_TYPE::CONST_CHAR)
            return true;
        
        // 检查变量是否在循环内被定值
        if (sym->type == SYM_TYPE::VAR) {
            for (auto& block : loop_blocks) {
                auto instr = block->start;
                while (instr) {
                    auto def = get_def(instr);
                    if (def && def->name == sym->name) {
                        return false;  // 在循环内被修改
                    }
                    if (instr == block->end) break;
                    instr = instr->next;
                }
            }
            return true;  // 不在循环内被修改
        }
        
        return false;
    };
    
    return check_operand(tac->b) && check_operand(tac->c);
}
```

### 5.7 控制流简化

**目的**：简化确定性的控制流。

**优化场景**：

1. **常量条件跳转**：
   - `ifz 0 goto L` → `goto L`（总是跳转）
   - `ifz 1 goto L` → 删除（永不跳转）

2. **冗余跳转**：
   - `goto L; label L;` → 删除 goto

**实现**：

```cpp
bool TACOptimizer::simplify_control_flow(std::shared_ptr<TAC> tac_start) {
    bool changed = false;
    auto current = tac_start;
    
    while (current) {
        if (current->op == TAC_OP::IFZ && current->b) {
            int cond_value;
            if (get_const_value(current->b, cond_value)) {
                if (cond_value == 0) {
                    // ifz 0 → 总是跳转
                    current->op = TAC_OP::GOTO;
                    current->b = nullptr;
                    changed = true;
                } else {
                    // ifz <非0> → 永不跳转，删除
                    auto next = current->next;
                    if (current->prev) current->prev->next = current->next;
                    if (current->next) current->next->prev = current->prev;
                    current = next;
                    changed = true;
                    continue;
                }
            }
        }
        
        current = current->next;
    }
    
    return changed;
}
```

### 5.8 不可达代码消除

**目的**：删除永远不会执行的代码块。

**方法**：从程序入口开始，进行可达性分析。

**实现**：

```cpp
bool TACOptimizer::eliminate_unreachable_code(
    std::vector<std::shared_ptr<BasicBlock>>& blocks) {
    
    bool changed = false;
    
    // 可达性分析
    std::unordered_set<std::shared_ptr<BasicBlock>> reachable;
    std::queue<std::shared_ptr<BasicBlock>> worklist;
    
    worklist.push(blocks[0]);  // 从第一个块开始
    reachable.insert(blocks[0]);
    
    while (!worklist.empty()) {
        auto block = worklist.front();
        worklist.pop();
        
        for (auto& succ : block->successors) {
            if (reachable.find(succ) == reachable.end()) {
                reachable.insert(succ);
                worklist.push(succ);
            }
        }
    }
    
    // 删除不可达的块
    std::vector<std::shared_ptr<BasicBlock>> new_blocks;
    for (auto& block : blocks) {
        if (reachable.find(block) != reachable.end()) {
            new_blocks.push_back(block);
        } else {
            // 从 TAC 链表中删除该块的指令
            auto tac = block->start;
            while (tac) {
                auto next = (tac == block->end) ? nullptr : tac->next;
                if (tac->prev) tac->prev->next = tac->next;
                if (tac->next) tac->next->prev = tac->prev;
                tac = next;
                if (!next) break;
            }
            changed = true;
        }
    }
    
    if (changed) blocks = new_blocks;
    return changed;
}
```

---

## 实现细节

### 6.1 优化器架构

```
TACOptimizer
├── BlockBuilder (控制流图构建)
├── DataFlowInfo (数据流分析结果)
│   ├── reaching_defs (到达定值)
│   ├── live_vars (活跃变量)
│   └── constants (常量信息)
└── 优化算法
    ├── 局部优化
    │   ├── local_constant_folding
    │   └── local_copy_propagation
    └── 全局优化
        ├── global_constant_propagation
        ├── global_dead_code_elimination
        ├── common_subexpression_elimination
        ├── loop_invariant_code_motion
        ├── simplify_control_flow
        └── eliminate_unreachable_code
```

### 6.2 优化流程

```cpp
void TACOptimizer::optimize() {
    // 1. 构建控制流图
    block_builder.build();
    auto blocks = block_builder.get_basic_blocks();
    
    // 2. 多轮迭代优化
    bool global_changed = true;
    int global_iter = 0;
    const int MAX_GLOBAL_ITER = 5;
    
    while (global_changed && global_iter < MAX_GLOBAL_ITER) {
        global_changed = false;
        global_iter++;
        
        // 2.1 公共子表达式消除
        for (auto& block : blocks) {
            if (common_subexpression_elimination(block)) {
                global_changed = true;
            }
        }
        
        // 2.2 循环不变量外提
        for (auto& block : blocks) {
            if (is_loop_header(block)) {
                if (loop_invariant_code_motion(block)) {
                    global_changed = true;
                }
            }
        }
        
        // 2.3 局部优化
        for (auto& block : blocks) {
            optimize_block_local(block);
        }
        
        // 2.4 数据流分析
        compute_reaching_definitions(blocks);
        compute_live_variables(blocks);
        compute_constant_propagation(blocks);
        
        // 2.5 全局常量传播
        if (global_constant_propagation(blocks)) {
            global_changed = true;
        }
        
        // 2.6 常量折叠
        for (auto& block : blocks) {
            if (local_constant_folding(block->start)) {
                global_changed = true;
            }
        }
        
        // 2.7 死代码消除
        if (global_dead_code_elimination(blocks)) {
            global_changed = true;
        }
        
        // 2.8 控制流简化
        if (simplify_control_flow(tac_first)) {
            global_changed = true;
            block_builder.build();
            blocks = block_builder.get_basic_blocks();
        }
        
        // 2.9 不可达代码消除
        if (eliminate_unreachable_code(blocks)) {
            global_changed = true;
        }
    }
}
```

### 6.3 辅助函数

**获取定值变量**：

```cpp
std::shared_ptr<SYM> TACOptimizer::get_def(std::shared_ptr<TAC> tac) const {
    if (!tac || !tac->a) return nullptr;
    
    if (tac->op == TAC_OP::COPY || tac->op == TAC_OP::ADD ||
        tac->op == TAC_OP::SUB || tac->op == TAC_OP::MUL ||
        tac->op == TAC_OP::DIV || /* ... */) {
        if (tac->a->type == SYM_TYPE::VAR)
            return tac->a;
    }
    
    return nullptr;
}
```

**获取使用的变量**：

```cpp
std::vector<std::shared_ptr<SYM>> TACOptimizer::get_uses(std::shared_ptr<TAC> tac) const {
    std::vector<std::shared_ptr<SYM>> uses;
    if (!tac) return uses;
    
    if (tac->b && tac->b->type == SYM_TYPE::VAR) {
        uses.push_back(tac->b);
    }
    
    if (tac->c && tac->c->type == SYM_TYPE::VAR) {
        uses.push_back(tac->c);
    }
    
    // 特殊指令
    if (tac->op == TAC_OP::RETURN || tac->op == TAC_OP::OUTPUT ||
        tac->op == TAC_OP::IFZ || tac->op == TAC_OP::ACTUAL) {
        if (tac->a && tac->a->type == SYM_TYPE::VAR) {
            uses.push_back(tac->a);
        }
    }
    
    return uses;
}
```

---

## 测试与验证

### 7.1 测试用例

**测试 1：简单常量传播和折叠**

```c
main(){
    int a, b, c;
    a = 5;
    b = 10;
    c = a + b;
    output c;
    return 0;
}
```

**优化结果**：

```
label main
begin
output 15    // a + b 被折叠为 15
return 0
end
```

**测试 2：带控制流的常量传播**

```c
main(){
    int x, y, z;
    x = 3;
    y = 4;
    z = x * y + 2;
    
    if(z > 10){
        output z;
    } else {
        output "ops!";
    }
    
    return 0;
}
```

**优化结果**：

```
label main
begin
output 14      // z 被优化为 14
label L3       // else 分支被删除（不可达）
return 0
end
```

**测试 3：循环（不应错误优化）**

```c
main(){
    int i, sum;
    i = 0;
    sum = 0;
    
    while(i < 5){
        sum = sum + i;
        i = i + 1;
    }
    
    output sum;
    return 0;
}
```

**优化结果**：循环结构保持完整，`i` 和 `sum` 不被错误地传播为常量。

**测试 4：嵌套循环与循环不变量外提**

```c
main(){
    int a, b, c, d, e, i, j;
    input a;
    input b;
    input c;
    j = 5;
    
    while(j > 0){
        output j;
        i = 9;
        while(i > 0){
            output i;
            d = a + b * c - (a + c) / b + 9;
            e = a + b * c - (c - a) / b + 9;
            i = i - 1;
        }
        j = j - 1;
    }
    
    output d;
    output e;
}
```

**优化效果**：

- **CSE**：`b * c` 只计算一次
- **LICM**：`d` 和 `e` 的计算移到循环外
- **性能提升**：内循环从 ~14 个运算减少到 2 个赋值

**优化前**（内循环每次迭代）：

```
@t2 = b * c
@t3 = a + @t2
@t4 = a + c
@t5 = @t4 / b
@t6 = @t3 - @t5
@t7 = @t6 + 9
d = @t7
@t8 = b * c        // 重复
@t9 = a + @t8      // 重复
@t10 = c - a
@t11 = @t10 / b
@t12 = @t9 - @t11
@t13 = @t12 + 9
e = @t13
```

**优化后**（外提到循环前）：

```
// 循环前
var @t2 : int
@t2 = b * c
var @t4 : int
@t4 = a + c
var @t10 : int
@t10 = c - a
var @t9 : int
@t9 = a + @t2
var @t5 : int
@t5 = @t4 / b
var @t11 : int
@t11 = @t10 / b
var @t6 : int
@t6 = @t9 - @t5
var @t12 : int
@t12 = @t9 - @t11
var @t7 : int
@t7 = @t6 + 9
var @t13 : int
@t13 = @t12 + 9

// 内循环
while(i > 0){
    output i;
    d = @t7        // 只剩赋值
    e = @t13       // 只剩赋值
    i = i - 1;
}
```

**性能影响**：

- 外循环：5 次迭代
- 内循环：每次 9 次迭代
- 总计：45 次内循环执行
- **节省运算**：~12 操作/次 × 45 = **540 次运算**

### 7.2 验证方法

1. **正确性验证**：
   - 比较优化前后的程序输出
   - 确保语义等价

2. **性能验证**：
   - 统计 TAC 指令数量
   - 计算减少的运算次数

3. **边界情况测试**：
   - 空程序
   - 单基本块程序
   - 深度嵌套循环
   - 复杂控制流

---

## 总结

### 优化器的关键要点

1. **迭代优化**：多轮优化可以发现更多机会
2. **优化顺序**：先进行简化优化（CSE、LICM），再进行传播和消除
3. **数据流分析**：是全局优化的基础
4. **正确性保证**：
   - 循环中的变量不能错误传播
   - 有副作用的指令不能删除
   - 保持程序语义

### 性能提升

- **常量折叠和传播**：减少运行时计算
- **CSE**：避免重复计算
- **LICM**：显著减少循环内的运算
- **死代码消除**：减小代码体积
- **控制流简化**：消除不必要的跳转

### 扩展方向

1. **更多优化**：
   - 强度削弱（用移位代替乘法）
   - 归纳变量优化
   - 函数内联

2. **更精确的分析**：
   - 别名分析（指针）
   - 过程间分析

3. **寄存器分配**：
   - 图着色算法
   - 线性扫描算法

---

## 参考资料

1. **Compilers: Principles, Techniques, and Tools** (龙书)
   - Alfred V. Aho, et al.
   - 第 9 章：代码优化

2. **Advanced Compiler Design and Implementation**
   - Steven S. Muchnick
   - 数据流分析和优化技术

3. **Engineering a Compiler**
   - Keith D. Cooper, Linda Torczon
   - 第 8-10 章：优化技术

4. **Static Single Assignment Book**
   - SSA 形式和现代优化技术

---

## 附录：完整优化流程图

```
┌─────────────────────────────────────────────────────────────┐
│                     TAC 优化器流程                           │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
        ┌───────────────────────────────────────┐
        │     1. 构建控制流图（CFG）              │
        │        - 划分基本块                    │
        │        - 建立前驱后继关系              │
        └───────────────────────────────────────┘
                            │
                            ▼
        ┌───────────────────────────────────────┐
        │     2. 开始迭代优化（最多5轮）          │
        └───────────────────────────────────────┘
                            │
        ┌───────────────────┴───────────────────┐
        │                                       │
        ▼                                       ▼
┌─────────────────────┐             ┌─────────────────────┐
│   局部优化          │             │   全局优化          │
├─────────────────────┤             ├─────────────────────┤
│ • CSE               │             │ • 数据流分析        │
│ • 常量折叠          │             │   - 到达定值        │
│ • 拷贝传播          │             │   - 活跃变量        │
│ • LICM              │             │   - 常量传播        │
└─────────────────────┘             │ • 全局常量传播      │
                                    │ • 死代码消除        │
                                    │ • 控制流简化        │
                                    │ • 不可达代码消除    │
                                    └─────────────────────┘
        │                                       │
        └───────────────────┬───────────────────┘
                            │
                            ▼
                    ┌───────────────┐
                    │  是否改变？    │
                    └───────────────┘
                        │       │
                    是  │       │ 否
                        │       │
                        ▼       ▼
                    继续迭代   结束优化
```

---

**文档版本**：1.0  
**最后更新**：2025年11月9日  
**作者**：CCPL 编译器项目组
