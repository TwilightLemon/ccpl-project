#include "parser.tab.hh"
#include "global.hh"
#include "modules/obj.hh"
#include <iostream>
#include <fstream>

twlm::ccpl::modules::ASTBuilder ast_builder;

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file> [output_file]" << std::endl;
        return 1;
    }

    FILE *input_file = fopen(argv[1], "r");
    if (!input_file)
    {
        std::cerr << "Error: Cannot open file " << argv[1] << std::endl;
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

        // twlm::ccpl::modules::TACOptimizer opt(tac_gen.get_tac_first());
        // opt.optimize();
        // tac_gen.print_tac();

        // Generate assembly code
        std::clog << "=== Assembly Code Generation ===" << std::endl;
        
        std::ostream* asm_output;
        std::ofstream asm_file;
        
        if (argc == 3)
        {
            // Output to file
            asm_file.open(argv[2]);
            if (!asm_file.is_open())
            {
                std::cerr << "Error: Cannot open output file " << argv[2] << std::endl;
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
            std::clog << "Assembly code written to " << argv[2] << std::endl;
        }

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
