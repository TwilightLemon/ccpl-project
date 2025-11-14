#pragma once
#include <string>
#include <memory>
#include <array>
#include <iostream>
#include <fstream>
#include "tac.hh"
#include "block.hh"

namespace twlm::ccpl::modules
{
    using namespace twlm::ccpl::abstraction;

    // Register definitions
    constexpr int R_UNDEF = -1;
    constexpr int R_FLAG = 0;    // Flag register
    constexpr int R_IP = 1;      // Instruction pointer
    constexpr int R_BP = 2;      // Base pointer
    constexpr int R_JP = 3;      // Jump register
    constexpr int R_TP = 4;      // Temporary pointer
    constexpr int R_GEN = 5;     // First general purpose register
    constexpr int R_NUM = 16;    // Total number of registers
    constexpr int R_IO = 15;     // I/O register

    // Frame layout offsets
    constexpr int FORMAL_OFF = -4;   // First formal parameter
    constexpr int OBP_OFF = 0;       // Dynamic chain (old BP)
    constexpr int RET_OFF = 4;       // Return address
    constexpr int LOCAL_OFF = 8;     // Local variables start

    // Register descriptor states
    enum class RegState
    {
        UNMODIFIED = 0,
        MODIFIED = 1
    };

    // Register descriptor
    struct RegDescriptor
    {
        std::shared_ptr<SYM> var;  // Variable currently in register
        RegState state;             // Modified or not

        RegDescriptor() : var(nullptr), state(RegState::UNMODIFIED) {}
    };

    // Assembly code generator
    class ObjGenerator
    {
    private:
        std::ostream& output;
        TACGenerator& tac_gen;
        BlockBuilder block_builder;

        // Register management
        std::array<RegDescriptor, R_NUM> reg_desc;

        // Memory offsets
        int tos;  // Top of static (global variables)
        int tof;  // Top of frame (local variables)
        int oof;  // Offset of formal parameters
        int oon;  // Offset of next frame

        // Helper methods
        void rdesc_clear(int r);
        void rdesc_fill(int r, std::shared_ptr<SYM> s, RegState state);
        
        void asm_write_back(int r);
        void asm_write_back_all();
        void asm_clear_all_regs();
        
        void asm_load(int r, std::shared_ptr<SYM> s);
        int reg_alloc(std::shared_ptr<SYM> s);
        
        //return: reg_b
        int asm_bin(const std::string& op, std::shared_ptr<SYM> a, 
                     std::shared_ptr<SYM> b, std::shared_ptr<SYM> c);
        void asm_cmp(TAC_OP op, std::shared_ptr<SYM> a, 
                     std::shared_ptr<SYM> b, std::shared_ptr<SYM> c);
        void asm_cond(const std::string& op, std::shared_ptr<SYM> a, 
                      const std::string& label);
        
        void asm_call(std::shared_ptr<SYM> ret, std::shared_ptr<SYM> func);
        void asm_return(std::shared_ptr<SYM> ret_val);
        
        void asm_head();
        void asm_tail();
        void asm_static();
        void asm_main();
        void asm_str(std::shared_ptr<SYM> s);
        
        void asm_code(std::shared_ptr<TAC> tac);

    public:
        ObjGenerator(std::ostream& out, TACGenerator& tac_generator);
        
        void generate();
        
        // Error handling
        void error(const std::string& msg);
    };
}