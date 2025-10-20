#include "parser.tab.hh"
#include "tac.hh"
#include <iostream>

extern yy::parser::symbol_type yylex();

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }
    
    FILE* input_file = fopen(argv[1], "r");
    if (!input_file) {
        std::cerr << "Error: Cannot open file " << argv[1] << std::endl;
        return 1;
    }
    
    extern FILE* yyin;
    yyin = input_file;
    
    // Initialize TAC generator
    tac_gen.init();
    
    try {
        yy::parser parser;
        int result = parser.parse();
        fclose(input_file);
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        fclose(input_file);
        return 1;
    }
}