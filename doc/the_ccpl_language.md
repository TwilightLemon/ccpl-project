## The ccpl Language
### Overview
ccpl is a C-style statically-typed programming language with support for basic data types, pointers, arrays, structures, and control flow statements.

### Convention
1. All variables are stored in the stack segment!!! Because there is not a memory management module (like `malloc`) yet.
2. Only basic type is supported in function parameters and return values (array and pointer types are supported).
3. There is no compile-time type checking for function calls yet.

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

Structures are user-defined composite data types. Structures can contain:
- Basic types
- Arrays
- Other structures
- Nested arrays and structures
For example, defining a structure:
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
Pointers to structures use the `->` operator:
```c
struct Student *pS=&s;
pS->id = 101;
pS->score = 98;
```

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