#include "parser.tab.hh"
#include "global.hh"
#include <iostream>

twlm::ccpl::modules::ASTBuilder ast_builder;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
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
        // Step 1: Parse and build AST
        yy::parser parser;
        int result = parser.parse();
        fclose(input_file);
        
        if (result != 0) {
            std::cerr << "Parsing failed" << std::endl;
            return result;
        }

        // Step 2: Get the AST
        auto program = ast_builder.get_program();
        
        // Optional: Print AST for debugging
        std::clog << "=== AST ===" << std::endl;
        std::clog << program->to_string() << std::endl;
        std::clog << std::endl;

        // Step 3: Generate TAC from AST
        twlm::ccpl::modules::ASTToTACGenerator tac_generator;
        tac_generator.generate(program);
        
        auto& tac_gen = tac_generator.get_tac_generator();
        
        // Print symbol table
        tac_gen.print_symbol_table();

        // Log original TAC
        std::clog << "=== Original TAC ===" << std::endl;
        tac_gen.print_tac(std::clog);
        std::clog << std::endl;

        // Step 4: Optimize TAC
        twlm::ccpl::modules::TACOptimizer opt(tac_gen.get_tac_first());
        opt.optimize();

        // Step 5: Output final TAC to stdout
        tac_gen.print_tac();

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        fclose(input_file);
        return 1;
    }
}
