# TAC (Three Address Code) 中间代码生成器

## 概述

本项目实现了一个基于 C++ `std::variant` 的现代化 TAC（三地址码）中间代码生成系统，用于编译器的中端处理。

## 架构设计

### 核心数据结构

#### 1. SYM（符号表项）

```cpp
struct SYM {
    SYM_TYPE type;      // 符号类型：VAR, FUNC, TEXT, LABEL, INT, CHAR
    SYM_SCOPE scope;    // 作用域：GLOBAL, LOCAL
    std::string name;   // 符号名称
    std::variant<int, char, std::string> value;  // 使用 variant 存储不同类型的值
    int offset;         // 偏移量（用于代码生成）
    int label;          // 标签编号（用于 TEXT 类型）
};
```

**使用 std::variant 的优势**：
- 类型安全：编译期检查类型
- 内存高效：只占用最大类型的空间
- 现代 C++ 特性：避免 union 的不安全性

#### 2. TAC（三地址码指令）

```cpp
struct TAC {
    TAC_OP op;                      // 操作码
    std::shared_ptr<SYM> a, b, c;   // 操作数（使用智能指针管理内存）
    std::shared_ptr<TAC> prev, next; // 双向链表结构
};
```

**支持的操作码**：
- **算术运算**：ADD, SUB, MUL, DIV, NEG
- **比较运算**：EQ, NE, LT, LE, GT, GE
- **赋值**：COPY
- **控制流**：GOTO, IFZ, LABEL
- **函数**：BEGINFUNC, ENDFUNC, FORMAL, ACTUAL, CALL, RETURN
- **I/O**：INPUT, OUTPUT
- **变量声明**：VAR

#### 3. EXP（表达式）

```cpp
struct EXP {
    std::shared_ptr<TAC> code;   // 表达式生成的 TAC 代码
    std::shared_ptr<SYM> place;  // 表达式结果存放的位置
    std::shared_ptr<EXP> next;   // 用于参数列表链接
};
```

### TACGenerator 类

这是核心的 TAC 生成器类，采用单例模式：

```cpp
class TACGenerator {
private:
    int scope;  // 当前作用域（0=全局, 1=局部）
    int next_tmp;  // 临时变量计数器
    int next_label;  // 标签计数器
    
    // 使用 unordered_map 实现符号表（O(1) 查找）
    std::unordered_map<std::string, std::shared_ptr<SYM>> sym_tab_global;
    std::unordered_map<std::string, std::shared_ptr<SYM>> sym_tab_local;
    
    std::shared_ptr<TAC> tac_first, tac_last;  // TAC 指令链表

public:
    // 符号表操作
    std::shared_ptr<SYM> mk_var(const std::string& name);
    std::shared_ptr<SYM> mk_tmp();
    std::shared_ptr<SYM> mk_const(int value);
    std::shared_ptr<SYM> mk_label(const std::string& name);
    
    // 表达式生成
    std::shared_ptr<EXP> do_bin(TAC_OP op, EXP* exp1, EXP* exp2);
    std::shared_ptr<EXP> do_un(TAC_OP op, EXP* exp);
    
    // 语句生成
    std::shared_ptr<TAC> do_assign(SYM* var, EXP* exp);
    std::shared_ptr<TAC> do_if(EXP* exp, TAC* stmt);
    std::shared_ptr<TAC> do_while(EXP* exp, TAC* stmt);
    std::shared_ptr<TAC> do_for(TAC* init, EXP* cond, TAC* update, TAC* body);
    
    // ... 更多方法
};
```

## 主要特性

### 1. 智能内存管理

使用 `std::shared_ptr` 自动管理内存，避免内存泄漏：

```cpp
auto sym = std::make_shared<SYM>();
auto tac = std::make_shared<TAC>();
// 无需手动 delete，智能指针自动管理生命周期
```

### 2. 类型安全的值存储

使用 `std::variant` 存储不同类型的值：

```cpp
sym->value = 42;              // 存储 int
sym->value = 'x';             // 存储 char
sym->value = std::string("hello");  // 存储 string

// 安全访问
if (std::holds_alternative<int>(sym->value)) {
    int val = std::get<int>(sym->value);
}
```

### 3. 高效的符号表

使用 `std::unordered_map` 实现 O(1) 时间复杂度的符号查找：

```cpp
std::unordered_map<std::string, std::shared_ptr<SYM>> sym_tab_global;
auto it = sym_tab_global.find(name);
if (it != sym_tab_global.end()) {
    return it->second;
}
```

## 与 Bison 集成

### parser.y 配置

```bison
%code requires {
    #include <string>
    #include <memory>
    #include "tac.hh"
}

%type <std::shared_ptr<TAC>> statement
%type <std::shared_ptr<EXP>> expression
%type <std::shared_ptr<SYM>> func_header
```

### 语义动作示例

#### 变量声明
```bison
decl_list: IDENTIFIER
{
    $$ = tac_gen.declare_var($1);
}
```

#### 二元表达式
```bison
expression: expression '+' expression
{
    $$ = tac_gen.do_bin(TAC_OP::ADD, $1, $3);
}
```

#### If 语句
```bison
if_stmt: IF '(' expression ')' block
{
    $$ = tac_gen.do_if($3, $5);
}
| IF '(' expression ')' block ELSE block
{
    $$ = tac_gen.do_if_else($3, $5, $7);
}
```

#### 函数定义
```bison
function: func_header '(' param_list ')' block
{
    $$ = tac_gen.do_func($1, $3, $5);
    tac_gen.leave_scope();
}
```

## TAC 生成算法

### 1. 表达式翻译

对于二元运算 `a + b`：

```
1. 生成左操作数代码：code1
2. 生成右操作数代码：code2
3. 创建临时变量：t = mk_tmp()
4. 生成指令：var t
5. 生成指令：t = a + b
6. 返回表达式：{ place: t, code: join(code1, code2, var t, t = a + b) }
```

实现：
```cpp
std::shared_ptr<EXP> TACGenerator::do_bin(TAC_OP op, 
                                           std::shared_ptr<EXP> exp1, 
                                           std::shared_ptr<EXP> exp2) {
    auto temp = mk_tmp();
    auto temp_decl = mk_tac(TAC_OP::VAR, temp);
    temp_decl->prev = join_tac(exp1->code, exp2->code);
    
    auto ret = mk_tac(op, temp, exp1->place, exp2->place);
    ret->prev = temp_decl;
    
    return mk_exp(temp, ret);
}
```

### 2. If-Else 语句翻译

对于 `if (cond) stmt1 else stmt2`：

```
1. 生成条件表达式代码
2. 生成：ifz cond_result goto L1
3. 生成：stmt1 的代码
4. 生成：goto L2
5. 生成：label L1
6. 生成：stmt2 的代码
7. 生成：label L2
```

### 3. While 循环翻译

对于 `while (cond) stmt`：

```
1. 生成：label L1
2. 生成：条件表达式代码
3. 生成：ifz cond_result goto L2
4. 生成：stmt 的代码
5. 生成：goto L1
6. 生成：label L2
```

### 4. For 循环翻译

对于 `for (init; cond; update) body`：

```
1. 生成：init 的代码
2. 生成：label L1
3. 生成：cond 的代码
4. 生成：ifz cond_result goto L2
5. 生成：body 的代码
6. 生成：update 的代码
7. 生成：goto L1
8. 生成：label L2
```

## 示例

### 输入源代码

```c
int main() {
    int a, b, c;
    input a;
    b = a + 1;
    c = b + 2;
    output c;
    return 0;
}
```

### 生成的 TAC

```
   1: label main
   2: begin
   3: var a
   4: var b
   5: var c
   6: input a
   7: var t0
   8: t0 = a + 1
   9: b = t0
  10: var t1
  11: t1 = b + 2
  12: c = t1
  13: output c
  14: return 0
  15: end
```

## 编译和运行

### 编译项目

```bash
cd /home/twlm/Source/compileTest/final/ccpl
make
```

### 运行测试

```bash
./build/ccpl test/assign.m
```

## 优势总结

### 相比 C 版本（reference 中的实现）

1. **类型安全**：使用 `std::variant` 代替 C 的 union，编译期类型检查
2. **内存安全**：使用 `std::shared_ptr` 自动管理内存，避免内存泄漏
3. **现代 C++**：使用 STL 容器（`unordered_map`）提高效率
4. **可维护性**：面向对象设计，清晰的类接口
5. **可扩展性**：易于添加新的操作和特性

### 设计模式

1. **单例模式**：全局 `tac_gen` 实例
2. **工厂模式**：`mk_*` 系列方法创建对象
3. **RAII**：智能指针自动管理资源

## 未来扩展

1. **优化**：
   - 常量折叠
   - 死代码消除
   - 公共子表达式消除

2. **分析**：
   - 活跃变量分析
   - 到达定义分析
   - 数据流分析

3. **类型系统**：
   - 类型检查
   - 类型推导
   - 泛型支持

## 参考

- 龙书（Compilers: Principles, Techniques, and Tools）
- Modern Compiler Implementation in C/Java/ML
- C++17 标准库参考
