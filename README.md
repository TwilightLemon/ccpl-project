# ccpl-project (Experimental)
A simple C-style programming language compiler written in C++ with yacc and bison using custom assembly code and virtual machine.

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

## The ccpl Language

### Overview
ccpl is a C-style statically-typed programming language with support for basic data types, pointers, arrays, structures, and control flow statements.

### Data Types

#### Basic Types
- `int` - Integer type
- `char` - Character type

For simplicity, all basic types are treated as 4-byte entities.

#### Composite Types
- **Pointers**: Declared with `*` operator (e.g., `int *ptr`, `char *p`)
- **Arrays**: Declared with `[]` operator (e.g., `int arr[10]`, `char str[20]`)
- **Structures**: User-defined composite types declared with `struct` keyword

### Variables

#### Declaration
Variables must be declared before use with type specification:
```c
int i, j, k;
char c;
int *ptr;
char arr[100];
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

### Literals

#### Integer Literals
Decimal integers: `0`, `123`, `-456`

#### Character Literals
Single characters enclosed in single quotes: `'a'`, `'A'`, `'0'`

#### String Literals
Sequences of characters enclosed in double quotes: `"Hello"`, `"World\n"`

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
struct Student {
    int id;
    int score;
};

int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

main() {
    struct Student s;
    int result;
    
    input s.id;
    input s.score;
    
    result = max(s.id, s.score);
    output "Maximum: ";
    output result;
    output "\n";
    
    return 0;
}
```
## Convention
...