# ccpl 实验报告

## 增进

ccpl 由示例代码 mini-object 迁移而来，并使用 C++完全重写并扩展。在语法器和 TAC 生成器中间新增了 AST 层，并实现了复杂的类型系统以支持数组、结构体和指针等特性，并使编译器代码结构更加完整，功能职责划分清晰，易于维护和扩展。得益于 C++的面对对象特性和智能指针，编译器代码更加安全和高效。

### 编译器架构设计

CCPL 编译器采用经典的多阶段编译架构：

```
源文件(.m) → 词法分析 → 语法分析 → AST构建 → TAC生成 → 优化 → 汇编生成 → 机器码
```

## 1. 词法分析模块

词法分析器使用 Flex 实现，使用正则表达式匹配语言的基本词法单元（Token）。在 `ccpl/parser.y` 中定义了所有的 token:
```bison
%token EOL
%token INT CHAR VOID STRUCT
%token EQ NE LT LE GT GE UMINUS DEREF
%token IF ELSE FOR WHILE INPUT OUTPUT RETURN
%token BREAK CONTINUE
%token SWITCH CASE DEFAULT
%token RIGHT_ARROW  // '->' 运算符，用于指针成员访问

%token <std::string> IDENTIFIER TEXT
%token <int> INTEGER
%token <char> CHARACTER
```
并在 `ccpl/lexer.l` 中实现了对应的正则表达式规则。


## 2. 语法分析模块

语法分析器使用 Bison 实现，基于上下文无关文法构建抽象语法树 AST。
### 文法设计理念

CCPL 的文法设计遵循以下核心设计理念：
1. **复用性**：文法规则设计具有良好的扩展性，便于添加新的语言特性
2. **通用性**：采用经典的 C 风格文法结构，符合程序员直觉
3. **合理性**：文法层次清晰，避免左递归和歧义
4. **简洁性**：使用最少的规则表达完整的语言语法
5. **可维护性**：模块化设计，便于理解和修改

### 完整的文法设计
CCPL 采用分层文法结构，从程序级别逐步细化到表达式级别：
#### 程序结构文法

```bison
program: decl_list ;

decl_list: decl_item
         | decl_list decl_item
         | decl_list type_spec var_decl_list EOL
         | type_spec var_decl_list EOL ;

decl_item: func_decl | struct_decl ;
```

#### 函数声明文法

```bison
func_decl: type_spec func_name '(' param_list ')' block
         | func_name '(' param_list ')' block ;  # 默认返回int类型

func_name: IDENTIFIER ;

param_list: /* empty */ | param_decl_list ;

param_decl_list: param_decl | param_decl_list ',' param_decl ;

param_decl: type_spec var_declarator ;
```

#### 结构体声明文法

```bison
struct_decl: STRUCT IDENTIFIER '{' struct_field_list '}' EOL ;

struct_field_list: type_spec var_decl_list EOL
                 | struct_field_list type_spec var_decl_list EOL ;
```

#### 类型系统文法

```bison
type_spec: INT | CHAR | VOID | STRUCT IDENTIFIER ;

var_declarator: '*' var_declarator              # 指针类型
              | direct_declarator ;             # 直接声明符

direct_declarator: IDENTIFIER                   # 简单变量
                 | direct_declarator '[' INTEGER ']' ;  # 数组类型

var_decl_list: var_declarator
             | var_decl_list ',' var_declarator
             | var_declarator '=' expression    # 带初始化的声明
             | var_decl_list ',' var_declarator '=' expression ;
```

#### 语句文法

```bison
statement: if_stmt | while_stmt | for_stmt | expr_stmt EOL
         | input_stmt EOL | output_stmt EOL | return_stmt EOL
         | BREAK EOL | CONTINUE EOL | switch_stmt
         | CASE INTEGER ':' | DEFAULT ':' ;

block: '{' stmt_list '}' ;

stmt_list: /* empty */ | stmt_list statement
         | stmt_list type_spec var_decl_list EOL ;  # 局部变量声明

if_stmt: IF '(' expression ')' block
       | IF '(' expression ')' block ELSE block ;

while_stmt: WHILE '(' expression ')' block ;

for_stmt: FOR '(' expr_stmt EOL expression EOL expr_stmt ')' block ;

switch_stmt: SWITCH '(' expression ')' block ;

input_stmt: INPUT IDENTIFIER ;

output_stmt: OUTPUT expression ;

return_stmt: RETURN expression | RETURN ;
```

#### 表达式文法

```bison
expression: const_expr | func_call_expr | assign_expr | IDENTIFIER
          | member_access_expr | initializer_list_expr
          | expression '+' expression | expression '-' expression
          | expression '*' expression | expression '/' expression
          | expression EQ expression | expression NE expression
          | expression LT expression | expression LE expression
          | expression GT expression | expression GE expression
          | '-' expression %prec UMINUS           # 一元负号
          | '&' expression %prec DEREF           # 取地址
          | '*' expression %prec DEREF           # 解引用
          | '(' expression ')' ;                 # 括号表达式

const_expr: INTEGER | CHARACTER | TEXT ;

func_call_expr: IDENTIFIER '(' arg_list ')' ;

assign_expr: expression '=' expression ;

member_access_expr: expression '[' expression ']'        # 数组访问
                  | expression '.' IDENTIFIER            # 结构体成员访问
                  | expression RIGHT_ARROW IDENTIFIER ;  # 指针成员访问

initializer_list_expr: '{' arg_list '}' ;

arg_list: /* empty */ | arg_list_nonempty ;

arg_list_nonempty: expression | arg_list_nonempty ',' expression ;
```

### 运算符优先级设计
运算符优先级是语法分析中的关键设计，确保表达式按照数学惯例正确解析：
```bison
%right '='                    # 赋值运算符右结合
%left EQ NE LT LE GT GE       # 比较运算符左结合，优先级相同
%left '+' '-'                 # 加减法左结合
%left '*' '/'                 # 乘除法左结合，优先级高于加减法
%right UMINUS '&' DEREF       # 一元运算符右结合，优先级最高
```
### 语法到 AST 的生成
语法分析过程中，通过 AST 构建器创建相应的 AST 节点：
```cpp
// 在parser.y中，每个语法规则对应AST节点的创建
expression '+' expression
{
    $$ = ast_builder.make_binary_op(TAC_OP::ADD, $1, $3);
}

if_stmt: IF '(' expression ')' block ELSE block
{
    $$ = ast_builder.make_if_stmt($3, $5, $7);
}

func_decl: type_spec func_name '(' param_list ')' block
{
    $$ = ast_builder.make_func_decl($1, $2, $4, $6);
}
```
## 3. 抽象语法树 (AST) 模块
AST 是源代码的树形表示，每个节点对应语言中的一个构造。CCPL 的 AST 设计采用面向对象的层次结构，支持复杂的语言特性。
### AST 节点层次结构
CCPL 的 AST 采用多层次的类继承结构，所有节点都继承自 `ASTNode` 基类：

```cpp
// 基础节点类 - 所有AST节点的基类
struct ASTNode {
    ASTNodeKind kind;      // 节点类型标识
    int line_number;       // 源代码行号，用于错误定位
    virtual std::string to_string() const = 0;  // 序列化方法
};

// 表达式节点基类 - 所有表达式的基类
struct Expression : public ASTNode {
    std::shared_ptr<Type> expr_type;  // 表达式类型信息
};

// 语句节点基类 - 所有语句的基类  
struct Statement : public ASTNode {};

// 声明节点基类 - 所有声明的基类
struct Declaration : public Statement {};
```
#### AST 节点类型
AST 节点类型覆盖了 CCPL 语言的所有语法构造：
```cpp
enum class ASTNodeKind {
    // 表达式节点类型
    CONST_INT, CONST_CHAR, STRING_LITERAL, IDENTIFIER,
    BINARY_OP, UNARY_OP, ASSIGN, FUNC_CALL, ARRAY_ACCESS,
    MEMBER_ACCESS, ADDRESS_OF, DEREFERENCE, INITIALIZER_LIST,
    
    // 语句节点类型
    EXPR_STMT, IF_STMT, WHILE_STMT, FOR_STMT, RETURN_STMT,
    BREAK_STMT, CONTINUE_STMT, BLOCK_STMT, INPUT_STMT, OUTPUT_STMT,
    SWITCH_STMT, CASE_STMT, DEFAULT_STMT,
    
    // 声明节点类型
    VAR_DECL, FUNC_DECL, PARAM_DECL, STRUCT_DECL,
    
    // 程序节点
    PROGRAM
};
```
#### 表达式节点实现
```cpp
// 常量表达式
struct ConstIntExpr : public Expression {
    int value;  // 常量值
    ConstIntExpr(int val) : Expression(ASTNodeKind::CONST_INT), value(val) {
        expr_type = Type::make_basic(DATA_TYPE::INT);
    }
};

// 二元运算表达式
struct BinaryOpExpr : public Expression {
    TAC_OP op;                          // 操作类型
    std::shared_ptr<Expression> left;   // 左操作数
    std::shared_ptr<Expression> right;  // 右操作数
};

// 函数调用表达式
struct FuncCallExpr : public Expression {
    std::string func_name;                           // 函数名
    std::vector<std::shared_ptr<Expression>> arguments;  // 参数列表
};

// 数组访问表达式
struct ArrayAccessExpr : public Expression {
    std::shared_ptr<Expression> array;  // 数组表达式
    std::shared_ptr<Expression> index;  // 索引表达式
};

// 成员访问表达式
struct MemberAccessExpr : public Expression {
    std::shared_ptr<Expression> object;  // 对象表达式
    std::string member_name;             // 成员名
    bool is_pointer_access;              // 是否为指针访问(->)
};
```

### 类型系统设计

CCPL 的类型系统支持复杂的类型组合和嵌套：

#### 类型种类定义

```cpp
enum class TypeKind {
    BASIC,      // 基本类型: int, char, void
    POINTER,    // 指针类型: T*
    ARRAY,      // 数组类型: T[n]
    FUNCTION,   // 函数类型: T(params)
    STRUCT      // 结构体类型: struct {...}
};
```

#### 类型结构设计

```cpp
struct Type {
    TypeKind kind;
    DATA_TYPE basic_type;           // 基本类型信息
    std::shared_ptr<Type> base_type; // 用于指针、数组的基类型
    
    // 数组类型信息
    int array_size;                 // 数组大小
    
    // 函数类型信息
    std::vector<std::shared_ptr<Type>> param_types;  // 参数类型列表
    std::shared_ptr<Type> return_type;               // 返回类型
    
    // 结构体类型信息
    std::string struct_name;                         // 结构体名
    std::vector<std::pair<std::string, std::shared_ptr<Type>>> fields;  // 字段列表
};
```

#### 类型构造方法

```cpp
// 类型工厂方法
static std::shared_ptr<Type> make_basic(DATA_TYPE dt) {
    return std::make_shared<Type>(TypeKind::BASIC, dt);
}

static std::shared_ptr<Type> make_pointer(std::shared_ptr<Type> base) {
    auto t = std::make_shared<Type>(TypeKind::POINTER);
    t->base_type = base;
    return t;
}

static std::shared_ptr<Type> make_array(std::shared_ptr<Type> base, int size) {
    auto t = std::make_shared<Type>(TypeKind::ARRAY);
    t->base_type = base;
    t->array_size = size;
    return t;
}

static std::shared_ptr<Type> make_struct(const std::string& name) {
    auto t = std::make_shared<Type>(TypeKind::STRUCT);
    t->struct_name = name;
    return t;
}
```
以上代码只展示了部分类型构造方法，完整请参阅 `ccpl/src/abstraction/ast_nodes.hh`.  

### AST 树示例展示

#### 示例 1：简单算术表达式

源代码：
```c
result = a + b * 2;
```

对应的 AST 树结构：
```
AssignExpr (result = ...)
├── IdentifierExpr (result)
└── BinaryOpExpr (+)
    ├── IdentifierExpr (a)
    └── BinaryOpExpr (*)
        ├── IdentifierExpr (b)
        └── ConstIntExpr (2)
```

#### 示例 2：复杂结构体访问

源代码：
```c
student.scores[0] = 95;
```

对应的 AST 树结构：
```
AssignExpr (student.scores[0] = 95)
├── ArrayAccessExpr (student.scores[0])
│   ├── MemberAccessExpr (student.scores)
│   │   ├── IdentifierExpr (student)
│   │   └── "scores"
│   └── ConstIntExpr (0)
└── ConstIntExpr (95)
```

#### 示例 3：函数调用和条件语句

源代码：
```c
if (max(a, b) > threshold) {
    output "Exceeded";
}
```

对应的 AST 树结构：
```
IfStmt
├── BinaryOpExpr (>)
│   ├── FuncCallExpr (max(a, b))
│   │   ├── "max"
│   │   └── ArgList
│   │       ├── IdentifierExpr (a)
│   │       └── IdentifierExpr (b)
│   └── IdentifierExpr (threshold)
└── BlockStmt
    └── OutputStmt
        └── StringLiteralExpr ("Exceeded")
```

#### 示例 4：嵌套数组和结构体

源代码：
```c
cls[5].grp[2].stu[3].name[1] = 'b';
```

对应的 AST 树结构：
```
AssignExpr
├── ArrayAccessExpr (cls[5].grp[2].stu[3].name[1])
│   ├── MemberAccessExpr (cls[5].grp[2].stu[3].name)
│   │   ├── ArrayAccessExpr (cls[5].grp[2].stu[3])
│   │   │   ├── MemberAccessExpr (cls[5].grp[2].stu)
│   │   │   │   ├── ArrayAccessExpr (cls[5].grp[2])
│   │   │   │   │   ├── MemberAccessExpr (cls[5].grp)
│   │   │   │   │   │   ├── ArrayAccessExpr (cls[5])
│   │   │   │   │   │   │   ├── IdentifierExpr (cls)
│   │   │   │   │   │   │   └── ConstIntExpr (5)
│   │   │   │   │   │   └── "grp"
│   │   │   │   │   └── ConstIntExpr (2)
│   │   │   │   └── "stu"
│   │   │   └── ConstIntExpr (3)
│   │   └── "name"
│   └── ConstIntExpr (1)
└── ConstCharExpr ('b')
```

### AST 构建器

`ASTBuilder` 类提供工厂方法创建各种 AST 节点，确保类型安全和一致性：

```cpp
class ASTBuilder {
public:
    // 表达式构建方法
    std::shared_ptr<BinaryOpExpr> make_binary_op(TAC_OP op, 
        std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
    
    std::shared_ptr<IfStmt> make_if_stmt(std::shared_ptr<Expression> cond,
        std::shared_ptr<Statement> then_stmt, std::shared_ptr<Statement> else_stmt);
    
    std::shared_ptr<FuncDecl> make_func_decl(std::shared_ptr<Type> return_type,
        const std::string& name, const std::vector<std::shared_ptr<ParamDecl>>& params,
        std::shared_ptr<BlockStmt> body);
    
    // 类型构建方法
    std::shared_ptr<Type> make_basic_type(DATA_TYPE dt);
    std::shared_ptr<Type> make_pointer_type(std::shared_ptr<Type> base);
    std::shared_ptr<Type> make_array_type(std::shared_ptr<Type> base, int size);
    std::shared_ptr<Type> make_struct_type(const std::string& name);
    
    // 程序构建方法
    void add_declaration(std::shared_ptr<Declaration> decl);
    std::shared_ptr<Program> complete();
};
```

### AST 序列化

每个 AST 节点都实现了 `to_string()` 方法，用于调试和可视化：

```cpp
// 示例：BinaryOpExpr的序列化
std::string BinaryOpExpr::to_string() const {
    std::ostringstream oss;
    oss << "(" << left->to_string();
    switch (op) {
        case TAC_OP::ADD: oss << " + "; break;
        case TAC_OP::SUB: oss << " - "; break;
        // ... 其他操作符
    }
    oss << right->to_string() << ")";
    return oss.str();
}
```
例如在测试用例 `ddd.m` 中，生成的 AST 序列化输出为：
```c
struct Row {
  char[10] cols;
};

int main() {
  struct Row[5] a;
  char* p = &a[2].cols[3];
  (p[0] = 'n');
  (*(p + 1) = 'h');
  output a[2].cols[3];
  output a[2].cols[4];
  char** pp = &p;
  (**pp = 'q');
  output a[2].cols[3];
  int[5] arr;
  (*arr = 10);
  output arr[0];
  (*(arr + 1) = 20);
  output arr[1];
}
```
测试用例 `ccc.m` 的 AST 序列化输出为：
```c
//...
int main() {
  int* p = &a1;
  (p[3] = 'c');
  output a1.r.d;
  return 0;
}
```

## 4. AST to TAC 和 TAC 生成器

TAC 是一种中间表示，每个指令最多有三个操作数，便于优化和代码生成。CCPL 的 TAC 生成模块负责将 AST 转换为 TAC 指令序列。

### TAC 指令集设计

CCPL 的 TAC 指令集覆盖了语言的所有操作：

```cpp
enum class TAC_OP {
    ADD, SUB, MUL, DIV,        // 算术运算: a = b + c
    EQ, NE, LT, LE, GT, GE,    // 比较运算: a = (b == c)  
    COPY,                      // 赋值: a = b
    GOTO, IFZ, LABEL,          // 控制流: goto L, ifz b goto L
    BEGINFUNC, ENDFUNC,        // 函数: beginfunc, endfunc
    CALL, RETURN,              // 函数调用: a = call f, return a
    ACTUAL, FORMAL,            // 参数传递: actual arg, formal param
    INPUT, OUTPUT,             // I/O: input a, output a
    ADDR, LOAD_PTR, STORE_PTR  // 指针操作: a = &b, a = *b, *a = b
};
```

### 程序结构翻译过程

#### If 语句翻译

源代码：
```c
if (x > 0) {
    output x;
} else {
    output -x;
}
```

TAC 生成过程：
```cpp
std::shared_ptr<TAC> TACGenerator::do_if_else(std::shared_ptr<EXP> exp,
                                               std::shared_ptr<TAC> stmt1,
                                               std::shared_ptr<TAC> stmt2) {
    auto label1 = mk_tac(TAC_OP::LABEL, mk_label("L" + std::to_string(next_label++)));
    auto label2 = mk_tac(TAC_OP::LABEL, mk_label("L" + std::to_string(next_label++)));
    
    auto code1 = mk_tac(TAC_OP::IFZ, label1->a, exp->place);
    auto code2 = mk_tac(TAC_OP::GOTO, label2->a);
    
    code1->prev = exp->code;
    code1 = join_tac(code1, stmt1);
    code2->prev = code1;
    label1->prev = code2;
    label1 = join_tac(label1, stmt2);
    label2->prev = label1;
    
    return label2;
}
```

生成的 TAC 序列：
```
_t1 = x > 0
ifz _t1 goto L1
output x
goto L2
L1:
_t2 = -x
output _t2
L2:
```

#### While 循环翻译

源代码：
```c
while (i < 10) {
    output i;
    i = i + 1;
}
```

TAC 生成过程：
```cpp
void TACGenerator::begin_while_loop() {
    auto continue_label = mk_label("L" + std::to_string(next_label++));
    auto break_label = mk_label("L" + std::to_string(next_label++));
    enter_loop(break_label, continue_label);
}

std::shared_ptr<TAC> TACGenerator::end_while_loop(std::shared_ptr<EXP> exp, std::shared_ptr<TAC> stmt) {
    auto loop = loop_stack.back();
    auto continue_label = mk_tac(TAC_OP::LABEL, loop.continue_label);
    auto break_label = mk_tac(TAC_OP::LABEL, loop.break_label);
    
    auto ifz = mk_tac(TAC_OP::IFZ, break_label->a, exp->place);
    auto goto_continue = mk_tac(TAC_OP::GOTO, continue_label->a);
    
    auto result = continue_label;
    result = join_tac(result, exp->code);
    ifz->prev = result;
    result = join_tac(ifz, stmt);
    goto_continue->prev = result;
    break_label->prev = goto_continue;
    
    leave_loop();
    return break_label;
}
```

生成的 TAC 序列：
```
L3:
_t1 = i < 10
ifz _t1 goto L4
output i
i = i + 1
goto L3
L4:
```

#### For 循环翻译

源代码：
```c
for (i = 0; i < 10; i = i + 1) {
    output i;
}
```

TAC 生成过程：
```cpp
void TACGenerator::begin_for_loop() {
    auto loop_start = mk_label("L" + std::to_string(next_label++));
    auto continue_label = mk_label("L" + std::to_string(next_label++));
    auto break_label = mk_label("L" + std::to_string(next_label++));
    enter_loop(break_label, continue_label, loop_start);
}

std::shared_ptr<TAC> TACGenerator::end_for_loop(std::shared_ptr<TAC> init,
                                                 std::shared_ptr<EXP> cond,
                                                 std::shared_ptr<TAC> update,
                                                 std::shared_ptr<TAC> body) {
    auto loop = loop_stack.back();
    auto loop_start = mk_tac(TAC_OP::LABEL, loop.loop_start_label);
    auto continue_label = mk_tac(TAC_OP::LABEL, loop.continue_label);
    auto break_label = mk_tac(TAC_OP::LABEL, loop.break_label);
    
    auto ifz = mk_tac(TAC_OP::IFZ, loop.break_label, cond->place);
    auto goto_loop = mk_tac(TAC_OP::GOTO, loop.loop_start_label);
    
    auto result = join_tac(init, loop_start);
    result = join_tac(result, cond->code);
    ifz->prev = result;
    result = join_tac(ifz, body);
    continue_label->prev = result;
    result = join_tac(continue_label, update);
    goto_loop->prev = result;
    break_label->prev = goto_loop;
    
    leave_loop();
    return break_label;
}
```

生成的 TAC 序列：
```
i = 0
L5:
_t1 = i < 10
ifz _t1 goto L7
output i
L6:
i = i + 1
goto L5
L7:
```

#### Switch 语句翻译

源代码：
```c
switch (x) {
    case 1: output "one"; break;
    case 2: output "two"; break;
    default: output "other";
}
```

TAC 生成过程：
```cpp
void TACGenerator::begin_switch() {
    auto break_label = mk_label("L" + std::to_string(next_label++));
    auto default_label = mk_label("L" + std::to_string(next_label++));
    enter_switch(break_label, default_label);
}

std::shared_ptr<TAC> TACGenerator::do_case(int value) {
    auto& ctx = switch_stack.back();
    auto case_label = mk_label("L" + std::to_string(next_label++));
    ctx.case_labels[value] = case_label;
    return mk_tac(TAC_OP::LABEL, case_label);
}

std::shared_ptr<TAC> TACGenerator::end_switch(std::shared_ptr<EXP> exp, std::shared_ptr<TAC> body) {
    auto& ctx = switch_stack.back();
    auto switch_end = mk_tac(TAC_OP::LABEL, ctx.break_label);
    
    std::shared_ptr<TAC> case_jumps = nullptr;
    for (const auto& [case_value, case_label] : ctx.case_labels) {
        auto const_sym = mk_const(case_value);
        auto temp = mk_tmp(exp->data_type);
        auto temp_decl = mk_tac(TAC_OP::VAR, temp);
        auto sub_tac = mk_tac(TAC_OP::SUB, temp, exp->place, const_sym);
        auto case_jump = mk_tac(TAC_OP::IFZ, case_label, temp);
        
        temp_decl->prev = case_jumps;
        sub_tac->prev = temp_decl;
        case_jump->prev = sub_tac;
        case_jumps = case_jump;
    }
    
    auto goto_default = mk_tac(TAC_OP::GOTO, ctx.default_label);
    goto_default->prev = case_jumps;
    case_jumps = goto_default;
    
    auto result = join_tac(case_jumps, body);
    switch_end->prev = result;
    
    leave_switch();
    return switch_end;
}
```

生成的 TAC 序列：
```
_t1 = x - 1
ifz _t1 goto L8
_t2 = x - 2
ifz _t2 goto L9
goto L10

L8:
output "one"
goto L11

L9:
output "two"
goto L11

L10:
output "other"

L11:
```

### 结构体和数组支持

#### 结构体元数据记录机制

CCPL 编译器在编译过程中精确记录和管理结构体元数据，确保嵌套结构体和数组访问的正确内存布局计算。结构体元数据的记录遵循特定的时机和流程。

##### 结构体元数据记录时机

结构体元数据的记录发生在编译过程的以下关键阶段：

1. **语法分析阶段**：当解析到结构体声明时，创建 `StructDecl` AST 节点
2. **AST 到 TAC 转换阶段**：在 `generate_struct_decl` 方法中创建并注册结构体元数据
3. **符号表管理阶段**：通过 `TACGenerator::declare_struct_type` 方法将结构体类型信息存入符号表

##### 结构体元数据记录流程

```cpp
// 1. 在 AST 到 TAC 转换阶段记录结构体元数据
void ASTToTACGenerator::generate_struct_decl(std::shared_ptr<StructDecl> decl) {
    if (!decl)
        return;

    // 创建结构体元数据对象
    auto metadata = std::make_shared<StructTypeMetadata>(decl->name);
    
    // 记录所有字段的完整类型信息
    for (const auto &field : decl->fields) {
        StructFieldMetadata field_meta(field->name, field->var_type);
        metadata->fields.push_back(field_meta);
    }
    
    // 注册结构体类型到符号表
    tac_gen.declare_struct_type(decl->name, metadata);
}

// 2. 在 TAC 生成器中管理结构体类型
std::shared_ptr<SYM> TACGenerator::declare_struct_type(const std::string& name,
                                                       std::shared_ptr<StructTypeMetadata> metadata) {
    auto it = struct_types.find(name);
    if (it != struct_types.end()) {
        error("Struct type already declared: " + name);
        return it->second;
    }
    
    auto struct_sym = std::make_shared<SYM>();
    struct_sym->type = SYM_TYPE::STRUCT_TYPE;
    struct_sym->name = name;
    struct_sym->scope = SYM_SCOPE::GLOBAL;
    
    // 存储完整的元数据信息
    struct_sym->struct_metadata = metadata;
    
    // 计算结构体大小（包含嵌套结构体和数组）
    if (metadata) {
        metadata->calculate_size(struct_types);
    }
    
    struct_types[name] = struct_sym;
    return struct_sym;
}
```

##### 结构体变量声明时的元数据关联

当声明结构体变量时，编译器将结构体类型信息与变量关联：

```cpp
std::shared_ptr<TAC> TACGenerator::declare_struct_var(const std::string& name, 
                                                      const std::string& struct_type_name) {
    // 查找结构体类型
    auto struct_type = get_struct_type(struct_type_name);
    if (!struct_type || !struct_type->struct_metadata) {
        error("Unknown struct type: " + struct_type_name);
        return nullptr;
    }
    
    // 创建结构体变量并关联元数据
    auto var = mk_var(name, DATA_TYPE::STRUCT);
    var->struct_type_name = struct_type_name;
    var->struct_metadata = struct_type->struct_metadata;
    
    return mk_tac(TAC_OP::VAR, var);
}
```
##### 结构体元数据的使用场景

记录的结构体元数据在以下场景中被使用：

1. **成员访问计算**：通过 `generate_member_address` 方法计算字段偏移
2. **嵌套结构体访问**：支持多层嵌套结构体的精确偏移计算
3. **数组元素访问**：当结构体包含数组字段时，计算数组元素地址
4. **指针成员访问**：处理 `ptr->field` 形式的指针成员访问
5. **类型检查**：确保成员访问的类型安全性

##### 示例：结构体元数据记录过程

源代码：
```c
struct Student {
    int id;
    char name[20];
    struct Address addr;
};
```

元数据记录过程：
1. 解析 `struct Student` 声明，创建 `StructDecl` AST 节点
2. 在 `generate_struct_decl` 中创建 `StructTypeMetadata` 对象
3. 记录三个字段：`id` (int), `name` (char[20]), `addr` (struct Address)
4. 调用 `calculate_size` 计算结构体总大小
5. 通过 `declare_struct_type` 注册到符号表
6. 后续成员访问时通过 `get_struct_type("Student")` 获取元数据

**记录机制优势**：
1. **编译时计算**：所有偏移和大小在编译时计算完成
2. **精确内存布局**：支持任意深度的嵌套结构体和数组
3. **类型安全**：通过元数据确保访问的正确性
4. **运行时高效**：避免运行时计算，提高执行效率

#### 元数据表和符号表

CCPL 使用元数据表来管理复杂类型的结构信息，支持嵌套数组和结构体的精确内存布局计算。

##### 数组元数据结构

```cpp
struct ArrayMetadata {
    std::string name;                    // 完整变量名（如 "a1.a" 或 "arr"）
    std::vector<int> dimensions;         // 维度大小从外到内（如 char[5][10] 为 [5, 10]）
    int element_size;                    // 每个元素的字节大小（通常为4，表示int/char）
    DATA_TYPE base_type;                 // 基础元素类型（如 char[5][10] 为 CHAR）
    std::string struct_type_name;        // 如果base_type是STRUCT，这是结构体类型名
    
    /**
     * 计算给定维度的步长
     * 步长是在该维度递增索引时需要跳过的元素数量
     * 例如，在a[5][10]中，维度0的步长为10，维度1的步长为1
     */
    int get_stride(size_t dim_index) const {
        if (dim_index >= dimensions.size()) {
            return 0;
        }
        
        int stride = 1;
        // 乘当前维度之后的所有维度大小
        for (size_t i = dim_index + 1; i < dimensions.size(); ++i) {
            stride *= dimensions[i];
        }
        return stride;
    }
    
    /**
     * 获取数组中的总元素数
     */
    int get_total_elements() const {
        int total = 1;
        for (int dim : dimensions) {
            total *= dim;
        }
        return total;
    }
};
```

##### 结构体元数据结构

```cpp
struct StructTypeMetadata {
    std::string name;                        // 结构体类型名
    std::vector<StructFieldMetadata> fields; // 字段定义（包含完整类型信息）
    int total_size;                          // 总大小（字节）
    
    /**
     * 计算结构体总大小和字段偏移
     */
    void calculate_size(const std::unordered_map<std::string, std::shared_ptr<SYM>>& declared_structs) {
        total_size = 0;
        for (auto& field : fields) {
            field.offset = total_size;  // 当前字段的偏移
            
            if (field.type) {
                switch (field.type->kind) {
                case TypeKind::BASIC:   // int/char 4字节
                case TypeKind::POINTER: // 指针 4字节
                    total_size += 4;
                    break;
                    
                case TypeKind::ARRAY:   // 数组：元素大小 × 数组大小
                    if (field.type->base_type && field.type->array_size > 0) {
                        int element_size = calculate_type_size(field.type->base_type, declared_structs);
                        total_size += element_size * field.type->array_size;
                    } else {
                        total_size += 4; // 默认大小
                    }
                    break;
                    
                case TypeKind::STRUCT:  // 嵌套结构体
                    auto struct_it = declared_structs.find(field.type->struct_name);
                    if (struct_it != declared_structs.end() && struct_it->second->struct_metadata) {
                        total_size += struct_it->second->struct_metadata->total_size;
                    } else {
                        total_size += 4; // 默认大小
                    }
                    break;
                }
            } else {
                total_size += 4; // 默认大小
            }
        }
    }
};
```

##### 类型大小计算算法

```cpp
/**
 * 递归计算类型大小
 */
static int calculate_type_size(const std::shared_ptr<Type>& type,
                               const std::unordered_map<std::string, std::shared_ptr<SYM>>& declared_structs) {
    if (!type) return 4; // 默认大小
    
    switch (type->kind) {
    case TypeKind::BASIC:
    case TypeKind::POINTER:
        return 4;  // 基本类型和指针都是4字节
        
    case TypeKind::ARRAY:
        if (type->base_type && type->array_size > 0) {
            int element_size = calculate_type_size(type->base_type, declared_structs);
            return element_size * type->array_size;  // 数组总大小 = 元素大小 × 元素数量
        }
        return 4;
        
    case TypeKind::STRUCT:
        auto struct_it = declared_structs.find(type->struct_name);
        if (struct_it != declared_structs.end() && struct_it->second->struct_metadata) {
            return struct_it->second->struct_metadata->total_size;  // 结构体大小
        }
        return 4;
    }
    return 4;
}
```

**算法解释**：
1. **基本类型和指针**：固定为 4 字节大小
2. **数组类型**：递归计算元素类型大小，乘以数组元素数量
3. **结构体类型**：查找结构体元数据表获取总大小
4. **嵌套支持**：支持任意深度的嵌套数组和结构体

#### 复杂偏移值计算方法

CCPL 编译器区分了数组访问地址计算和成员访问地址计算，分别由不同的方法处理：

##### 方法签名

```cpp
// 计算数组访问地址的方法
std::shared_ptr<EXP> ASTToTACGenerator::generate_array_address(
    std::shared_ptr<ArrayAccessExpr> expr);

// 计算成员访问地址的方法  
std::shared_ptr<EXP> ASTToTACGenerator::generate_member_address(
    std::shared_ptr<MemberAccessExpr> expr);
```

##### 数组访问地址计算算法

```cpp
std::shared_ptr<EXP> ASTToTACGenerator::generate_array_address(
    std::shared_ptr<ArrayAccessExpr> expr) {
    
    // 1. 收集访问链：从内到外
    std::vector<std::shared_ptr<ArrayAccessExpr>> access_chain;
    std::vector<int> constant_indices; // 用于常量访问优化
    std::shared_ptr<Expression> current = expr;
    
    while (current && current->kind == ASTNodeKind::ARRAY_ACCESS) {
        auto arr_access = std::dynamic_pointer_cast<ArrayAccessExpr>(current);
        access_chain.push_back(arr_access);
        
        // 收集常量索引用于优化
        if (arr_access->index->kind == ASTNodeKind::CONST_INT) {
            auto const_idx = std::dynamic_pointer_cast<ConstIntExpr>(arr_access->index);
            constant_indices.push_back(const_idx->value);
        }
        
        current = arr_access->array;
    }
    
    // 反转以获取从外到内的顺序
    std::reverse(access_chain.begin(), access_chain.end());
    std::reverse(constant_indices.begin(), constant_indices.end());
    
    // 2. 获取基础数组元数据
    std::string base_array_name = current->to_string();
    auto metadata = get_array_metadata(base_array_name);
    if (!metadata) {
        metadata = infer_array_metadata_from_access(current, base_array_name);
    }
    
    // 3. 计算基础地址
    std::shared_ptr<EXP> base_addr;
    if (current->kind == ASTNodeKind::MEMBER_ACCESS) {
        // 成员访问如 a1[0].a - 获取成员地址
        auto member = std::dynamic_pointer_cast<MemberAccessExpr>(current);
        base_addr = generate_member_address(member);
    } else if (current->kind == ASTNodeKind::ARRAY_ACCESS) {
        // 嵌套数组访问如 a[i][j] 访问 a[i]
        auto arr = std::dynamic_pointer_cast<ArrayAccessExpr>(current);
        base_addr = generate_array_address(arr);
    } else {
        // 简单标识符 - 获取变量地址
        auto base_var = tac_gen.get_var(base_array_name);
        auto base_exp = tac_gen.mk_exp(base_var, nullptr);
        base_addr = tac_gen.do_address_of(base_exp);
    }
    
    // 4. 计算元素偏移
    std::shared_ptr<EXP> element_offset = nullptr;
    int constant_element_offset = 0;
    
    for (size_t dim = 0; dim < access_chain.size(); ++dim) {
        if (dim < constant_indices.size() && 
            access_chain[dim]->index->kind == ASTNodeKind::CONST_INT) {
            // 常量索引优化
            constant_element_offset += constant_indices[dim] * metadata->get_stride(dim);
            continue;
        }
        
        auto index_exp = generate_expression(access_chain[dim]->index);
        int stride = metadata->get_stride(dim);
        std::shared_ptr<EXP> scaled_index;
        
        if (stride == 1) {
            scaled_index = index_exp;
        } else {
            auto stride_sym = tac_gen.mk_const(stride);
            auto stride_exp = tac_gen.mk_exp(stride_sym, nullptr);
            scaled_index = tac_gen.do_bin(TAC_OP::MUL, index_exp, stride_exp);
        }
        
        element_offset = element_offset ? 
            tac_gen.do_bin(TAC_OP::ADD, element_offset, scaled_index) : scaled_index;
    }
    
    // 5. 转换为字节偏移并计算最终地址
    std::shared_ptr<EXP> dynamic_byte_offset = nullptr;
    int constant_byte_offset = constant_element_offset * metadata->element_size;
    
    if (element_offset) {
        if (metadata->element_size == 1) {
            dynamic_byte_offset = element_offset;
        } else {
            auto size_const = tac_gen.mk_const(metadata->element_size);
            auto size_exp = tac_gen.mk_exp(size_const, nullptr);
            dynamic_byte_offset = tac_gen.do_bin(TAC_OP::MUL, element_offset, size_exp);
        }
    }
    
    // 6. 生成最终地址：base_addr + dynamic_byte_offset + constant_byte_offset
    auto result_temp = tac_gen.mk_tmp(DATA_TYPE::INT);
    result_temp->is_pointer = true;
    
    if (dynamic_byte_offset && constant_byte_offset != 0) {
        // 同时有动态和常量偏移
        auto temp1 = tac_gen.mk_tmp(DATA_TYPE::INT);
        temp1->is_pointer = true;
        
        // temp1 = base_addr + dynamic_byte_offset
        auto add1_tac = tac_gen.mk_tac(TAC_OP::ADD, temp1, base_addr->place, dynamic_byte_offset->place);
        
        // result = temp1 + constant_byte_offset
        auto const_offset = tac_gen.mk_const(constant_byte_offset);
        auto add2_tac = tac_gen.mk_tac(TAC_OP::ADD, result_temp, temp1, const_offset);
        
        return tac_gen.mk_exp(result_temp, add2_tac);
    } else if (dynamic_byte_offset) {
        // 只有动态偏移
        auto add_tac = tac_gen.mk_tac(TAC_OP::ADD, result_temp, base_addr->place, dynamic_byte_offset->place);
        return tac_gen.mk_exp(result_temp, add_tac);
    } else if (constant_byte_offset != 0) {
        // 只有常量偏移
        auto const_offset = tac_gen.mk_const(constant_byte_offset);
        auto add_tac = tac_gen.mk_tac(TAC_OP::ADD, result_temp, base_addr->place, const_offset);
        return tac_gen.mk_exp(result_temp, add_tac);
    } else {
        // 没有偏移，直接返回基础地址
        return base_addr;
    }
}
```

**算法解释**：
1. **访问链收集**：从内到外收集所有数组访问表达式
2. **常量索引优化**：识别常量索引进行编译时计算
3. **基础地址计算**：根据基础表达式类型（标识符、成员访问、嵌套数组）计算基础地址
4. **元素偏移计算**：使用步长计算每个维度的线性偏移
5. **字节偏移转换**：将元素偏移转换为字节偏移
6. **最终地址合成**：组合基础地址和偏移得到最终地址

##### 成员访问地址计算算法

```cpp
std::shared_ptr<EXP> ASTToTACGenerator::generate_member_address(
    std::shared_ptr<MemberAccessExpr> expr) {
    
    // 1. 获取对象基础地址
    std::shared_ptr<EXP> base_addr;
    std::string struct_type_name;
    
    if (expr->object->kind == ASTNodeKind::IDENTIFIER) {
        // 简单情况：obj.field
        auto id = std::dynamic_pointer_cast<IdentifierExpr>(expr->object);
        auto base_var = tac_gen.get_var(id->name);
        
        if (expr->is_pointer_access && base_var->is_pointer) {
            // 指针成员访问：ptr->field
            base_addr = tac_gen.mk_exp(base_var, nullptr);
            struct_type_name = base_var->struct_type_name;
        } else {
            // 普通成员访问：obj.field
            auto base_exp = tac_gen.mk_exp(base_var, nullptr);
            base_addr = tac_gen.do_address_of(base_exp);
            struct_type_name = base_var->struct_type_name;
        }
    } else if (expr->object->kind == ASTNodeKind::ARRAY_ACCESS) {
        // 数组元素成员访问：arr[i].field
        auto arr_access = std::dynamic_pointer_cast<ArrayAccessExpr>(expr->object);
        base_addr = generate_array_address(arr_access);
        struct_type_name = infer_struct_type_from_expr(expr->object);
    } else if (expr->object->kind == ASTNodeKind::MEMBER_ACCESS) {
        // 嵌套成员访问：obj1.obj2.field
        auto nested = std::dynamic_pointer_cast<MemberAccessExpr>(expr->object);
        base_addr = generate_member_address(nested);
        struct_type_name = infer_struct_type_from_expr(expr->object);
    }
    
    // 2. 查找结构体类型元数据
    auto struct_type = tac_gen.get_struct_type(struct_type_name);
    auto field_meta = struct_type->struct_metadata->get_field(expr->member_name);
    
    // 3. 计算字段地址：base_addr + field_offset
    if (field_meta->offset == 0) {
        return base_addr;  // 字段在结构体开头
    } else {
        auto offset_const = tac_gen.mk_const(field_meta->offset);
        
        // 直接ADD，不使用指针算术缩放
        auto result_temp = tac_gen.mk_tmp(DATA_TYPE::INT);
        result_temp->is_pointer = true;
        auto result_decl = tac_gen.mk_tac(TAC_OP::VAR, result_temp);
        result_decl->prev = base_addr->code;
        
        auto add_tac = tac_gen.mk_tac(TAC_OP::ADD, result_temp, base_addr->place, offset_const);
        add_tac->prev = result_decl;
        
        auto result = tac_gen.mk_exp(result_temp, add_tac);
        result->data_type = DATA_TYPE::INT;
        return result;
    }
}
```

**算法解释**：
1. **基础地址获取**：根据对象类型（标识符、数组访问、嵌套成员访问）获取基础地址
2. **结构体类型推断**：通过符号表推断结构体类型名
3. **字段偏移查找**：从结构体元数据中查找字段偏移量
4. **地址计算**：基础地址 + 字段偏移得到最终字段地址
5. **指针访问处理**：区分普通成员访问（obj. Field）和指针成员访问（ptr->field）

##### 完整示例展示

**示例：`cls[5].grp[2].stu[3].name[1]` 的完整计算过程**

源代码：
```c
cls[5].grp[2].stu[3].name[1] = 'b';
```

计算步骤：

1. **分解访问链**：
   - `cls[5]` → 数组访问
   - `.grp` → 成员访问
   - `[2]` → 数组访问
   - `.stu` → 成员访问
   - `[3]` → 数组访问
   - `.name` → 成员访问
   - `[1]` → 数组访问

2. **从内到外计算**：
   - 计算 `cls[5]` 的地址
   - 计算 `cls[5].grp` 的成员地址
   - 计算 `cls[5].grp[2]` 的数组地址
   - 计算 `cls[5].grp[2].stu` 的成员地址
   - 计算 `cls[5].grp[2].stu[3]` 的数组地址
   - 计算 `cls[5].grp[2].stu[3].name` 的成员地址
   - 计算 `cls[5].grp[2].stu[3].name[1]` 的数组地址

3. **元数据使用**：
   - `cls` 数组元数据：维度 [10, 5]，元素大小 4，基础类型 STRUCT
   - `grp` 结构体字段：偏移 8
   - `stu` 结构体字段：偏移 16
   - `name` 结构体字段：偏移 24

4. **偏移计算**：
   - `cls[5]` 偏移：5 × 10 × 4 = 200
   - `grp` 偏移：8
   - `grp[2]` 偏移：2 × 4 = 8
   - `stu` 偏移：16
   - `stu[3]` 偏移：3 × 4 = 12
   - `name` 偏移：24
   - `name[1]` 偏移：1 × 4 = 4
   - 总偏移：200 + 8 + 8 + 16 + 12 + 24 + 4 = 272

**算法优势**：
1. **精确计算**：使用元数据表确保偏移计算的准确性
2. **常量优化**：识别常量索引进行编译时计算
3. **类型安全**：通过类型推断确保访问的正确性
4. **嵌套支持**：支持任意深度的数组和结构体嵌套
5. **性能优化**：减少运行时计算，提高代码效率

#### 符号表管理

`TACGenerator` 维护多级符号表：

```cpp
class TACGenerator {
private:
    // 全局符号表：函数、全局变量、常量
    std::unordered_map<std::string, std::shared_ptr<SYM>> sym_tab_global;
    
    // 局部符号表：局部变量、参数
    std::unordered_map<std::string, std::shared_ptr<SYM>> sym_tab_local;
    
    // 结构体类型表：结构体定义
    std::unordered_map<std::string, std::shared_ptr<SYM>> struct_types;
    
    // 临时变量计数器
    int next_tmp;
    
    // 标签计数器  
    int next_label;
    
public:
    // 符号查找：先局部后全局
    std::shared_ptr<SYM> lookup_sym(const std::string& name) {
        if (scope == SYM_SCOPE::LOCAL) {
            auto it = sym_tab_local.find(name);
            if (it != sym_tab_local.end()) return it->second;
        }
        auto it = sym_tab_global.find(name);
        return it != sym_tab_global.end() ? it->second : nullptr;
    }
    
    // 临时变量生成
    std::shared_ptr<SYM> mk_tmp(DATA_TYPE dtype) {
        std::string name = "@t" + std::to_string(next_tmp++);
        auto sym = std::make_shared<SYM>();
        sym->type = SYM_TYPE::VAR;
        sym->data_type = dtype;
        sym->name = name;
        sym->scope = scope;
        return sym;
    }
};
```

### 指针支持机制

#### 指针运算

CCPL 支持完整的指针运算，包括取地址、解引用和指针算术：

```cpp
// 取地址操作：&var
std::shared_ptr<EXP> TACGenerator::do_address_of(std::shared_ptr<EXP> exp) {
    auto temp = mk_tmp(DATA_TYPE::INT);
    temp->is_pointer = true;
    auto temp_decl = mk_tac(TAC_OP::VAR, temp);
    temp_decl->prev = exp->code;
    
    auto addr_tac = mk_tac(TAC_OP::ADDR, temp, exp->place);
    addr_tac->prev = temp_decl;
    
    return mk_exp(temp, addr_tac);
}

// 解引用操作：*ptr
std::shared_ptr<EXP> TACGenerator::do_dereference(std::shared_ptr<EXP> exp) {
    auto temp = mk_tmp(DATA_TYPE::INT);
    auto temp_decl = mk_tac(TAC_OP::VAR, temp);
    temp_decl->prev = exp->code;
    
    auto deref_tac = mk_tac(TAC_OP::LOAD_PTR, temp, exp->place);
    deref_tac->prev = temp_decl;
    
    return mk_exp(temp, deref_tac);
}

// 指针赋值：*ptr = value
std::shared_ptr<TAC> TACGenerator::do_pointer_assign(std::shared_ptr<EXP> ptr_exp, 
                                                      std::shared_ptr<EXP> value_exp) {
    auto code = join_tac(ptr_exp->code, value_exp->code);
    auto store_tac = mk_tac(TAC_OP::STORE_PTR, ptr_exp->place, value_exp->place);
    store_tac->prev
```

## 5. 优化模块

CCPL 的优化模块在 TAC（三地址码）级别进行，采用多轮迭代的优化策略，结合局部优化和全局优化技术。优化器通过分析控制流图和数据流信息，实现多种经典编译器优化算法。

### 优化器架构设计

优化器采用分层架构，包含局部优化、全局优化和高级优化三个层次：

```cpp
class TACOptimizer {
private:
    std::shared_ptr<TAC> tac_first;      // TAC指令链首指针
    BlockBuilder block_builder;          // 基本块构建器
    
    // 局部优化方法
    bool local_constant_folding(std::shared_ptr<TAC> tac, std::shared_ptr<TAC> end);
    bool local_copy_propagation(std::shared_ptr<TAC> tac, std::shared_ptr<TAC> end);
    bool local_binary_swapping(std::shared_ptr<TAC> tac, std::shared_ptr<TAC> end);
    bool local_chain_folding(std::shared_ptr<TAC> tac, std::shared_ptr<TAC> end);
    void optimize_block_local(std::shared_ptr<BasicBlock> block);
    
    // 全局优化方法
    bool global_constant_propagation(const std::vector<std::shared_ptr<BasicBlock>>& blocks);
    bool global_dead_code_elimination(const std::vector<std::shared_ptr<BasicBlock>>& blocks);
    
    // 高级优化方法
    bool common_subexpression_elimination(std::shared_ptr<BasicBlock> block);
    bool loop_invariant_code_motion(std::shared_ptr<BasicBlock> loop_header);
    bool simplify_control_flow(std::shared_ptr<TAC> tac_start);
    bool eliminate_unreachable_code(std::vector<std::shared_ptr<BasicBlock>>& blocks);
    bool eliminate_unused_var_declarations(std::shared_ptr<TAC> tac_start);
    
public:
    TACOptimizer(std::shared_ptr<TAC> first) : tac_first(first), block_builder(first) {}
    void optimize();  // 主优化入口
};
```

### 基本块划分与控制流图构建

优化过程首先将 TAC 指令序列划分为基本块，并构建控制流图（CFG）：

#### 基本块定义
- **单一入口点**：第一条指令是唯一入口
- **单一出口点**：最后一条指令是唯一出口
- **无内部分支**：除末尾外无跳转指令

#### 控制流图结构

```cpp
struct BasicBlock {
    int id;                                    // 基本块标识符
    std::shared_ptr<TAC> start;                // 起始指令
    std::shared_ptr<TAC> end;                  // 结束指令
    std::vector<std::shared_ptr<BasicBlock>> predecessors;  // 前驱块
    std::vector<std::shared_ptr<BasicBlock>> successors;    // 后继块
};
```

**算法**：
1. **识别 leader 节点**：程序入口、跳转目标、跳转后的指令
2. **构建基本块**：从领导者到下一个领导者之间的指令序列
3. **建立控制流**：根据跳转指令连接基本块

### 局部优化技术

局部优化在基本块内部进行，不依赖跨块信息。

#### 常量折叠（Constant Folding）

**功能**：在编译时计算常量表达式，减少运行时计算。

```cpp
bool TACOptimizer::local_constant_folding(std::shared_ptr<TAC> tac, std::shared_ptr<TAC> end) {
    bool changed = false;
    auto current = tac;

    while (current != nullptr) {
        // 二元运算：a = b op c，其中b和c都是常量
        if (current->op == TAC_OP::ADD || current->op == TAC_OP::SUB ||
            current->op == TAC_OP::MUL || current->op == TAC_OP::DIV) {
            int val_b, val_c;
            if (current->b->get_const_value(val_b) && current->c->get_const_value(val_c)) {
                int result = 0;
                bool valid = true;

                switch (current->op) {
                case TAC_OP::ADD: result = val_b + val_c; break;
                case TAC_OP::SUB: result = val_b - val_c; break;
                case TAC_OP::MUL: result = val_b * val_c; break;
                case TAC_OP::DIV: 
                    if (val_c != 0) result = val_b / val_c;
                    else { valid = false; warning("Constant Folding", "Division by zero!!!"); }
                    break;
                }

                if (valid) {
                    current->op = TAC_OP::COPY;  // 转换为拷贝指令
                    current->b = make_const(result);
                    current->c = nullptr;
                    changed = true;
                }
            }
        }
        // 比较运算：a = (b relop c)，其中b和c都是常量
        else if (current->op == TAC_OP::EQ || current->op == TAC_OP::NE ||
                 current->op == TAC_OP::LT || current->op == TAC_OP::LE ||
                 current->op == TAC_OP::GT || current->op == TAC_OP::GE) {
            int val_b, val_c;
            if (current->b->get_const_value(val_b) && current->c->get_const_value(val_c)) {
                int result = 0;
                switch (current->op) {
                case TAC_OP::EQ: result = (val_b == val_c) ? 1 : 0; break;
                case TAC_OP::NE: result = (val_b != val_c) ? 1 : 0; break;
                case TAC_OP::LT: result = (val_b < val_c) ? 1 : 0; break;
                case TAC_OP::LE: result = (val_b <= val_c) ? 1 : 0; break;
                case TAC_OP::GT: result = (val_b > val_c) ? 1 : 0; break;
                case TAC_OP::GE: result = (val_b >= val_c) ? 1 : 0; break;
                }
                current->op = TAC_OP::COPY;
                current->b = make_const(result);
                current->c = nullptr;
                changed = true;
            }
        }
        // 一元运算：a = -b，其中b是常量
        else if (current->op == TAC_OP::NEG) {
            int val_b;
            if (current->b->get_const_value(val_b)) {
                current->op = TAC_OP::COPY;
                current->b = make_const(-val_b);
                current->c = nullptr;
                changed = true;
            }
        }
        if(current==end) break;
        current = current->next;
    }
    return changed;
}
```

**算法解释**：
1. **识别常量操作数**：检查操作数是否为常量
2. **编译时计算**：根据操作符类型执行相应的计算
3. **指令替换**：将计算指令替换为拷贝指令，直接使用计算结果
4. **安全性检查**：除法操作检查除零错误


#### 拷贝传播（Copy Propagation）

**功能**：消除不必要的拷贝操作，直接使用原始变量。

```cpp
bool TACOptimizer::local_copy_propagation(std::shared_ptr<TAC> tac, std::shared_ptr<TAC> end) {
    bool changed = false;
    std::unordered_map<std::shared_ptr<SYM>, std::shared_ptr<SYM>> copy_map;

    auto current = tac;
    while (current != nullptr) {
        // 记录拷贝关系：a = b
        if (current->op == TAC_OP::COPY && current->a && current->b &&
            current->b->type == SYM_TYPE::VAR) {
            copy_map[current->a] = current->b;
        }
        // 如果变量被重新赋值，清除其拷贝信息
        else if (current->a) {
            copy_map.erase(current->a);
        }

        // 替换使用处：将使用拷贝变量的地方替换为原始变量
        bool is_pointer_op = (current->op == TAC_OP::ADDR || 
                              current->op == TAC_OP::LOAD_PTR || 
                              current->op == TAC_OP::STORE_PTR);

        if (current->b && current->b->type == SYM_TYPE::VAR && !is_pointer_op) {
            auto it = copy_map.find(current->b);
            if (it != copy_map.end()) {
                current->b = it->second;
                changed = true;
            }
        }

        if (current->c && current->c->type == SYM_TYPE::VAR && !is_pointer_op) {
            auto it = copy_map.find(current->c);
            if (it != copy_map.end()) {
                current->c = it->second;
                changed = true;
            }
        }

        if(current==end) break;
        current = current->next;
    }
    return changed;
}
```

**算法解释**：
1. **建立拷贝映射**：记录变量之间的拷贝关系
2. **传播替换**：在使用拷贝变量的地方直接使用原始变量
3. **失效处理**：当变量被重新赋值时，清除相关的拷贝信息
4. **指针安全**：避免对指针操作进行传播，防止别名问题


#### 二元交换（Binary Swapping）

**功能**：利用交换律将常量操作数移到右侧，便于链式表达式折叠优化。

```cpp
bool TACOptimizer::local_binary_swapping(std::shared_ptr<TAC> tac, std::shared_ptr<TAC> end) {
    bool changed = false;
    auto current = tac;
    while (current != nullptr) {
        // 交换操作数：const + var -> var + const
        if ((current->op == TAC_OP::ADD || current->op == TAC_OP::MUL) &&
            current->b && current->c &&
            (current->b->type == SYM_TYPE::CONST_INT && current->c->type == SYM_TYPE::VAR)) {
            std::swap(current->b, current->c);
            changed = true;
        }
        if(current==end) break;
        current = current->next;
    }
    return changed;
}
```

**算法解释**：
1. **识别交换模式**：查找形如 `const + var` 的表达式
2. **应用交换律**：交换操作数位置为 `var + const`
3. **优化准备**：为后续的常量折叠和链式折叠优化做准备


#### 链式表达式折叠（Chain Folding）

**功能**：利用加法乘法结合律合并连续的常量运算，减少中间临时变量。`

```cpp
bool TACOptimizer::local_chain_folding(std::shared_ptr<TAC> tac, std::shared_ptr<TAC> end) {
    bool changed = false;
    
    // 记录每个变量的定义信息
    struct DefInfo {
        TAC_OP op;
        std::shared_ptr<SYM> left;
        std::shared_ptr<SYM> right;
        std::shared_ptr<TAC> tac;
    };
    std::unordered_map<std::shared_ptr<SYM>, DefInfo> def_map;
    
    auto current = tac;
    while (current != nullptr) {
        // 处理加法和减法链
        if ((current->op == TAC_OP::ADD || current->op == TAC_OP::SUB) && 
            current->a && current->b && current->c) {
            
            def_map[current->a] = {current->op, current->b, current->c, current};
            
            // 情况1: t2 = t1 op2 const2，其中 t1 = var op1 const1
            if (current->b->type == SYM_TYPE::VAR) {
                auto it = def_map.find(current->b);
                if (it != def_map.end()) {
                    const auto& def = it->second;
                    int const1, const2;
                    bool left_has_const = def.right && def.right->get_const_value(const1);
                    bool right_is_const = current->c->get_const_value(const2);
                    
                    if (left_has_const && right_is_const && def.left && def.left->type == SYM_TYPE::VAR) {
                        int combined_const = 0;
                        TAC_OP new_op = def.op;
                        
                        // 计算合并后的常量
                        if (def.op == TAC_OP::ADD && current->op == TAC_OP::ADD) {
                            combined_const = const1 + const2;  // var + c1 + c2 = var + (c1+c2)
                        } else if (def.op == TAC_OP::ADD && current->op == TAC_OP::SUB) {
                            combined_const = const1 - const2;  // var + c1 - c2 = var + (c1-c2)
                        } else if (def.op == TAC_OP::SUB && current->op == TAC_OP::ADD) {
                            combined_const = const2 - const1;  // var - c1 + c2 = var + (c2-c1)
                        } else if (def.op == TAC_OP::SUB && current->op == TAC_OP::SUB) {
                            combined_const = const1 + const2;  // var - c1 - c2 = var - (c1+c2)
                        }
                        
                        // 如果合并后的常量为负数且操作是ADD，转换为SUB
                        if (new_op == TAC_OP::ADD && combined_const < 0) {
                            new_op = TAC_OP::SUB;
                            combined_const = -combined_const;
                        } else if (new_op == TAC_OP::SUB && combined_const < 0) {
                            new_op = TAC_OP::ADD;
                            combined_const = -combined_const;
                        }
                        
                        // 应用优化：直接使用原始变量和合并后的常量
                        current->op = new_op;
                        current->b = def.left;  // 使用链条起始的变量
                        current->c = make_const(combined_const);
                        
                        // 更新定义映射
                        def_map[current->a] = {new_op, def.left, current->c, current};
                        
                        changed = true;
                    }
                }
            }
            
            // 情况2: t2 = const2 op2 t1，其中 t1 = const1 op1 var (仅适用于加法交换律)
            if (current->op == TAC_OP::ADD && current->c->type == SYM_TYPE::VAR) {
                auto it = def_map.find(current->c);
                if (it != def_map.end()) {
                    const auto& def = it->second;
                    
                    int const1, const2;
                    bool left_is_const = current->b->get_const_value(const2);
                    bool def_left_is_const = def.left && def.left->get_const_value(const1);
                    
                    // t1 = const1 + var; t2 = const2 + t1
                    if (def.op == TAC_OP::ADD && left_is_const && def_left_is_const &&
                        def.right && def.right->type == SYM_TYPE::VAR) {
                        // const2 + (const1 + var) = (const1+const2) + var
                        int combined_const = const1 + const2;
                        
                        current->op = TAC_OP::ADD;
                        current->b = def.right;  // 使用原始变量
                        current->c = make_const(combined_const);
                        
                        def_map[current->a] = {TAC_OP::ADD, def.right, current->c, current};
                        
                        changed = true;
                    }
                }
            }
        }
        // 如果变量被重新定义，清除其定义信息
        else if (current->a && current->a->type == SYM_TYPE::VAR) {
            def_map.erase(current->a);
        }
        
        if (current == end)
            break;
        current = current->next;
    }
    
    return changed;
}
```

**算法解释**：
1. **跟踪定义链**：记录每个变量的定义信息，包括操作符和操作数
2. **识别链式模式**：查找形如 `t1 = var + const1; t2 = t1 + const2` 的表达式链
3. **常量合并**：根据操作符组合规则合并常量部分
4. **符号简化**：处理负常数，将 `var + (-c)` 转换为 `var - c`
5. **链式消除**：用合并后的表达式替换原始链式表达式

对于测试用例 `opt_test4.m`，如果只有常量折叠和常量传播，则无法进行优化，因为计算链上存在前置未知数，其后的所有相关表达式计算均被视为变量。但是实际上可以利用加法交换律折叠它们。以下是该测试用例生成的原始 TAC：
```TAC
label main
begin
var a : int
a = 1
var b : int
var c : int
c = 9
var d : int
d = 4
var e : int
input b
var @t0 : int
@t0 = a + b
var @t1 : int
@t1 = @t0 - c
var @t2 : int
@t2 = @t1 + d
e = @t2
output e
return 0
end
```
优化后，表达式 `+a-c+d` 被折叠为一次计算：
```TAC
label main
begin
var b : int
var e : int
input b
var @t2 : int
@t2 = b - 4
e = @t2
output e
return 0
end
```
此外，这项优化还能对数组和结构体成员访问生成的偏移值计算进行进一步优化。一般的偏移值因为前置包含取基地址而无法被常量折叠和传播优化。
### 全局优化技术
全局优化基于控制流图和数据流分析，跨越基本块边界进行优化。
#### 全局常量传播（Global Constant Propagation）
**功能**：在整个函数范围内传播常量值，减少运行时计算。
```cpp
bool TACOptimizer::global_constant_propagation(const std::vector<std::shared_ptr<BasicBlock>>& blocks) {
    bool changed = false;
    const int BOTTOM = INT_MIN;
    const auto& block_in = block_builder.get_block_in();
    
    for (auto& block : blocks) {
        auto& in_constants = block_in.at(block).constants;
        auto current_constants = in_constants;
        
        auto tac = block->start;
        while (tac) {
            // 替换使用的变量为常量
            bool is_pointer_op = (tac->op == TAC_OP::ADDR || 
                                  tac->op == TAC_OP::LOAD_PTR || 
                                  tac->op == TAC_OP::STORE_PTR);
            
            if (tac->b && tac->b->type == SYM_TYPE::VAR && !is_pointer_op) {
                auto it = current_constants.find(tac->b);
                if (it != current_constants.end() && it->second != BOTTOM) {
                    tac->b = make_const(it->second);
                    changed = true;
                }
            }
            
            if (tac->c && tac->c->type == SYM_TYPE::VAR && !is_pointer_op) {
                auto it = current_constants.find(tac->c);
                if (it != current_constants.end() && it->second != BOTTOM) {
                    tac->c = make_const(it->second);
                    changed = true;
                }
            }
            
            // 特殊指令处理
            if (tac->op == TAC_OP::RETURN || tac->op == TAC_OP::OUTPUT ||
                tac->op == TAC_OP::IFZ || tac->op == TAC_OP::ACTUAL) {
                if (tac->a && tac->a->type == SYM_TYPE::VAR) {
                    auto it = current_constants.find(tac->a);
                    if (it != current_constants.end() && it->second != BOTTOM) {
                        tac->a = make_const(it->second);
                        changed = true;
                    }
                }
            }
            
            // 更新当前常量状态
            auto def = tac->get_def();
            if (def) {
                if (tac->op == TAC_OP::COPY) {
                    int val;
                    if (tac->b->get_const_value(val)) {
                        current_constants[def] = val;  // 记录常量值
                    } else {
                        current_constants[def] = BOTTOM;  // 标记为非常量
                    }
                } else {
                    current_constants[def] = BOTTOM;  // 复杂计算，标记为非常量
                }
            }
            
            if (tac == block->end) break;
            tac = tac->next;
        }
    }
    
    return changed;
}
```

**算法解释**：
1. **数据流分析**：使用前向数据流分析计算每个基本块入口处的常量状态
2. **常量替换**：在指令中使用已知的常量值替换变量
3. **状态更新**：根据指令类型更新常量状态
4. **安全传播**：避免对指针操作进行常量传播

#### 全局死代码消除（Global Dead Code Elimination）
**功能**：基于活跃变量分析删除不会被使用的变量定义。
```cpp
bool TACOptimizer::global_dead_code_elimination(const std::vector<std::shared_ptr<BasicBlock>>& blocks) {
    bool changed = false;
    const auto& block_out = block_builder.get_block_out();
    
    for (auto& block : blocks) {
        auto& out_live = block_out.at(block).live_vars;
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
            auto def = instr->get_def();
            
            // 如果定义的变量不活跃，且指令没有副作用，可以删除
            if (def && current_live.find(def) == current_live.end()) {
                // 检查是否有副作用（CALL, INPUT 等不能删除）
                if (instr->op != TAC_OP::CALL && instr->op != TAC_OP::INPUT &&
                    instr->op != TAC_OP::LOAD_PTR && instr->op != TAC_OP::STORE_PTR) {
                    // 从指令链中删除该指令
                    auto instr_prev = instr->prev;
                    auto instr_next = instr->next;
                    
                    if (instr_prev) instr_prev->next = instr_next;
                    if (instr_next) instr_next->prev = instr_prev;
                    
                    // 更新块的起始或结束指针
                    if (instr == block->start) block->start = instr_next;
                    if (instr == block->end) block->end = instr_prev;
                    
                    // 清空被删除指令的链接
                    instr->prev = nullptr;
                    instr->next = nullptr;
                    
                    changed = true;
                    continue;
                }
            }
            
            // 更新活跃变量集合
            if (def) current_live.erase(def);
            auto uses = instr->get_uses();
            for (auto& use : uses) current_live.insert(use);
        }
    }
    
    return changed;
}
```

**算法解释**：
1. **活跃变量分析**：使用后向数据流分析计算每个基本块出口处的活跃变量
2. **逆序处理**：从基本块末尾向前处理指令
3. **死代码识别**：如果定义的变量在后续不被使用，且指令无副作用，则删除
4. **副作用保护**：保留有副作用的指令（函数调用、I/O 操作等）

### 高级优化技术
#### 公共子表达式消除（Common Subexpression Elimination）
**功能**：识别并消除重复计算的相同表达式。

```cpp
bool TACOptimizer::common_subexpression_elimination(std::shared_ptr<BasicBlock> block) {
    bool changed = false;
    
    // 表达式 -> 结果变量的映射
    std::unordered_map<std::string, std::shared_ptr<SYM>> expr_map;
    
    auto tac = block->start;
    while (tac) {
        std::string key = get_expression_key(tac);
        
        if (!key.empty() && tac->a && tac->a->type == SYM_TYPE::VAR) {
            auto it = expr_map.find(key);
            if (it != expr_map.end()) {
                // 找到相同的表达式，替换为拷贝
                tac->op = TAC_OP::COPY;
                tac->b = it->second;    // 使用之前计算的结果
                tac->c = nullptr;
                changed = true;
            } else {
                // 记录新表达式
                expr_map[key] = tac->a;
            }
        }
        
        // 如果有变量被重新定值，使相关表达式失效
        auto def = tac->get_def();
        if (def) {
            // 移除包含该变量的表达式
            std::vector<std::string> to_remove;
            for (auto& [expr_key, var] : expr_map) {
                if (expr_key.find(def->to_string()) != std::string::npos) {
                    to_remove.push_back(expr_key);
                }
            }
            for (const auto& key : to_remove) expr_map.erase(key);
        }
        
        if (tac == block->end) break;
        tac = tac->next;
    }
    
    return changed;
}
```

**算法解释**：
1. **表达式哈希**：为每个表达式生成唯一键值
2. **重复检测**：检查当前表达式是否已经计算过
3. **结果复用**：用之前的结果替换重复计算
4. **失效处理**：当操作数被重新赋值时，清除相关的表达式缓存

#### 循环不变量外提（Loop Invariant Code Motion）
**功能**：将循环中不变的计算移到循环外部。
```cpp
bool TACOptimizer::loop_invariant_code_motion(std::shared_ptr<BasicBlock> loop_header) {
    bool changed = false;

    // 查找循环体中的所有基本块
    std::unordered_set<std::shared_ptr<BasicBlock>> loop_blocks;
    find_loop_blocks(loop_header, loop_blocks);

    if (loop_blocks.size() <= 1) return false;

    // 收集循环内的指令和定义信息
    std::vector<std::shared_ptr<TAC>> loop_instructions;
    std::unordered_map<std::shared_ptr<TAC>, std::shared_ptr<BasicBlock>> instr_block;
    std::unordered_map<std::string, std::vector<std::shared_ptr<TAC>>> defs_in_loop;
    std::unordered_map<std::string, std::shared_ptr<TAC>> var_decl_in_loop;

    // 记录循环内的所有指令
    for (auto& block : loop_blocks) {
        if (!block || block == loop_header || !block->start || !block->end) continue;
        auto tac = block->start;
        while (tac) {
            instr_block[tac] = block;
            loop_instructions.push_back(tac);
            
            if (tac->op == TAC_OP::VAR && tac->a && tac->a->type == SYM_TYPE::VAR) {
                var_decl_in_loop[tac->a->name] = tac;
            }
            
            auto def = tac->get_def();
            if (def) defs_in_loop[def->name].push_back(tac);
            
            if (tac == block->end) break;
            tac = tac->next;
        }
    }

    // 识别可外提的不变量指令
    std::unordered_set<std::shared_ptr<TAC>> movable;
    bool progress = true;
    while (progress) {
        progress = false;
        for (auto& instr : loop_instructions) {
            if (movable.find(instr) != movable.end()) continue;
            
            // 检查是否支持的操作类型
            if (!is_supported_op(instr->op)) continue;
            
            auto def = instr->get_def();
            if (!def) continue;
            
            // 检查变量在循环中是否只有一次定义
            auto def_it = defs_in_loop.find(def->name);
            if (def_it == defs_in_loop.end() || def_it->second.size() != 1) continue;
            
            // 检查操作数是否已准备就绪（定义在循环外或已标记为可移动）
            if (!operand_ready(instr->b, movable)) continue;
            if (!operand_ready(instr->c, movable)) continue;
            
            movable.insert(instr);
            progress = true;
        }
    }

    if (movable.empty()) return false;

    // 将可移动指令移到循环前导块
    std::shared_ptr<BasicBlock> preheader = nullptr;
    for (auto& pred : loop_header->predecessors) {
        if (loop_blocks.find(pred) == loop_blocks.end()) {
            preheader = pred;
            break;
        }
    }

    if (!preheader || !preheader->end) return false;

    // 按顺序移动指令
    auto insertion_point = preheader->end;
    for (auto& instr : loop_instructions) {
        if (movable.find(instr) != movable.end()) {
            // 移动变量声明（如果需要）
            auto def = instr->get_def();
            if (def) {
                auto decl_it = var_decl_in_loop.find(def->name);
                if (decl_it != var_decl_in_loop.end()) {
                    auto decl = decl_it->second;
                    remove_from_block(decl);
                    insert_after(decl, insertion_point);
                    insertion_point = decl;
                    changed = true;
                }
            }
            
            // 移动指令本身
            remove_from_block(instr);
            insert_after(instr, insertion_point);
            insertion_point = instr;
            changed = true;
        }
    }

    if (changed) {
        std::clog << "    Loop invariant code motion: found "
                  << movable.size() << " invariant instructions" << std::endl;
    }

    return changed;
}
```

**算法解释**：
1. **循环识别**：查找循环体中的所有基本块
2. **不变量分析**：识别操作数在循环中不变的指令
3. **依赖检查**：确保指令的所有操作数在循环外定义或已标记为可移动
4. **指令外提**：将不变量指令移到循环前导块中
5. **顺序保持**：保持指令的相对顺序，避免依赖问题

#### 控制流简化（Control Flow Simplification）

**功能**：简化控制流结构，删除不必要的跳转指令。

```cpp
bool TACOptimizer::simplify_control_flow(std::shared_ptr<TAC> tac_start) {
    bool changed = false;
    auto current = tac_start;
    
    while (current) {
        // 查找 IFZ 指令
        if (current->op == TAC_OP::IFZ && current->b) {
            int cond_value;
            if (current->b->get_const_value(cond_value)) {
                // 条件是常量
                if (cond_value == 0) {
                    // ifz 0 goto L -> 总是跳转，转换为 goto L
                    current->op = TAC_OP::GOTO;
                    current->b = nullptr;
                    current->c = nullptr;
                    changed = true;
                    std::clog << "    Simplified: ifz 0 -> goto " << current->a->to_string() << std::endl;
                } else {
                    // ifz <非0> goto L -> 永不跳转，删除此指令
                    std::clog << "    Simplified: ifz " << cond_value << " -> removed (never jumps)" << std::endl;
                    
                    auto next = current->next;
                    auto prev = current->prev;
                    
                    // 删除该指令
                    if (prev) prev->next = next;
                    if (next) next->prev = prev;
                    
                    // 清空链接
                    current->prev = nullptr;
                    current->next = nullptr;
                    
                    current = next;
                    changed = true;
                    continue;
                }
            }
        }
        // 查找连续的 GOTO 指令
        else if (current->op == TAC_OP::GOTO && current->next && 
                 current->next->op == TAC_OP::LABEL) {
            // goto L; label L; -> 可以删除 goto
            if (current->a && current->next->a && 
                current->a->name == current->next->a->name) {
                std::clog << "    Removed redundant goto to next label" << std::endl;
                
                Auto next = current->next;
                Auto prev = current->prev;
                
                If (prev) prev->next = next;
                If (next) next->prev = prev;
                
                // 清空链接
                Current->prev = nullptr;
                Current->next = nullptr;
                
                Current = next;
                Changed = true;
                Continue;
            }
        }
        
        Current = current->next;
    }
    
    Return changed;
}
```

**算法解释**：
1. **常量条件优化**：将常量条件的分支转换为无条件跳转或删除
2. **冗余跳转消除**：删除指向下一标签的无用跳转
3. **控制流简化**：减少不必要的控制流转换
4. **指令删除**：安全删除无用的控制流指令


#### 不可达代码消除（Unreachable Code Elimination）

**功能**：删除永远不会被执行到的代码块。

```cpp
bool TACOptimizer:: eliminate_unreachable_code (std::vector<std::shared_ptr<BasicBlock>>& blocks) {
    Bool changed = false;
    
    If (blocks.Empty ()) return false;
    
    // 使用可达性分析
    std::unordered_set<std::shared_ptr<BasicBlock>> reachable;
    std::queue<std::shared_ptr<BasicBlock>> worklist;
    
    // 从第一个块开始（程序入口）
    Worklist.Push (blocks[0]);
    Reachable.Insert (blocks[0]);
    
    While (! Worklist.Empty ()) {
        Auto block = worklist.Front ();
        Worklist.Pop ();
        
        For (auto& succ : block->successors) {
            If (reachable.Find (succ) == reachable.End ()) {
                Reachable.Insert (succ);
                Worklist.Push (succ);
            }
        }
    }
    
    // 删除不可达的块
    std::vector<std::shared_ptr<BasicBlock>> new_blocks;
    For (auto& block : blocks) {
        If (reachable.Find (block) != reachable.End ()) {
            New_blocks. Push_back (block);
        } else {
            std:: clog << "    Removing unreachable block " << block->id << std:: endl;
            
            // 从 TAC 链表中删除该块的所有指令
            Auto tac = block->start;
            While (tac) {
                Auto next = (tac == block->end) ? Nullptr : tac->next;
                Auto prev = tac->prev;
                Auto curr_next = tac->next;
                
                If (prev) prev->next = curr_next;
                If (curr_next) curr_next->prev = prev;
                
                // 清空被删除指令的链接
                Tac->prev = nullptr;
                Tac->next = nullptr;
                
                Tac = next;
                If (! Next) break;
            }
            
            Changed = true;
        }
    }
    
    If (changed) {
        Blocks = new_blocks;
    }
    
    Return changed;
}
```

**算法解释**：
1. **可达性分析**：从程序入口开始广度优先搜索可达的基本块
2. **不可达识别**：标记无法从入口到达的基本块
3. **代码删除**：安全删除不可达块中的所有指令
4. **控制流更新**：更新基本块列表


#### 未使用变量声明消除（Unused Variable Declaration Elimination）

**功能**：删除从未被使用的变量声明。

```cpp
bool TACOptimizer:: eliminate_unused_var_declarations (std::shared_ptr<TAC> tac_start) {
    Bool changed = false;
    
    // 收集所有被使用的变量（除了声明）
    std::unordered_set<std::string> used_vars;
    
    Auto current = tac_start;
    While (current) {
        // 跳过 VAR 声明，收集其他地方使用的变量
        If (current->op != TAC_OP::VAR) {
            If (current->a && current->a->type == SYM_TYPE::VAR)
                Used_vars.Insert (current->a->name);
            If (current->b && current->b->type == SYM_TYPE::VAR)
                Used_vars.Insert (current->b->name);
            If (current->c && current->c->type == SYM_TYPE::VAR)
                Used_vars.Insert (current->c->name);
        }
        
        Current = current->next;
    }
    
    // 删除未使用的 VAR 声明
    Current = tac_start;
    While (current) {
        Auto next = current->next;
        
        If (current->op == TAC_OP:: VAR && current->a && current->a->type == SYM_TYPE::VAR) {
            // 检查该变量是否被使用
            If (used_vars.Find (current->a->name) == used_vars.End ()) {
                std:: clog << "    Removing unused var declaration: " << current->a->name << std:: endl;
                
                Auto prev = current->prev;
                Auto curr_next = current->next;
                
                // 删除该声明
                If (prev) prev->next = curr_next;
                If (curr_next) curr_next->prev = prev;
                
                // 清空链接
                Current->prev = nullptr;
                Current->next = nullptr;
                
                Changed = true;
            }
        }
        
        Current = next;
    }
    
    Return changed;
}
```

**算法解释**：
1. **使用分析**：收集所有被使用的变量名
2. **声明检查**：检查每个变量声明是否被使用
3. **声明删除**：删除未被使用的变量声明
4. **内存优化**：减少不必要的内存分配


### 优化流程

CCPL 优化器采用多轮迭代的优化策略，结合局部优化和全局优化：

```cpp
Void TACOptimizer:: optimize () {
    // 构建控制流图
    Block_builder.Build ();
    Block_builder. Print_basic_blocks (std::clog);
    Auto blocks = block_builder. Get_basic_blocks ();
    
    If (blocks.Empty ()) return;
    
    // 多轮迭代优化
    Bool global_changed = true;
    Int global_iter = 0;
    Const int MAX_GLOBAL_ITER = 20;
    
    While (global_changed && global_iter < MAX_GLOBAL_ITER) {
        Global_changed = false;
        Global_iter++;
        
        Std:: clog << "\n=== Optimization Pass " << global_iter << " ===" << std:: endl;

        // 局部优化（基本块内）
        For (auto& block : blocks) {
            Optimize_block_local (block);
        }
        
        // 公共子表达式消除（每个基本块内）
        For (auto& block : blocks) {
            If (common_subexpression_elimination (block)) {
                Global_changed = true;
                std:: clog << "  - CSE applied in block " << block->id << std:: endl;
            }
        }
        
        // 循环不变量外提
        For (auto& block : blocks) {
            If (is_loop_header (block)) {
                If (loop_invariant_code_motion (block)) {
                    Global_changed = true;
                    std:: clog << "  - LICM applied for loop at block " << block->id << std:: endl;
                }
            }
        }
        
        // 数据流分析
        Block_builder. Compute_data_flow ();
        
        // 全局优化
        If (global_constant_propagation (blocks)) {
            Global_changed = true;
            Std:: clog << "  - Global constant propagation applied" << std:: endl;
        }
        
        // 再次进行局部常量折叠（处理新产生的常量）
        For (auto& block : blocks) {
            If (local_constant_folding (block->start, block->end)) {
                Global_changed = true;
            }
        }
        
        // 全局死代码消除
        If (global_dead_code_elimination (blocks)) {
            Global_changed = true;
            Std:: clog << "  - Dead code elimination applied" << std:: endl;

            Block_builder. Build ();
            Blocks = block_builder. Get_basic_blocks ();
        }
        
        // 控制流简化
        If (simplify_control_flow (tac_first)) {
            Global_changed = true;
            Std:: clog << "  - Control flow simplification applied" << std:: endl;

            Block_builder. Build ();
            Blocks = block_builder. Get_basic_blocks ();
        }
        
        // 不可达代码消除
        If (eliminate_unreachable_code (blocks)) {
            Global_changed = true;
            Std:: clog << "  - Unreachable code elimination applied" << std:: endl;

            Block_builder. Build ();
            Blocks = block_builder. Get_basic_blocks ();
        }
    }
    
    // 最后一轮清理：删除未使用的变量声明（包括临时变量）
    If (eliminate_unused_var_declarations (tac_first)) {
        Std:: clog << "  - Unused variable declarations eliminated" << std:: endl;
    }
    
    Std:: clog << "\n=== Optimization completed in " << global_iter << " passes ===" << std:: endl;
}
```

**优化流程解释**：
1. **控制流分析**：构建基本块和控制流图
2. **局部优化**：在每个基本块内进行快速优化
3. **全局优化**：基于数据流分析进行跨块优化
4. **高级优化**：应用复杂的优化技术
5. **迭代改进**：多轮迭代直到收敛
6. **清理阶段**：最终清理未使用的声明

## 6. 目标代码生成模块

目标代码生成模块负责将优化后的 TAC（三地址码）翻译为自定义汇编代码。这一部分在示例代码 obj.c 的基础上进行了重要扩展，增加了对数组、指针和立即数指令的完整支持。

### 模块架构设计

目标代码生成器采用寄存器分配和内存管理策略，将 TAC 指令映射到目标机器的汇编指令：

```cpp
class ObjGenerator {
private:
    std::ostream& output;                    // 输出流
    TACGenerator& tac_gen;                   // TAC 生成器引用
    BlockBuilder block_builder;              // 基本块构建器
    
    // 寄存器管理
    std::array<RegDescriptor, R_NUM> reg_desc;  // 寄存器描述符数组
    
    // 内存偏移管理
    int tos;  // 静态区顶部（全局变量）
    int tof;  // 栈帧顶部（局部变量）  
    int oof;  // 形参偏移
    int oon;  // 下一帧偏移
};
```

### 寄存器分配策略

编译器采用简单的寄存器分配算法，支持 16 个通用寄存器：

```cpp
int ObjGenerator::reg_alloc(std::shared_ptr<SYM> s) {
    // 1. 检查是否已在寄存器中
    for (int r = R_GEN; r < R_NUM; r++) {
        if (reg_desc[r].var == s) {
            if (reg_desc[r].state == RegState::MODIFIED) {
                asm_write_back(r);  // 写回已修改的值
            }
            return r;
        }
    }
    
    // 2. 查找空闲寄存器
    for (int r = R_GEN; r < R_NUM; r++) {
        if (reg_desc[r].var == nullptr) {
            asm_load(r, s);
            rdesc_fill(r, s, RegState::UNMODIFIED);
            return r;
        }
    }
    
    // 3. 查找未修改的寄存器
    for (int r = R_GEN; r < R_NUM; r++) {
        if (reg_desc[r].state == RegState::UNMODIFIED) {
            asm_load(r, s);
            rdesc_fill(r, s, RegState::UNMODIFIED);
            return r;
        }
    }
    
    // 4. 随机选择一个寄存器并溢出
    std::srand(std::time(nullptr));
    int random = (std::rand() % (R_NUM - R_GEN)) + R_GEN;
    asm_write_back(random);
    asm_load(random, s);
    rdesc_fill(random, s, RegState::UNMODIFIED);
    return random;
}
```

### 数组支持实现

数组支持通过精确的地址计算和内存访问指令实现：

#### 数组地址计算

```cpp
case TAC_OP::ADDR:
    // 计算变量地址：a = &b
    {
        // 确保变量值已写回内存
        for (int i = R_GEN; i < R_NUM; i++) {
            if (reg_desc[i].var == tac->b && reg_desc[i].state == RegState::MODIFIED) {
                asm_write_back(i);
                break;
            }
        }
        
        // 分配寄存器存储地址
        int r = find_free_register();
        
        if (tac->b->scope == SYM_SCOPE::LOCAL) {
            // 局部变量：BP + offset
            output << "\tLOD R" << r << ",R" << R_BP << "\n";
            if (tac->b->offset >= 0)
                output << "\tADD R" << r << "," << tac->b->offset << "\n";
            else
                output << "\tSUB R" << r << "," << (-tac->b->offset) << "\n";
        } else {
            // 全局变量：STATIC + offset
            output << "\tLOD R" << r << ",STATIC\n";
            output << "\tADD R" << r << "," << tac->b->offset << "\n";
        }
        
        rdesc_fill(r, tac->a, RegState::MODIFIED);
    }
    return;
```

#### 数组元素访问

数组元素访问通过指针解引用实现：

```cpp
case TAC_OP::LOAD_PTR:
    // a = *b : 从地址 b 加载值
    {
        int r_ptr = reg_alloc(tac->b);  // 加载指针值
        
        // 为结果分配寄存器
        int r_val = find_free_register();
        
        // 根据数据类型选择加载指令
        if (tac->a->data_type == DATA_TYPE::CHAR) {
            output << "\tLDC R" << r_val << ",(R" << r_ptr << ")\n";  // 字节加载
        } else {
            output << "\tLOD R" << r_val << ",(R" << r_ptr << ")\n";  // 字加载
        }
        rdesc_fill(r_val, tac->a, RegState::MODIFIED);
    }
    return;

case TAC_OP::STORE_PTR:
    // *a = b : 将值存储到地址 a
    {
        int r_ptr = reg_alloc(tac->a);
        int r_val = reg_alloc(tac->b);
        
        // 处理寄存器冲突
        if (r_ptr == r_val) {
            r_ptr = R_TP;  // 使用临时寄存器
            // 重新加载指针地址
            if (tac->a->scope == SYM_SCOPE::LOCAL) {
                output << "\tLOD R" << r_ptr << ",(R" << R_BP << "+" << tac->a->offset << ")\n";
            } else {
                output << "\tLOD R" << R_TP << ",STATIC\n";
                output << "\tLOD R" << r_ptr << ",(R" << R_TP << "+" << tac->a->offset << ")\n";
            }
        }
        
        // 根据数据类型选择存储指令
        if (tac->b->data_type == DATA_TYPE::CHAR)
            output << "\tSTC (R" << r_ptr << "),R" << r_val << "\n";  // 字节存储
        else
            output << "\tSTO (R" << r_ptr << "),R" << r_val << "\n";  // 字存储
        
        // 指针存储后使所有寄存器失效，强制重新加载
        for (int i = R_GEN; i < R_NUM; i++) {
            if (reg_desc[i].var && reg_desc[i].var->type == SYM_TYPE::VAR && 
                reg_desc[i].state == RegState::MODIFIED) {
                asm_write_back(i);
            }
        }
        // 清除所有寄存器描述符
        for (int i = R_GEN; i < R_NUM; i++) {
            rdesc_clear(i);
        }
    }
    return;
```

### 指针支持实现

指针运算支持包括取地址、解引用和指针算术：

#### 取地址操作

```cpp
// 取地址：&var
std::shared_ptr<EXP> TACGenerator::do_address_of(std::shared_ptr<EXP> exp) {
    auto temp = mk_tmp(DATA_TYPE::INT);
    temp->is_pointer = true;
    auto temp_decl = mk_tac(TAC_OP::VAR, temp);
    temp_decl->prev = exp->code;
    
    auto addr_tac = mk_tac(TAC_OP::ADDR, temp, exp->place);
    addr_tac->prev = temp_decl;
    
    return mk_exp(temp, addr_tac);
}
```

#### 解引用操作

```cpp
// 解引用：*ptr  
std::shared_ptr<EXP> TACGenerator::do_dereference(std::shared_ptr<EXP> exp) {
    auto temp = mk_tmp(DATA_TYPE::INT);
    auto temp_decl = mk_tac(TAC_OP::VAR, temp);
    temp_decl->prev = exp->code;
    
    auto deref_tac = mk_tac(TAC_OP::LOAD_PTR, temp, exp->place);
    deref_tac->prev = temp_decl;
    
    return mk_exp(temp, deref_tac);
}
```

### 立即数指令支持

立即数指令支持允许在指令中直接使用常量值，减少寄存器分配和内存访问：

#### 立即数二元运算

```cpp
int ObjGenerator::asm_bin(const std::string& op, std::shared_ptr<SYM> a,
                           std::shared_ptr<SYM> b, std::shared_ptr<SYM> c) {
    int reg_b = reg_alloc(b);
    
    // 关键修复：临时标记 reg_b 为 MODIFIED，防止 reg_alloc(c) 选择同一寄存器
    auto original_state = reg_desc[reg_b].state;
    reg_desc[reg_b].state = RegState::MODIFIED;

    // 立即数优化：如果 c 是常量，直接使用立即数指令
    if(c->type == SYM_TYPE::CONST_INT || c->type == SYM_TYPE::CONST_CHAR) {
        if (c->type == SYM_TYPE::CONST_INT) {
            output << "\t" << op << " R" << reg_b << "," << std::get<int>(c->value) << "\n";
        } else if (c->type == SYM_TYPE::CONST_CHAR) {
            output << "\t" << op << " R" << reg_b << "," << static_cast<int>(std::get<char>(c->value)) << "\n";
        }
        rdesc_fill(reg_b, a, RegState::MODIFIED);
        return reg_b;
    }
    
    int reg_c = reg_alloc(c);
    
    // 恢复原始状态
    reg_desc[reg_b].state = original_state;

    // 处理寄存器冲突
    if (reg_b == reg_c) {
        output << "\tLOD R" << R_TP << ",R" << reg_c << "\n";
        reg_c = R_TP;
    }

    output << "\t" << op << " R" << reg_b << ",R" << reg_c << "\n";
    rdesc_fill(reg_b, a, RegState::MODIFIED);

    return reg_b;
}
```

#### 立即数加载

```cpp
void ObjGenerator::asm_load(int r, std::shared_ptr<SYM> s) {
    // 检查是否已在寄存器中
    for (int i = R_GEN; i < R_NUM; i++) {
        if (reg_desc[i].var == s) {
            output << "\tLOD R" << r << ",R" << i << "\n";
            return;
        }
    }

    // 根据符号类型生成加载指令
    switch (s->type) {
    case SYM_TYPE::CONST_INT:
        if (std::holds_alternative<int>(s->value)) {
            output << "\tLOD R" << r << "," << std::get<int>(s->value) << "\n";  // 立即数加载
        }
        break;

    case SYM_TYPE::CONST_CHAR:
        if (std::holds_alternative<char>(s->value)) {
            output << "\tLOD R" << r << "," << static_cast<int>(std::get<char>(s->value)) << "\n";  // 立即数加载
        }
        break;

    case SYM_TYPE::VAR:
        if (s->scope == SYM_SCOPE::LOCAL) {
            // 局部变量：基于 BP 的地址
            if (s->offset >= 0) {
                output << "\tLOD R" << r << ",(R" << R_BP << "+" << s->offset << ")\n";
            } else {
                output << "\tLOD R" << r << ",(R" << R_BP << s->offset << ")\n";
            }
        } else {
            // 全局变量：基于 STATIC 的地址
            output << "\tLOD R" << R_TP << ",STATIC\n";
            output << "\tLOD R" << r << ",(R" << R_TP << "+" << s->offset << ")\n";
        }
        break;

    case SYM_TYPE::TEXT:
        output << "\tLOD R" << r << ",L" << s->label << "\n";  // 字符串地址加载
        break;

    default:
        error("Cannot load symbol type: " + s->to_string());
        break;
    }
}
```

### 内存布局设计

编译器采用标准栈帧布局，支持局部变量、参数和返回地址：

```cpp
// 栈帧布局常量
constexpr int FORMAL_OFF = -4;   // 第一个形参
constexpr int OBP_OFF = 0;       // 动态链（旧 BP）
constexpr int RET_OFF = 4;       // 返回地址  
constexpr int LOCAL_OFF = 8;     // 局部变量起始位置
```

#### 函数调用支持

```cpp
void ObjGenerator::asm_call(std::shared_ptr<SYM> ret, std::shared_ptr<SYM> func) {
    asm_write_back_all();
    asm_clear_all_regs();

    // 存储旧 BP
    output << "\tSTO (R" << R_BP << "+" << (tof + oon) << "),R" << R_BP << "\n";
    oon += 4;

    // 存储返回地址
    output << "\tLOD R4,R1+32\n";  // 4*8=32
    output << "\tSTO (R" << R_BP << "+" << (tof + oon) << "),R4\n";
    oon += 4;

    // 加载新 BP
    output << "\tLOD R" << R_BP << ",R" << R_BP << "+" << (tof + oon - 8) << "\n";

    // 跳转到函数
    output << "\tJMP " << func->name << "\n";

    // 处理返回值
    if (ret != nullptr) {
        int r = reg_alloc(ret);
        output << "\tLOD R" << r << ",R" << R_TP << "\n";
        reg_desc[r].state = RegState::MODIFIED;
    }

    oon = 0;
}
``` 

## 编译并测试ccpl
### 环境准备
在Ubuntu 24.04 LTS WSL2上测试. 需要准备以下依赖:
1. build-essential
2. meson
3. yacc & bison

### 编译ccpl、汇编器和虚拟机
1. 使用meson和ninja编译ccpl:
   ```bash
   cd ccpl
   meson setup build
   ninja -C build
   ```
   如果使用vscode，可以直接从命令面板运行构建任务.(需要安装meson插件)
2. 编译汇编器和虚拟机:
    ```bash
    cd asm-machine
    make
    ```

### 编译并运行一个ccpl源文件:
```bash
./ccpl/build/ccpl path/to/source.m output.s; 
./asm-machine/build/asm output.s; 
./asm-machine/build/machine output.o
```
或者测试`ccpl/test/`下的用例：
```bash
make test=xxx
```
