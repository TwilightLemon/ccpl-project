#include "obj.hh"
#include <string>
#include <memory>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <stdexcept>

using namespace twlm::ccpl::modules;
using namespace twlm::ccpl::abstraction;

ObjGenerator::ObjGenerator(std::ostream& out, TACGenerator& tac_generator)
    : output(out), tac_gen(tac_generator), tos(0), tof(0), oof(0), oon(0),block_builder(tac_generator.get_tac_first())
{
    // Initialize register descriptors
    for (int i = 0; i < R_NUM; i++)
    {
        rdesc_clear(i);
    }
    // Build basic blocks and dataflow analysis
    block_builder.build();
    block_builder.compute_data_flow();
}

void ObjGenerator::rdesc_clear(int r)
{
    reg_desc[r].var = nullptr;
    reg_desc[r].state = RegState::UNMODIFIED;
}

void ObjGenerator::rdesc_fill(int r, std::shared_ptr<SYM> s, RegState state)
{
    // Clear any other register that holds this variable
    for (int i = R_GEN; i < R_NUM; i++)
    {
        if (reg_desc[i].var == s)
        {
            rdesc_clear(i);
        }
    }

    reg_desc[r].var = s;
    reg_desc[r].state = state;
}

void ObjGenerator::asm_write_back(int r)
{
    if (reg_desc[r].var != nullptr && reg_desc[r].state == RegState::MODIFIED)
    {
        auto var = reg_desc[r].var;
        
        if (var->scope == SYM_SCOPE::LOCAL)
        {
            // Local variable
            if (var->offset >= 0)
            {
                output << "\tSTO (R" << R_BP << "+" << var->offset << "),R" << r << "\n";
            }
            else
            {
                output << "\tSTO (R" << R_BP << var->offset << "),R" << r << "\n";
            }
        }
        else
        {
            // Global variable
            output << "\tLOD R" << R_TP << ",STATIC\n";
            output << "\tSTO (R" << R_TP << "+" << var->offset << "),R" << r << "\n";
        }
        
        reg_desc[r].state = RegState::UNMODIFIED;
    }
}

void ObjGenerator::asm_write_back_all()
{
    for (int r = R_GEN; r < R_NUM; r++)
    {
        asm_write_back(r);
    }
}

void ObjGenerator::asm_clear_all_regs()
{
    for (int r = R_GEN; r < R_NUM; r++)
    {
        rdesc_clear(r);
    }
}

void ObjGenerator::asm_load(int r, std::shared_ptr<SYM> s)
{
    // Check if already in a register
    for (int i = R_GEN; i < R_NUM; i++)
    {
        if (reg_desc[i].var == s)
        {
            // Load from the register
            output << "\tLOD R" << r << ",R" << i << "\n";
            return;
        }
    }

    // Not in a register, load based on symbol type
    switch (s->type)
    {
    case SYM_TYPE::CONST_INT:
        if (std::holds_alternative<int>(s->value))
        {
            output << "\tLOD R" << r << "," << std::get<int>(s->value) << "\n";
        }
        break;

    case SYM_TYPE::CONST_CHAR:
        if (std::holds_alternative<char>(s->value))
        {
            output << "\tLOD R" << r << "," << static_cast<int>(std::get<char>(s->value)) << "\n";
        }
        break;

    case SYM_TYPE::VAR:
        if (s->scope == SYM_SCOPE::LOCAL)
        {
            // Local variable
            if (s->offset >= 0)
            {
                output << "\tLOD R" << r << ",(R" << R_BP << "+" << s->offset << ")\n";
            }
            else
            {
                output << "\tLOD R" << r << ",(R" << R_BP << s->offset << ")\n";
            }
        }
        else
        {
            // Global variable
            output << "\tLOD R" << R_TP << ",STATIC\n";
            output << "\tLOD R" << r << ",(R" << R_TP << "+" << s->offset << ")\n";
        }
        break;

    case SYM_TYPE::TEXT:
        output << "\tLOD R" << r << ",L" << s->label << "\n";
        break;

    default:
        error("Cannot load symbol type: " + s->to_string());
        break;
    }
}

int ObjGenerator::reg_alloc(std::shared_ptr<SYM> s)
{
    // Check if already in a register
    for (int r = R_GEN; r < R_NUM; r++)
    {
        if (reg_desc[r].var == s)
        {
            if (reg_desc[r].state == RegState::MODIFIED)
            {
                asm_write_back(r);
            }
            return r;
        }
    }

    // Find an empty register
    for (int r = R_GEN; r < R_NUM; r++)
    {
        if (reg_desc[r].var == nullptr)
        {
            asm_load(r, s);
            rdesc_fill(r, s, RegState::UNMODIFIED);
            return r;
        }
    }

    // Find an unmodified register
    for (int r = R_GEN; r < R_NUM; r++)
    {
        if (reg_desc[r].state == RegState::UNMODIFIED)
        {
            asm_load(r, s);
            rdesc_fill(r, s, RegState::UNMODIFIED);
            return r;
        }
    }

    // Pick a random register and spill it
    std::srand(std::time(nullptr));
    int random = (std::rand() % (R_NUM - R_GEN)) + R_GEN;
    asm_write_back(random);
    asm_load(random, s);
    rdesc_fill(random, s, RegState::UNMODIFIED);
    return random;
}

int ObjGenerator::asm_bin(const std::string& op, std::shared_ptr<SYM> a,
                           std::shared_ptr<SYM> b, std::shared_ptr<SYM> c)
{
    int reg_b = reg_alloc(b);
    
    // CRITICAL FIX: Mark reg_b as MODIFIED temporarily to prevent reg_alloc(c)
    // from choosing this register and overwriting the value
    auto original_state = reg_desc[reg_b].state;
    reg_desc[reg_b].state = RegState::MODIFIED;

    if(c->type ==SYM_TYPE::CONST_INT || c->type == SYM_TYPE::CONST_CHAR){
        // For immediate values, we can directly use them in the instruction
        if (c->type == SYM_TYPE::CONST_INT)
        {
            output << "\t" << op << " R" << reg_b << "," << std::get<int>(c->value) << "\n";
        }
        else if (c->type == SYM_TYPE::CONST_CHAR)
        {
            output << "\t" << op << " R" << reg_b << "," << static_cast<int>(std::get<char>(c->value)) << "\n";
        }
        rdesc_fill(reg_b, a, RegState::MODIFIED);
        return reg_b;
    }
    
    int reg_c = reg_alloc(c);
    
    // Restore original state
    reg_desc[reg_b].state = original_state;

    // If they're the same register (same variable), we need to use a temporary
    if (reg_b == reg_c)
    {
        // Load c into a temporary register
        output << "\tLOD R" << R_TP << ",R" << reg_c << "\n";
        reg_c = R_TP;
    }

    output << "\t" << op << " R" << reg_b << ",R" << reg_c << "\n";
    rdesc_fill(reg_b, a, RegState::MODIFIED);

    return reg_b;
}

void ObjGenerator::asm_cmp(TAC_OP op, std::shared_ptr<SYM> a,
                           std::shared_ptr<SYM> b, std::shared_ptr<SYM> c)
{
    int reg_b = asm_bin("SUB",a,b,c);
    output << "\tTST R" << reg_b << "\n";

    switch (op)
    {
    case TAC_OP::EQ:  // ==
        output << "\tLOD R3,R1+40\n";
        output << "\tJEZ R3\n";
        output << "\tLOD R" << reg_b << ",0\n";
        output << "\tLOD R3,R1+24\n";
        output << "\tJMP R3\n";
        output << "\tLOD R" << reg_b << ",1\n";
        break;

    case TAC_OP::NE:  // !=
        output << "\tLOD R3,R1+40\n";
        output << "\tJEZ R3\n";
        output << "\tLOD R" << reg_b << ",1\n";
        output << "\tLOD R3,R1+24\n";
        output << "\tJMP R3\n";
        output << "\tLOD R" << reg_b << ",0\n";
        break;

    case TAC_OP::LT:  // <
        output << "\tLOD R3,R1+40\n";
        output << "\tJLZ R3\n";
        output << "\tLOD R" << reg_b << ",0\n";
        output << "\tLOD R3,R1+24\n";
        output << "\tJMP R3\n";
        output << "\tLOD R" << reg_b << ",1\n";
        break;

    case TAC_OP::LE:  // <=
        output << "\tLOD R3,R1+40\n";
        output << "\tJGZ R3\n";
        output << "\tLOD R" << reg_b << ",1\n";
        output << "\tLOD R3,R1+24\n";
        output << "\tJMP R3\n";
        output << "\tLOD R" << reg_b << ",0\n";
        break;

    case TAC_OP::GT:  // >
        output << "\tLOD R3,R1+40\n";
        output << "\tJGZ R3\n";
        output << "\tLOD R" << reg_b << ",0\n";
        output << "\tLOD R3,R1+24\n";
        output << "\tJMP R3\n";
        output << "\tLOD R" << reg_b << ",1\n";
        break;

    case TAC_OP::GE:  // >=
        output << "\tLOD R3,R1+40\n";
        output << "\tJLZ R3\n";
        output << "\tLOD R" << reg_b << ",1\n";
        output << "\tLOD R3,R1+24\n";
        output << "\tJMP R3\n";
        output << "\tLOD R" << reg_b << ",0\n";
        break;

    default:
        error("Unknown comparison operator");
        break;
    }

    // Update descriptor
    rdesc_clear(reg_b);
    rdesc_fill(reg_b, a, RegState::MODIFIED);
}

void ObjGenerator::asm_cond(const std::string& op, std::shared_ptr<SYM> a,
                            const std::string& label)
{
    asm_write_back_all();

    if (a != nullptr)
    {
        int r = -1;

        // Check if in register
        for (int i = R_GEN; i < R_NUM; i++)
        {
            if (reg_desc[i].var == a)
            {
                r = i;
                break;
            }
        }

        if (r >= R_GEN)
        {
            output << "\tTST R" << r << "\n";
        }
        else
        {
            r = reg_alloc(a);
            output << "\tTST R" << r << "\n";
        }
    }

    output << "\t" << op << " " << label << "\n";
}

void ObjGenerator::asm_call(std::shared_ptr<SYM> ret, std::shared_ptr<SYM> func)
{
    asm_write_back_all();
    asm_clear_all_regs();

    // Store old BP
    output << "\tSTO (R" << R_BP << "+" << (tof + oon) << "),R" << R_BP << "\n";
    oon += 4;

    // Store return address
    output << "\tLOD R4,R1+32\n";  // 4*8=32
    output << "\tSTO (R" << R_BP << "+" << (tof + oon) << "),R4\n";
    oon += 4;

    // Load new BP
    output << "\tLOD R" << R_BP << ",R" << R_BP << "+" << (tof + oon - 8) << "\n";

    // Jump to function
    output << "\tJMP " << func->name << "\n";

    // Handle return value
    if (ret != nullptr)
    {
        int r = reg_alloc(ret);
        output << "\tLOD R" << r << ",R" << R_TP << "\n";
        reg_desc[r].state = RegState::MODIFIED;
    }

    oon = 0;
}

void ObjGenerator::asm_return(std::shared_ptr<SYM> ret_val)
{
    asm_write_back_all();
    asm_clear_all_regs();

    if (ret_val != nullptr)
    {
        // Load return value into R_TP
        asm_load(R_TP, ret_val);
    }

    // Load return address
    output << "\tLOD R3,(R" << R_BP << "+4)\n";
    // Restore BP
    output << "\tLOD R" << R_BP << ",(R" << R_BP << ")\n";
    // Jump to return address
    output << "\tJMP R3\n";
}

void ObjGenerator::asm_head()
{
    output << "\tLOD R" << R_BP << ",STACK\n";
    output << "\tSTO (R" << R_BP << "),0\n";
    output << "\tLOD R4,EXIT\n";
    output << "\tSTO (R" << R_BP << "+4),R4\n";
}

void ObjGenerator::asm_tail()
{
    output << "EXIT:\n";
    output << "\tEND\n";
}

void ObjGenerator::asm_str(std::shared_ptr<SYM> s)
{
    if (!std::holds_alternative<std::string>(s->value))
    {
        return;
    }

    std::string text = std::get<std::string>(s->value);
    output << "L" << s->label << ":\n";
    output << "\tDBS ";

    bool first = true;
    // Process string literal - need to handle escape sequences
    // The string includes quotes, so skip them
    size_t start = 0, end = text.length();
    if (text.length() >= 2 && text[0] == '"') {
        start = 1;
        end = text.length() - 1;
    }
    
    for (size_t i = start; i < end; i++)
    {
        if (!first) output << ",";
        first = false;
        
        if (text[i] == '\\' && i + 1 < end)
        {
            i++;
            switch (text[i])
            {
            case 'n':
                output << static_cast<int>('\n');
                break;
            case 't':
                output << static_cast<int>('\t');
                break;
            case 'r':
                output << static_cast<int>('\r');
                break;
            case '\\':
                output << static_cast<int>('\\');
                break;
            case '"':
                output << static_cast<int>('"');
                break;
            case '0':
                output << "0";
                break;
            default:
                output << static_cast<int>(static_cast<unsigned char>(text[i]));
                break;
            }
        }
        else
        {
            output << static_cast<int>(static_cast<unsigned char>(text[i]));
        }
    }

    // Always end with null terminator
    if (!first) {
        output << ",0\n";
    } else {
        output << "0\n";
    }
}

void ObjGenerator::asm_static()
{
    // Get all TEXT symbols from global symbol table
    const auto& global_symbols = tac_gen.get_global_symbols();
    
    for (const auto& pair : global_symbols)
    {
        const auto& sym = pair.second;
        if (sym->type == SYM_TYPE::TEXT)
        {
            asm_str(sym);
        }
    }

    output << "STATIC:\n";
    output << "\tDBN 0," << tos << "\n";
    output << "STACK:\n";
}

void ObjGenerator::asm_code(std::shared_ptr<TAC> tac)
{
    if (!tac)
    {
        return;
    }

    int r;

    switch (tac->op)
    {
    case TAC_OP::UNDEF:
        error("Cannot translate TAC_UNDEF");
        return;

    case TAC_OP::ADD:
        asm_bin("ADD", tac->a, tac->b, tac->c);
        return;

    case TAC_OP::SUB:
        asm_bin("SUB", tac->a, tac->b, tac->c);
        return;

    case TAC_OP::MUL:
        asm_bin("MUL", tac->a, tac->b, tac->c);
        return;

    case TAC_OP::DIV:
        asm_bin("DIV", tac->a, tac->b, tac->c);
        return;

    case TAC_OP::NEG:
        {
            auto zero = std::make_shared<SYM>();
            zero->type = SYM_TYPE::CONST_INT;
            zero->value = 0;
            asm_bin("SUB", tac->a, zero, tac->b);
        }
        return;

    case TAC_OP::EQ:
    case TAC_OP::NE:
    case TAC_OP::LT:
    case TAC_OP::LE:
    case TAC_OP::GT:
    case TAC_OP::GE:
        asm_cmp(tac->op, tac->a, tac->b, tac->c);
        return;

    case TAC_OP::COPY:
        r = reg_alloc(tac->b);
        rdesc_fill(r, tac->a, RegState::MODIFIED);
        return;

    case TAC_OP::INPUT:
        r = reg_alloc(tac->a);
        if(tac->a->data_type==DATA_TYPE::CHAR)
            output << "\tITC\n";
        else if(tac->a->data_type==DATA_TYPE::INT)
            output << "\tITI\n";
        else throw std::runtime_error("Unsupported data type for INPUT");
        output << "\tLOD R" << r << ",R" << R_IO << "\n";
        reg_desc[r].state = RegState::MODIFIED;
        return;

    case TAC_OP::OUTPUT:
        r = reg_alloc(tac->a);
        output << "\tLOD R" << R_IO << ",R" << r << "\n";
        
        if (tac->a->type == SYM_TYPE::CONST_INT||
            (tac->a->type == SYM_TYPE::VAR && tac->a->data_type == DATA_TYPE::INT))
        {
            output << "\tOTI\n";
        }else  if (tac->a->type == SYM_TYPE::CONST_CHAR ||
            (tac->a->type == SYM_TYPE::VAR && tac->a->data_type == DATA_TYPE::CHAR )){
            output << "\tOTC\n";
        }
        else if (tac->a->type == SYM_TYPE::TEXT)
        {
            output << "\tOTS\n";
        }
        return;

    case TAC_OP::GOTO:
        asm_cond("JMP", nullptr, tac->a->name);
        return;

    case TAC_OP::IFZ:
        asm_cond("JEZ", tac->b, tac->a->name);
        return;

    case TAC_OP::LABEL:
        asm_write_back_all();
        asm_clear_all_regs();
        output << tac->a->name << ":\n";
        return;

    case TAC_OP::ACTUAL:
        r = reg_alloc(tac->a);
        output << "\tSTO (R" << R_BP << "+" << (tof + oon) << "),R" << r << "\n";
        oon += 4;
        return;

    case TAC_OP::CALL:
        asm_call(tac->a, tac->b);
        return;

    case TAC_OP::BEGINFUNC:
        tof = LOCAL_OFF;
        oof = FORMAL_OFF;
        oon = 0;
        return;

    case TAC_OP::FORMAL:
        tac->a->scope = SYM_SCOPE::LOCAL;
        tac->a->offset = oof;
        oof -= 4;
        return;

    case TAC_OP::VAR:
    {
        int var_size = tac->a->get_size();
        
        if (tac->a->scope == SYM_SCOPE::LOCAL)
        {
            tac->a->offset = tof;
            tof += var_size;
        }
        else
        {
            tac->a->offset = tos;
            tos += var_size;
        }
        return;
    }

    case TAC_OP::RETURN:
        asm_return(tac->a);
        return;

    case TAC_OP::ENDFUNC:
        asm_return(nullptr);
        return;

    case TAC_OP::ADDR:
        {
            for (int i = R_GEN; i < R_NUM; i++)
            {
                if (reg_desc[i].var == tac->b && reg_desc[i].state == RegState::MODIFIED)
                {
                    asm_write_back(i);
                    break;
                }
            }
            
            r = -1;
            for (int i = R_GEN; i < R_NUM; i++)
            {
                if (reg_desc[i].var == nullptr)
                {
                    r = i;
                    break;
                }
            }
            
            if (r == -1)
            {
                for (int i = R_GEN; i < R_NUM; i++)
                {
                    if (reg_desc[i].state == RegState::UNMODIFIED)
                    {
                        r = i;
                        break;
                    }
                }
                if (r == -1)
                {
                    r = R_GEN;
                    asm_write_back(r);
                }
            }
            
            if (tac->b->scope == SYM_SCOPE::LOCAL)
            {
                output << "\tLOD R" << r << ",R" << R_BP << "\n";
                if (tac->b->offset >= 0)
                    output << "\tADD R" << r << "," << tac->b->offset << "\n";
                else
                    output << "\tSUB R" << r << "," << (-tac->b->offset) << "\n";
            }
            else
            {
                output << "\tLOD R" << r << ",STATIC\n";
                output << "\tADD R" << r << "," << tac->b->offset << "\n";
            }
            
            rdesc_fill(r, tac->a, RegState::MODIFIED);
        }
        return;

    case TAC_OP::LOAD_PTR:
        // a = *b : Load value from address stored in b
        {
            int r_ptr = reg_alloc(tac->b);  // Load pointer value
            
            // Find a free register for the result (don't load tac->a, it's the result!)
            int r_val = -1;
            for (int i = R_GEN; i < R_NUM; i++) {
                if (reg_desc[i].var == nullptr) {
                    r_val = i;
                    break;
                }
            }
            
            if (r_val == -1) {
                // No free register, find an unmodified one
                for (int i = R_GEN; i < R_NUM; i++) {
                    if (reg_desc[i].state == RegState::UNMODIFIED && i != r_ptr) {
                        r_val = i;
                        rdesc_clear(r_val);
                        break;
                    }
                }
            }
            
            if (r_val == -1) {
                // All registers are modified, write back one that's not r_ptr
                for (int i = R_GEN; i < R_NUM; i++) {
                    if (i != r_ptr) {
                        r_val = i;
                        asm_write_back(r_val);
                        rdesc_clear(r_val);
                        break;
                    }
                }
            }
            
            // Load value from address in r_ptr
            if (tac->a->data_type == DATA_TYPE::CHAR) {
                output << "\tLDC R" << r_val << ",(R" << r_ptr << ")\n";
            } else {
                output << "\tLOD R" << r_val << ",(R" << r_ptr << ")\n";
            }
            rdesc_fill(r_val, tac->a, RegState::MODIFIED);
        }
        return;

    case TAC_OP::STORE_PTR:
        {
            int r_ptr = reg_alloc(tac->a);
            int r_val = reg_alloc(tac->b);
            
            if (r_ptr == r_val)
            {
                // The second reg_alloc overwrote the first register
                // Need to reload the pointer address from memory
                r_ptr = R_TP;
                if (tac->a->scope == SYM_SCOPE::LOCAL)
                {
                    output << "\tLOD R" << r_ptr << ",(R" << R_BP << "+" << tac->a->offset << ")\n";
                }
                else
                {
                    output << "\tLOD R" << R_TP << ",STATIC\n";
                    output << "\tLOD R" << r_ptr << ",(R" << R_TP << "+" << tac->a->offset << ")\n";
                }
            }
            
            if (tac->b->data_type == DATA_TYPE::CHAR)
                output << "\tSTC (R" << r_ptr << "),R" << r_val << "\n";
            else
                output << "\tSTO (R" << r_ptr << "),R" << r_val << "\n";
            
            // After pointer store, invalidate all registers since we don't know what was modified
            // Write back all modified variables first, then clear all descriptors
            for (int i = R_GEN; i < R_NUM; i++)
            {
                if (reg_desc[i].var && reg_desc[i].var->type == SYM_TYPE::VAR && 
                    reg_desc[i].state == RegState::MODIFIED)
                {
                    asm_write_back(i);
                }
            }
            // Clear all register descriptors to force reload from memory
            for (int i = R_GEN; i < R_NUM; i++)
            {
                rdesc_clear(i);
            }
        }
        return;

    default:
        error("Unknown TAC opcode: " + std::to_string(static_cast<int>(tac->op)));
        return;
    }
}

void ObjGenerator::generate()
{
    tof = LOCAL_OFF;
    oof = FORMAL_OFF;
    oon = 0;

    // Initialize register descriptors
    for (int r = 0; r < R_NUM; r++)
    {
        rdesc_clear(r);
    }

    // Generate assembly header
    asm_head();
    asm_main();

    // Translate TAC to assembly
    auto cur = tac_gen.get_tac_first();
    while (cur != nullptr)
    {
        output << "\n\t# " << cur->to_string() << "\n";
        asm_code(cur);
        cur = cur->next;
    }

    // Generate assembly tail
    asm_tail();

    // Generate static data section
    asm_static();
}

void ObjGenerator::asm_main(){
    auto cur =tac_gen.get_tac_first();
    //check if the first label of func is main
    while(cur!=nullptr){
        if(cur->op==TAC_OP::LABEL){
            if(cur->a->name=="main"){
                return;
            }else{
                break;
            }
        }
        cur=cur->next;
    }
    output << "\n\t# Jump to main\n";
    output << "\tJMP main\n";
}

void ObjGenerator::error(const std::string& msg)
{
    std::cerr << "Assembly Generation Error: " << msg << std::endl;
    throw std::runtime_error(msg);
}