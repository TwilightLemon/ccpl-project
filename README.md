# ccpl-project (Experimental)
A simple C-style programming language compiler written in C++ with yacc and bison using custom assembly code and virtual machine.
> This project is for educational purposes only and is not intended for production use.  
> In a word, this is a coursework of twlm🐱.

## Environment Setup
Tested on Ubuntu 24.04 LTS WSL2.  
1. build-essential
2. meson
3. yacc & bison

## Run and Test ccpl
1. Build the ccpl project using Meson and Ninja:
   ```bash
   cd ccpl
   meson setup build
   ninja -C build
   ```
   or you can run vscode commands declared in `.vscode/tasks.json`.
2. Build the assembler and virtual machine:
    ```bash
    cd asm-machine
    make
    ```
3. Compile and run a ccpl source file:
    ```bash
    ./ccpl/build/ccpl path/to/source.m output.s; 
	./asm-machine/build/asm output.s; 
	./asm-machine/build/machine output.o
    ```
    or test a example in `ccpl/test/`:
    ```bash
    make test=xxx
    ```
    where `xxx` is the name of the test file without extension.

## Convention
1. All variables are stored in the stack segment!!! Because there is not a memory management module (like `malloc`) yet.
2. Only basic type is supported in function parameters and return values (array and pointer types are supported).
3. There is no compile-time type checking for function calls yet.

## The ccpl Language

### Overview
ccpl is a C-style statically-typed programming language with support for basic data types, pointers, arrays, structures, and control flow statements.

### Data Types

#### Basic Types
- `int` - Integer type
- `char` - Character type

> For simplicity, all basic types are treated as 4-byte entities.

#### Composite Types
- **Pointers**: Declared with `*` operator (e.g., `int *ptr`, `char *p`)
- **Arrays**: Declared with `[]` operator (e.g., `int arr[10]`, `char str[20]`)
- **Structures**: User-defined composite types declared with `struct` keyword

### Variables

#### Declaration
Variables must be declared before use with type specification:
```c
int i, j, *k;
char c;
int *ptr;
char arr[100];
struct A a1, *pA;
```

Basic types and arrays can be initialized at declaration:
```c 
int x = 10, y = 20;
struct A a1, *pA=&a1;
int arr[3] = {1, 2, 3};
```
Note that you may not initialize struct variables and string literals at declaration.
```c
char str[5] = "Hello"; // Invalid
struct A a1 = {1, 'a'}; // Invalid
```

#### Scope
- **Global variables**: Declared outside any function
- **Local variables**: Declared inside functions or blocks

### Operators

#### Arithmetic Operators
- `+` - Addition
- `-` - Subtraction
- `*` - Multiplication
- `/` - Division
- `-` - Unary negation

#### Comparison Operators
- `==` - Equal to
- `!=` - Not equal to
- `<` - Less than
- `<=` - Less than or equal to
- `>` - Greater than
- `>=` - Greater than or equal to

#### Assignment Operator
- `=` - Assignment

#### Pointer Operators
- `&` - Address-of operator
- `*` - Dereference operator

#### Member Access Operators
- `.` - Structure member access
- `[]` - Array subscript
- `->` - Pointer to structure member access

### Literals

#### Integer Literals
Decimal integers: `0`, `123`, `-456`

#### Character Literals
Single characters enclosed in single quotes: `'a'`, `'A'`, `'0'`

#### String Literals
Sequences of characters enclosed in double quotes: `"Hello"`, `"World\n"`

#### Initializer Lists
- Curly braces `{}` used to initialize arrays and structures
- Example: `int arr[3] = {1, 2, 3};`

### Structures

Structures are user-defined composite data types:

```c
struct StructName {
    int field1;
    char field2[10];
    struct OtherStruct field3;
};
```

Structure members are accessed using the `.` operator:
```c
struct Student s;
s.id = 100;
s.score = 95;
```

Structures can contain:
- Basic types
- Arrays
- Other structures
- Nested arrays and structures

### Functions

#### Function Declaration
Functions are declared with return type, name, parameters, and body:

```c
int functionName(int param1, char param2) {
    // function body
    return value;
}
```

Functions with no return type default to `int`:
```c
main() {
    // entry point
    return 0;
}
```

#### Function Parameters
- Pass by value for basic types
- Pointer parameters for pass by reference: `int *ptr`

#### Function Calls
```c
result = functionName(arg1, arg2);
```

### Statements

#### Expression Statement
Any expression followed by semicolon:
```c
x = 5;
y = x + 10;
func();
```

#### Block Statement
Compound statements enclosed in braces:
```c
{
    int local_var;
    local_var = 10;
    output local_var;
}
```

#### Control Flow Statements

##### If Statement
```c
if (condition) {
    // true branch
}

if (condition) {
    // true branch
} else {
    // false branch
}
```

##### While Loop
```c
while (condition) {
    // loop body
}
```

##### For Loop
```c
for (init; condition; increment) {
    // loop body
}
```

Example:
```c
for (i = 0; i < 10; i = i + 1) {
    output i;
}
```

##### Switch Statement
```c
switch (expression) {
    case value1:
        // statements
        break;
    case value2:
        // statements
        break;
    default:
        // statements
        break;
}
```

##### Break and Continue
- `break;` - Exit from loop or switch
- `continue;` - Skip to next iteration of loop

##### Return Statement
```c
return;           // void return
return expression; // return value
```

### Input/Output

#### Input Statement
Read input into a variable:
```c
input variable;
```

#### Output Statement
Output an expression or string:
```c
output expression;
output "Hello World";
output variable;
```

### Comments
Single-line comments start with `#`:
```c
# This is a comment
int x; # Another comment
```

### Program Structure

A ccpl program consists of:
1. Optional structure declarations
2. Optional global variable declarations
3. Function declarations (including `main`)

Example:
```c
# Global variable
int globalVar;

# Structure definition
struct Point {
    int x;
    int y;
};

# Main function
main() {
    int i;
    input i;
    output i;
    return 0;
}

# Other functions
int compute(int a, int b) {
    return a + b;
}
```

### Complete Example

```c
struct BB{
    char q[2];
};
struct A{
    int id;
    char a[4][5];
    struct BB b[2];
};
int main(){
    struct A a1[2];
    int i,j,k;
    i=0;j=2;k=3;

    a1[0].a[2][3]='z';
    output a1[0].a[2][3];

    char *p;
    p=&a1[i].a[j][k];
    *p = 'q';
    output a1[i].a[2][3];
    #*p='R';
    output *p;
    #*p = 'w';
    output a1[i].a[j][k];
    return 0;
}
```
Expected Output: `zqRw`.

## The ccpl Compiler Architecture

The ccpl compiler is a multi-stage compiler that transforms ccpl source code into custom assembly code for execution on a custom virtual machine. The architecture follows a classic compiler pipeline with modern optimization capabilities.

### Compilation Pipeline Overview

The compilation process consists of five main phases:

```
Source Code (.m) → Lexical Analysis → Syntax Analysis → Abstract Syntax Tree (AST)
 → Three-Address Code (TAC) → Assembly Code (.s) → Machine Code (.o)
```

### 1. Lexical Analysis
The lexical analysis phase tokenizes the input source code into a stream of tokens. This is achieved using a lexer generated by Flex. See the lexer file:`ccpl/src/lexer.l`. Flex will recognize keywords, identifiers, literals, operators, and delimiters, producing tokens for the parser.

Simply, Flex recognizes token patterns by regular expressions and makes symbols. For example, to recognize identifiers and integer literals:
```
[_A-Za-z]([_A-Za-z]|[0-9])*  {  
	return yy::parser::make_IDENTIFIER(std::string(yytext));
}

[0-9]+	{
	return yy::parser::make_INTEGER(atoi(yytext));
}
```
### 2. Syntax Analysis (Parsing)

The syntax analysis phase builds an Abstract Syntax Tree (AST) from the token stream. This is accomplished using a parser generated by Bison (Yacc) from the grammar file: `ccpl/src/parser.y`.

#### Grammar Overview

The ccpl grammar follows a hierarchical structure that defines the language syntax:

**Program Structure:**
```
program → declarations
declarations → function_decl | struct_decl | global_var_decl
```

**Type System:**
```
type_spec → int | char | void | struct IDENTIFIER
declarator → pointer_declarator | direct_declarator
direct_declarator → IDENTIFIER | IDENTIFIER[INTEGER]
```

**Expressions:**
The parser handles operator precedence and associativity using Bison's precedence declarations:
```bison
%right '='
%left EQ NE LT LE GT GE
%left '+' '-'
%left '*' '/'
%right UMINUS '&' DEREF
```

This ensures that expressions like `a = b + c * d` are parsed correctly with multiplication having higher precedence than addition.

**Example:** Consider parsing this ccpl code:
```c
int compute(int x, int y) {
    return x * 2 + y;
}
```

The parser recognizes:
1. `int` as the return type (`type_spec`)
2. `compute` as the function name (`func_name`)
3. Parameter list with two `int` parameters
4. A block containing a return statement
5. The expression `x * 2 + y` with correct precedence (multiplication first)

The parser builds an AST node hierarchy: `FuncDecl` → `BlockStmt` → `ReturnStmt` → `BinaryOpExpr` (ADD) with left operand `BinaryOpExpr` (MUL) and right operand `IdentifierExpr`.

See `ccpl/src/parser.y` for expression parsing rules and `ccpl/src/abstraction/ast_nodes.hh` for AST node definitions.

### 3. Abstract Syntax Tree (AST)

The AST is a tree representation of the source code's syntactic structure. Each node represents a construct in the language. The AST implementation is in `ccpl/src/abstraction/ast_nodes.hh` and `ccpl/src/abstraction/ast_nodes.cc`.

#### AST Node Hierarchy

The AST uses a class hierarchy with `ASTNode` as the base class:

```
ASTNode (base)
├── Expression
│   ├── ConstIntExpr (e.g., 42)
│   ├── ConstCharExpr (e.g., 'a')
│   ├── StringLiteralExpr (e.g., "Hello")
│   ├── IdentifierExpr (e.g., variable name)
│   ├── BinaryOpExpr (e.g., a + b)
│   ├── UnaryOpExpr (e.g., -x)
│   ├── AssignExpr (e.g., x = 5)
│   ├── FuncCallExpr (e.g., max(a, b))
│   ├── ArrayAccessExpr (e.g., arr[i])
│   ├── MemberAccessExpr (e.g., student.name)
│   ├── AddressOfExpr (e.g., &x)
│   └── DereferenceExpr (e.g., *ptr)
├── Statement
│   ├── ExprStmt
│   ├── BlockStmt
│   ├── IfStmt
│   ├── WhileStmt
│   ├── ForStmt
│   ├── SwitchStmt
│   ├── ReturnStmt
│   ├── BreakStmt
│   ├── ContinueStmt
│   ├── InputStmt
│   └── OutputStmt
└── Declaration
    ├── VarDecl
    ├── ParamDecl
    ├── FuncDecl
    └── StructDecl
```

#### AST Builder

The `ASTBuilder` class (`ccpl/src/modules/ast_builder.hh`) provides factory methods for creating AST nodes during parsing. For example:

```cpp
// In parser.y:
expression '+' expression
{
    $$ = ast_builder.make_binary_op(TAC_OP::ADD, $1, $3);
}
```

This creates a `BinaryOpExpr` node with the ADD operation and two child expressions.

#### Type Information

Each expression node carries type information through the `expr_type` field. The type system supports:
- **Basic types**: `int`, `char`, `void`
- **Pointer types**: `int*`, `char*`
- **Array types**: `int[10]`, `char[20]`
- **Struct types**: User-defined composite types

**Example:** For the expression `arr[i].value`, where `arr` is an array of structs:
1. `ArrayAccessExpr` node accesses the array
2. `MemberAccessExpr` node accesses the struct field
3. Type information flows through the tree to determine the final type

See `ccpl/test/arr-struct.m` for a complex example with nested structs and arrays.

### 4. Three-Address Code (TAC) Generation

TAC is an intermediate representation where each instruction has at most three operands. This simplifies optimization and code generation. The TAC generation is implemented in `ccpl/src/modules/ast_to_tac.cc` and `ccpl/src/modules/tac.cc`.

#### TAC Instruction Format

Each TAC instruction follows the form:
```
result = operand1 operator operand2
```

TAC operations are defined in `ccpl/src/abstraction/tac_definitions.hh`:
- **Arithmetic**: `ADD`, `SUB`, `MUL`, `DIV`, `NEG`
- **Comparison**: `EQ`, `NE`, `LT`, `LE`, `GT`, `GE`
- **Assignment**: `COPY`
- **Control flow**: `GOTO`, `IFZ`, `LABEL`
- **Function operations**: `BEGINFUNC`, `ENDFUNC`, `CALL`, `RETURN`, `FORMAL`, `ACTUAL`
- **I/O**: `INPUT`, `OUTPUT`
- **Pointer operations**: `ADDR`, `LOAD_PTR`, `STORE_PTR`

#### Symbol Table

The `TACGenerator` maintains symbol tables for:
- **Global symbols**: Variables and functions declared at the top level
- **Local symbols**: Variables declared within function scope
- **Struct types**: Metadata for struct definitions
- **Temporary variables**: Compiler-generated temporaries (e.g., `@t1`, `@t2`)
- **Labels**: Jump targets for control flow (e.g., `_L1`, `_L2`)

See `ccpl/src/modules/tac.hh` for the symbol table implementation.

#### Translation Examples

**Example 1: Arithmetic Expression**

Source code:
```c
int result;
result = a * 2 + b;
```

Generated TAC:
```
_t1 = a * 2
_t2 = _t1 + b
result = _t2
```

Each complex expression is broken down into simple operations with temporary variables.

**Example 2: Control Flow (If Statement)**

Source code:
```c
if (x > 0) {
    output x;
} else {
    output -x;
}
```

Generated TAC:
```
_t1 = x > 0
ifz _t1 goto _L1
output x
goto _L2
_L1:
_t2 = -x
output _t2
_L2:
```

The comparison result is stored in a temporary, then tested with `ifz` (if zero, jump).

**Example 3: While Loop**

Source code (`ccpl/test/for.m`):
```c
for(i=0; i<j; i=i+1) {
    output i; 
}
```

Generated TAC:
```
i = 0               # init
_L1:                # loop start
_t1 = i < j         # condition
ifz _t1 goto _L2    # exit if false
output i            # body
i = i + 1           # increment
goto _L1            # loop back
_L2:                # loop end
```

**Example 4: Function Call**

Source code (`ccpl/test/func-int.m`):
```c
c = max(a, b);
```

Generated TAC:
```
actual a            # push first argument
actual b            # push second argument
_t1 = call max      # call function, result in _t1
c = _t1             # assign result
```

**Example 5: Array and Struct Access**

Source code (`ccpl/test/arr-struct.m`):
```c
cls[5].grp[2].stu[3].name[1] = 'b';
```

For nested array and struct accesses, the compiler calculates the memory offset:
1. Base address of `cls`
2. Add offset for `cls[5]`
3. Add offset for `grp` field
4. Add offset for `grp[2]`
5. Add offset for `stu` field
6. Add offset for `stu[3]`
7. Add offset for `name` field
8. Add offset for `name[1]`
9. Store 'b' at the calculated address

This involves complex address calculation with the `ADDR`, `ADD`, and `STORE_PTR` operations.

#### AST to TAC Translation

The `ASTToTACGenerator` class walks the AST and generates TAC instructions. Key methods:

- `generate_statement()`: Translates statement nodes
- `generate_expression()`: Translates expression nodes, returns an `EXP` structure containing:
  - `place`: The symbol holding the result
  - `code`: TAC instructions to compute the expression
  - `next`: Linked list for multiple expressions (e.g., function arguments)

**Example flow** for `x = a + b * c`:
1. Generate TAC for `b * c` → produces `_t1` and code `_t1 = b * c`
2. Generate TAC for `a + _t1` → produces `_t2` and code `_t2 = a + _t1`
3. Generate TAC for assignment → produces code `x = _t2`
4. Join all TAC code sequences into a linked list

See `ccpl/src/modules/ast_to_tac.cc` for the complete implementation.

> More detailed info, like metadata tables and offset calculation for a complex array/struct member access, is to be continued.

### 5. Optimization (Optional)

The compiler includes an optional optimization phase implemented in `ccpl/src/modules/opt.cc`. The optimizer performs at the TAC level.

> This part is not completed yet.

#### Basic Block Construction

The optimizer first divides the TAC into basic blocks using `BlockBuilder` (`ccpl/src/modules/block.cc`). A basic block is a sequence of instructions with:
- One entry point (first instruction)
- One exit point (last instruction)
- No branches except at the end

Leaders (start of basic blocks) are identified:
1. First instruction of the program
2. Target of any jump (label)
3. Instruction following a jump

#### Control Flow Graph (CFG)

After identifying basic blocks, the optimizer builds a CFG where:
- Nodes represent basic blocks
- Edges represent control flow between blocks

This allows analysis of program structure for optimization.

#### Optimization Techniques

**1. Constant Folding**
Evaluates constant expressions at compile time:
```
Before: @t1 = 2 * 3
After:  @t1 = 6
```

**2. Constant Propagation**
Replaces variables with their constant values:
```
Before: x = 5
        y = x + 3
After:  x = 5
        y = 8
```

**3. Copy Propagation**
Eliminates unnecessary copies:
```
Before: x = y
        z = x + 1
After:  x = y
        z = y + 1
```

**4. Dead Code Elimination**
Removes code that doesn't affect program output:
```
Before: x = 5     # x is never used
        y = 10
        output y
After:  y = 10
        output y
```

Note: The optimization phase is currently commented out in `main.cc` (lines 41-44) but can be enabled for educational purposes.

### 6. Assembly Code Generation

The final phase translates TAC into custom assembly code for the ccpl virtual machine. This is implemented in `ccpl/src/modules/obj.cc`.

#### Target Machine Architecture

The virtual machine has:
- **16 registers** (`R0` to `R15`):
  - `R0` (FLAG): Stores comparison results
  - `R1` (IP): Instruction pointer
  - `R2` (BP): Base pointer (frame pointer)
  - `R3` (JP): Jump register
  - `R4` (TP): Temporary pointer
  - `R5-R14`: General-purpose registers
  - `R15` (IO): I/O register for input/output operations
- **Memory**: 64KB (256×256 bytes)
- **Stack-based**: All variables are allocated on the stack

#### Instruction Set

The custom instruction set is defined in `asm-machine/inst.h`:

**Arithmetic:**
- `ADD_0 rx, constant`: `reg[rx] = reg[rx] + constant`
- `ADD_1 rx, ry`: `reg[rx] = reg[rx] + reg[ry]`
- `SUB_0`, `SUB_1`, `MUL_0`, `MUL_1`, `DIV_0`, `DIV_1`: Similar patterns

**Memory Access:**
- `LOD_0 rx, constant`: Load from memory address (BP + constant)
- `LOD_1 rx, ry`: Load from memory address in register
- `STO_0 rx, constant`: Store to memory address (BP + constant)
- `STO_1 rx, ry`: Store to memory address in register

**Control Flow:**
- `JMP_0 constant`: Jump to address
- `JMP_1 rx`: Jump to address in register
- `JEZ_0 constant`: Jump if FLAG == 0
- `JLZ_0 constant`: Jump if FLAG < 0
- `JGZ_0 constant`: Jump if FLAG > 0
- `TST_0 rx`: Set FLAG based on register value

**I/O:**
- `ITC`: Input character to R15
- `ITI`: Input integer to R15
- `OTC`: Output character from R15
- `OTI`: Output integer from R15
- `OTS`: Output string pointed to by R15

**Other:**
- `NOP`: No operation
- `END`: Halt execution

See `asm-machine/inst.h` for complete instruction definitions.

#### Register Allocation
> TO BE IMPROVED.  

The `ObjGenerator` uses a simple register allocation strategy:

1. **Register Descriptors**: Track which variable is in which register and whether it's modified
2. **Allocation**: When a register is needed, allocate one and load the variable
3. **Spilling**: If all registers are in use, write back a modified register to memory
4. **Write-back**: Modified registers are written back to memory before function calls, jumps, or when needed

#### Assembly Generation Process

**1. Program Structure:**
For example:
```c
main()
{
	int a;
	input a;
	output a+1;
	return 0;
}
```
generated asm code:
```assembly
	# Program Header
	LOD R2,STACK
	STO (R2),0
	LOD R4,EXIT
	STO (R2+4),R4

	# label main
main:

	# begin

	# var a : int

	# input a
	LOD R5,(R2+8)
	ITI
	LOD R5,R15

	# var @t0 : int

	# @t0 = a + 1
	STO (R2+8),R5
	LOD R6,1
	ADD R5,R6

	# output @t0
	STO (R2+12),R5
	LOD R15,R5
	OTI

	# return 0
	LOD R4,0
	LOD R3,(R2+4)
	LOD R2,(R2)
	JMP R3

	# end
	LOD R3,(R2+4)
	LOD R2,(R2)
	JMP R3

	# Program Exit
EXIT:
	END
STATIC:
	DBN 0,0
STACK:

```

**2. Function Prologue:**
```assembly
_func_name:
    # Allocate stack frame
    # Save old BP
    # Initialize local variables
```

**3. Function Epilogue:**
```assembly
    # Restore BP
    # Return
```

**4. TAC to Assembly Translation:**

Each TAC operation is translated to one or more assembly instructions:

| TAC | Assembly |
|-----|----------|
| `a = b + c` | `LOD r1, b; LOD r2, c; ADD_1 r1, r2; STO r1, a` |
| `ifz x goto L` | `LOD r1, x; TST_0 r1; JEZ_0 L` |
| `output x` | `LOD r15, x; OTI` |
| `input x` | `ITI; STO r15, x` |

**Example Translation:**

For the TAC:
```
_t1 = a + b
c = _t1
```

The assembly code generated:
```assembly
LOD_0 r5, [offset_of_a]    # Load 'a' into r5
LOD_0 r6, [offset_of_b]    # Load 'b' into r6
ADD_1 r5, r6               # r5 = r5 + r6
STO_0 r5, [offset_of_c]    # Store r5 to 'c'
```

#### Memory Layout

**Stack Frame Layout:**
```
High Address
+------------------+
| Actual Params    |  (BP - 8, BP - 12, ...)
+------------------+
| Old BP           |  (BP + 0)
+------------------+
| Return Address   |  (BP + 4)
+------------------+
| Local Vars       |  (BP + 8, BP + 12, ...)
+------------------+
Low Address
```

All variables (including arrays and structs) are allocated on the stack. The compiler calculates offsets from BP for each variable.

**Example:** Function with parameters and locals:
```c
int func(int x, int y) {
    int a, b;
    a = x + y;
    return a;
}
```

Memory layout:
- `y` at BP - 8
- `x` at BP - 4
- Old BP at BP + 0
- Return address at BP + 4
- `a` at BP + 8
- `b` at BP + 12

See `ccpl/src/modules/obj.cc` for the complete implementation.

### 7. Assembler and Virtual Machine

The final step is to assemble the assembly code and run it on the virtual machine.

#### Assembler

The assembler (`asm-machine/asm.l` and `asm-machine/asm.y`) translates assembly code into machine code:

1. **First Pass**: Build symbol table (labels and their addresses)
2. **Second Pass**: Generate binary machine code

The output is a binary file (`.o`) containing the machine code.

**Instruction Format:**
Each instruction is 8 bytes:
```
[opcode: 2 bytes][rx: 1 byte][ry: 1 byte][constant: 4 bytes]
```

Example:
```assembly
ADD_0 5, 10
```
Encoded as: `[0x0030][5][0][10]`

#### Virtual Machine

The virtual machine (`asm-machine/machine.c`) executes the machine code:

**Execution Loop:**
```c
for(;;) {
    cycle++;
    instruction(reg[R_IP]);  // Fetch instruction
    
    switch(op) {             // Decode and execute
        case I_ADD_0:
            reg[rx] = reg[rx] + constant;
            break;
        // ... other instructions
    }
    
    reg[R_IP] += 8;          // Advance IP
}
```

**Features:**
- Instruction-by-instruction execution
- Performance counters: clock cycles, memory accesses, mul/div operations
- Register dump on completion


to be continued...