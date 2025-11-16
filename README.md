# ccpl-project (Experimental)
A simple C-style programming language compiler written in C++ with yacc and bison using custom assembly code and virtual machine.
> This project is for educational purposes only and is not intended for production use.  
> In a word, this is a coursework of twlm🐱.

## Key Features
- C-style syntax
- Statically-typed language
- Basic data types: `int`, `char`
- Composite data types: pointers, arrays, structures
- Control flow statements: `if`, `else`, `while`, `for`, `switch`...
- Global and local TAC optimization
- Custom assembly language and virtual machine

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
    or test an example in `ccpl/test/`:
    ```bash
    make test=xxx
    ```
    where `xxx` is the name of the test file without extension.

## The ccpl Language
Please refer to [the_ccpl_language.md](doc/the_ccpl_language.md) for detailed documentation of the ccpl language.

## The ccpl Compiler Architecture
Please refer to [the_ccpl_architecture.md](doc/the_ccpl_architecture.md) for detailed documentation of the ccpl compiler architecture.  
Or you can refer to [Deepwiki](https://deepwiki.com/TwilightLemon/ccpl-project) for more detailed explanations and design decisions.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

