#include "parser.tab.hh"
#include "global.hh"
#include <iostream>

twlm::ccpl::modules::TACGenerator tac_gen;
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

    tac_gen.init();

    try
    {
        yy::parser parser;
        int result = parser.parse();
        fclose(input_file);
        tac_gen.print_symbol_table();

        //先log一个原始结果：
        std::clog << "=== Original TAC ===" << std::endl;
        tac_gen.print_tac(std::clog);
        std::clog << std::endl;

        twlm::ccpl::modules::TACOptimizer opt(tac_gen.get_tac_first());
        opt.optimize();

        //写入标准输出的是最终结果
        tac_gen.print_tac();

        return result;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        fclose(input_file);
        return 1;
    }
}
