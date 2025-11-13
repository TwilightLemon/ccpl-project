#pragma once
#include <string>
#include <variant>
#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include "tac_definitions.hh"
#include "struct_metadata.hh"
#include "array_metadata.hh"

namespace twlm::ccpl::abstraction
{
    struct SYM
    {
        SYM_TYPE type;       // Symbol type (VAR, FUNC, etc.)
        DATA_TYPE data_type; // Data type (INT, CHAR, VOID)
        SYM_SCOPE scope;
        std::string name;
        std::variant<int, char, std::string> value;
        int offset;
        int label;

        // For functions
        std::vector<DATA_TYPE> param_types;
        DATA_TYPE return_type;

        // For struct types
        std::string struct_type_name;
        std::shared_ptr<StructTypeMetadata> struct_metadata; // Complete struct type metadata (not expanded)

        // For arrays
        bool is_array;
        std::shared_ptr<ArrayMetadata> array_metadata; // Array dimension and size info

        // For pointers
        bool is_pointer;
        DATA_TYPE base_type;

        SYM() : type(SYM_TYPE::UNDEF), data_type(DATA_TYPE::UNDEF),
                scope(SYM_SCOPE::GLOBAL), offset(-1), label(-1),
                return_type(DATA_TYPE::UNDEF), is_array(false), is_pointer(false) {}
        
        //get size in byte
        int get_size() const{
            int size=4; //by default
            if(is_array && array_metadata){
                size = array_metadata->get_total_elements() * array_metadata->element_size;
            } else if(data_type == DATA_TYPE::STRUCT && struct_metadata){
                size = struct_metadata->total_size;
            }
            return size;
        }

        bool get_const_value(int &out_value) const {
            if (type == SYM_TYPE::CONST_INT && data_type == DATA_TYPE::INT) {
                out_value = std::get<int>(value);
                return true;
            } else if (type == SYM_TYPE::CONST_CHAR && data_type == DATA_TYPE::CHAR) {
                out_value = static_cast<int>(std::get<char>(value));
                return true;
            }
            return false;
        }
        
        std::string to_string() const
        {
            switch (type)
            {
            case SYM_TYPE::VAR:
            case SYM_TYPE::FUNC:
            case SYM_TYPE::LABEL:
            case SYM_TYPE::STRUCT_TYPE:
                return name;

            case SYM_TYPE::TEXT:
            {
                std::ostringstream oss;
                oss << "L" << label;
                return oss.str();
            }

            case SYM_TYPE::CONST_INT:
                if (std::holds_alternative<int>(value))
                {
                    return std::to_string(std::get<int>(value));
                }
                return name;

            case SYM_TYPE::CONST_CHAR:
                if (std::holds_alternative<char>(value))
                {
                    return "'" + std::string(1, std::get<char>(value)) + "'";
                }
                return name;

            default:
                return "?";
            }
        }
    };

    struct TAC
    {
        TAC_OP op;
        std::shared_ptr<SYM> a, b, c;
        std::shared_ptr<TAC> prev, next;

        TAC(TAC_OP op = TAC_OP::UNDEF) : op(op), prev(nullptr), next(nullptr) {}

        std::shared_ptr<SYM> get_def() const{
            switch (op)
            {
            case TAC_OP::ADD:
            case TAC_OP::SUB:
            case TAC_OP::MUL:
            case TAC_OP::DIV:
            case TAC_OP::EQ:
            case TAC_OP::NE:
            case TAC_OP::LT:
            case TAC_OP::LE:
            case TAC_OP::GT:
            case TAC_OP::GE:
            case TAC_OP::NEG:
            case TAC_OP::COPY:
            case TAC_OP::LOAD_PTR:
            case TAC_OP::ADDR:
            case TAC_OP::INPUT:
            case TAC_OP::CALL:
                return a;
            default:
                return nullptr;
            }
        }

        std::vector<std::shared_ptr<SYM>> get_uses() const{
            std::vector<std::shared_ptr<SYM>> uses;

            // b 操作数
            if (b && b->type == SYM_TYPE::VAR)
            {
                uses.push_back(b);
            }

            // c 操作数
            if (c && c->type == SYM_TYPE::VAR)
            {
                uses.push_back(c);
            }

            // 特殊指令中的 a 操作数也是使用
            if (op == TAC_OP::RETURN || op == TAC_OP::OUTPUT ||
                op == TAC_OP::IFZ || op == TAC_OP::ACTUAL ||
                op == TAC_OP::STORE_PTR)
            {
                if (a && a->type == SYM_TYPE::VAR)
                {
                    uses.push_back(a);
                }
            }

            return uses;
        }

        std::string to_string() const
        {
            std::ostringstream oss;

            switch (op)
            {
            case TAC_OP::ADD:
                oss << a->to_string() << " = " << b->to_string() << " + " << c->to_string();
                break;
            case TAC_OP::SUB:
                oss << a->to_string() << " = " << b->to_string() << " - " << c->to_string();
                break;
            case TAC_OP::MUL:
                oss << a->to_string() << " = " << b->to_string() << " * " << c->to_string();
                break;
            case TAC_OP::DIV:
                oss << a->to_string() << " = " << b->to_string() << " / " << c->to_string();
                break;
            case TAC_OP::EQ:
                oss << a->to_string() << " = (" << b->to_string() << " == " << c->to_string() << ")";
                break;
            case TAC_OP::NE:
                oss << a->to_string() << " = (" << b->to_string() << " != " << c->to_string() << ")";
                break;
            case TAC_OP::LT:
                oss << a->to_string() << " = (" << b->to_string() << " < " << c->to_string() << ")";
                break;
            case TAC_OP::LE:
                oss << a->to_string() << " = (" << b->to_string() << " <= " << c->to_string() << ")";
                break;
            case TAC_OP::GT:
                oss << a->to_string() << " = (" << b->to_string() << " > " << c->to_string() << ")";
                break;
            case TAC_OP::GE:
                oss << a->to_string() << " = (" << b->to_string() << " >= " << c->to_string() << ")";
                break;
            case TAC_OP::NEG:
                oss << a->to_string() << " = -" << b->to_string();
                break;
            case TAC_OP::COPY:
                oss << a->to_string() << " = " << b->to_string();
                break;
            case TAC_OP::GOTO:
                oss << "goto " << a->to_string();
                break;
            case TAC_OP::IFZ:
                oss << "ifz " << b->to_string() << " goto " << a->to_string();
                break;
            case TAC_OP::LABEL:
                oss << "label " << a->to_string();
                break;
            case TAC_OP::VAR:
                oss << "var " << a->to_string();
                if (a->is_array && a->array_metadata)
                {
                    oss << " : array";
                    if (a->array_metadata->base_type == DATA_TYPE::STRUCT)
                    {
                        oss << " of struct " << a->array_metadata->struct_type_name;
                    }
                    else
                    {
                        oss << " of " << data_type_to_string(a->array_metadata->base_type);
                    }
                }
                else if (a->data_type == DATA_TYPE::STRUCT)
                {
                    oss << " : struct";
                    if (!a->struct_type_name.empty())
                    {
                        oss << " " << a->struct_type_name;
                    }
                }
                else if (a->data_type != DATA_TYPE::UNDEF)
                {
                    oss << " : " << data_type_to_string(a->data_type);
                }
                break;
            case TAC_OP::FORMAL:
                oss << "formal " << a->to_string();
                break;
            case TAC_OP::ACTUAL:
                oss << "actual " << a->to_string();
                break;
            case TAC_OP::CALL:
                if (a != nullptr)
                {
                    oss << a->to_string() << " = call " << b->to_string();
                }
                else
                {
                    oss << "call " << b->to_string();
                }
                break;
            case TAC_OP::RETURN:
                if (a != nullptr)
                {
                    oss << "return " << a->to_string();
                }
                else
                {
                    oss << "return";
                }
                break;
            case TAC_OP::INPUT:
                oss << "input " << a->to_string();
                break;
            case TAC_OP::OUTPUT:
                oss << "output " << a->to_string();
                break;
            case TAC_OP::BEGINFUNC:
                oss << "begin";
                break;
            case TAC_OP::ENDFUNC:
                oss << "end";
                break;
            case TAC_OP::ADDR:
                oss << a->to_string() << " = &" << b->to_string();
                break;
            case TAC_OP::LOAD_PTR:
                oss << a->to_string() << " = *" << b->to_string();
                break;
            case TAC_OP::STORE_PTR:
                oss << "*" << a->to_string() << " = " << b->to_string();
                break;
            default:
                oss << "undef";
                break;
            }

            return oss.str();
        }
    };

    struct EXP
    {
        std::shared_ptr<TAC> code;
        std::shared_ptr<SYM> place;
        DATA_TYPE data_type;       // type of the expression result
        std::shared_ptr<EXP> next; // for argument lists

        EXP() : code(nullptr), place(nullptr),
                data_type(DATA_TYPE::UNDEF), next(nullptr) {}
    };

    struct LoopContext {
        std::shared_ptr<SYM> break_label;
        std::shared_ptr<SYM> continue_label;
        std::shared_ptr<SYM> loop_start_label;
    };

    struct SwitchContext {
        std::shared_ptr<SYM> break_label;
        std::unordered_map<int, std::shared_ptr<SYM>> case_labels;
        std::shared_ptr<SYM> default_label;

        SwitchContext(std::shared_ptr<SYM> break_lbl, std::shared_ptr<SYM> default_lbl)
            : break_label(break_lbl), default_label(default_lbl) {}
    };
}