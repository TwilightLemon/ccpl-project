#include "parser.tab.hh"
#include "global.hh"
#include "modules/obj.hh"
#include <iostream>
#include <fstream>
#include <cstring>

twlm::ccpl::modules::ASTBuilder ast_builder;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " [-o] <input_file> [output_file]" << std::endl;
        std::cerr << "  -o: Enable TAC optimization" << std::endl;
        return 1;
    }

    bool enable_optimization = false;
    int arg_index = 1;
    
    // Check for -o flag
    if (argc > 1 && strcmp(argv[1], "-o") == 0)
    {
        enable_optimization = true;
        arg_index = 2;
    }
    
    if (arg_index >= argc)
    {
        std::cerr << "Error: No input file specified" << std::endl;
        std::cerr << "Usage: " << argv[0] << " [-o] <input_file> [output_file]" << std::endl;
        return 1;
    }
    
    const char *logo ="                     ___      \n                    /\\_ \\     \n  ___    ___   _____\\//\\ \\    \n /'___\\ /'___\\/\\ '__`\\\\ \\ \\   \n/\\ \\__//\\ \\__/\\ \\ \\L\\ \\\\_\\ \\_ \n\\ \\____\\ \\____\\\\ \\ ,__//\\____\\\n \\/____/\\/____/ \\ \\ \\/ \\/____/\n                 \\ \\_\\        \n                  \\/_/        \n";
    std::clog<< logo<<"    v0.1 powered by twlm"<<std::endl<<std::endl;

    FILE *input_file = fopen(argv[arg_index], "r");
    if (!input_file)
    {
        std::cerr << "Error: Cannot open file " << argv[arg_index] << std::endl;
        return 1;
    }

    extern FILE *yyin;
    yyin = input_file;

    ast_builder.init();

    try
    {
        yy::parser parser;
        int result = parser.parse();
        fclose(input_file);
        
        if (result != 0) {
            std::cerr << "Parsing failed" << std::endl;
            return result;
        }

        auto program = ast_builder.get_program();
        
        std::clog << "=== AST ===" << std::endl;
        std::clog << program->to_string() << std::endl;
        std::clog << std::endl;

        twlm::ccpl::modules::ASTToTACGenerator tac_generator;
        tac_generator.generate(program);
        
        auto& tac_gen = tac_generator.get_tac_generator();
        tac_gen.print_symbol_table();

        std::clog << "=== Original TAC ===" << std::endl;
        tac_gen.print_tac(std::clog);
        std::clog << std::endl;

        //multi-function optimization is not supported yet
        const auto & sym=tac_gen.get_global_symbols();
        int func_count=0;
        for(const auto& [name, sym_ptr]:sym){
            if(sym_ptr->type==SYM_TYPE::FUNC){
                func_count++;
            }
            if(func_count>1){
                enable_optimization=false;
                break;
            }
        }

        if (enable_optimization)
        {
            twlm::ccpl::modules::TACOptimizer opt(tac_gen.get_tac_first());
            opt.optimize();
            std::clog << "=== Optimized TAC ===" << std::endl;
            tac_gen.print_tac(std::clog);
            std::clog << std::endl;
        }

        // Generate assembly code
        std::clog << "=== Assembly Code Generation ===" << std::endl;
        
        std::ostream* asm_output;
        std::ofstream asm_file;
        
        // Check if output file is specified (last argument)
        if (arg_index + 1 < argc)
        {
            // Output to file
            asm_file.open(argv[arg_index + 1]);
            if (!asm_file.is_open())
            {
                std::cerr << "Error: Cannot open output file " << argv[arg_index + 1] << std::endl;
                return 1;
            }
            asm_output = &asm_file;
        }
        else
        {
            // Output to stdout
            asm_output = &std::cout;
        }
        
        twlm::ccpl::modules::ObjGenerator obj_gen(*asm_output, tac_gen);
        obj_gen.generate();
        
        if (asm_file.is_open())
        {
            asm_file.close();
            std::clog << "Assembly code written to " << argv[arg_index + 1] << std::endl;
        }
        std::clog<<"ccpl tasks completed successfully."<<std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        if(input_file)
            fclose(input_file);
        return 1;
    }
}
