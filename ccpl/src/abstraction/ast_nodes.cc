#include "ast_nodes.hh"
#include <sstream>

namespace twlm::ccpl::abstraction
{
    std::string Type::to_string() const {
        std::ostringstream oss;
        switch (kind) {
            case TypeKind::BASIC:
                oss << data_type_to_string(basic_type);
                break;
            case TypeKind::POINTER:
                oss << base_type->to_string() << "*";
                break;
            case TypeKind::ARRAY:
                oss << base_type->to_string() << "[" << array_size << "]";
                break;
            case TypeKind::FUNCTION:
                oss << return_type->to_string() << "(";
                for (size_t i = 0; i < param_types.size(); ++i) {
                    if (i > 0) oss << ", ";
                    oss << param_types[i]->to_string();
                }
                oss << ")";
                break;
            case TypeKind::STRUCT:
                oss << "struct " << struct_name;
                break;
        }
        return oss.str();
    }

    std::string BinaryOpExpr::to_string() const {
        std::ostringstream oss;
        oss << "(" << left->to_string();
        switch (op) {
            case TAC_OP::ADD: oss << " + "; break;
            case TAC_OP::SUB: oss << " - "; break;
            case TAC_OP::MUL: oss << " * "; break;
            case TAC_OP::DIV: oss << " / "; break;
            case TAC_OP::EQ: oss << " == "; break;
            case TAC_OP::NE: oss << " != "; break;
            case TAC_OP::LT: oss << " < "; break;
            case TAC_OP::LE: oss << " <= "; break;
            case TAC_OP::GT: oss << " > "; break;
            case TAC_OP::GE: oss << " >= "; break;
            default: oss << " ? "; break;
        }
        oss << right->to_string() << ")";
        return oss.str();
    }

    std::string UnaryOpExpr::to_string() const {
        std::ostringstream oss;
        switch (op) {
            case TAC_OP::NEG: oss << "-"; break;
            default: oss << "?"; break;
        }
        oss << operand->to_string();
        return oss.str();
    }

    std::string AssignExpr::to_string() const {
        return target->to_string() + " = " + value->to_string();
    }

    std::string FuncCallExpr::to_string() const {
        std::ostringstream oss;
        oss << func_name << "(";
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << arguments[i]->to_string();
        }
        oss << ")";
        return oss.str();
    }

    std::string ArrayAccessExpr::to_string() const {
        return array->to_string() + "[" + index->to_string() + "]";
    }

    std::string MemberAccessExpr::to_string() const {
        return object->to_string() + (is_pointer_access ? "->" : ".") + member_name;
    }

    std::string AddressOfExpr::to_string() const {
        return "&" + operand->to_string();
    }

    std::string DereferenceExpr::to_string() const {
        return "*" + operand->to_string();
    }

    std::string ExprStmt::to_string() const {
        if (expression) {
            return expression->to_string() + ";";
        }
        return ";";
    }

    std::string BlockStmt::to_string() const {
        std::ostringstream oss;
        oss << "{\n";
        for (const auto& stmt : statements) {
            oss << "  " << stmt->to_string() << "\n";
        }
        oss << "}";
        return oss.str();
    }

    std::string IfStmt::to_string() const {
        std::ostringstream oss;
        oss << "if (" << condition->to_string() << ") " << then_branch->to_string();
        if (else_branch) {
            oss << " else " << else_branch->to_string();
        }
        return oss.str();
    }

    std::string WhileStmt::to_string() const {
        return "while (" + condition->to_string() + ") " + body->to_string();
    }

    std::string ForStmt::to_string() const {
        std::ostringstream oss;
        oss << "for (";
        if (init) oss << init->to_string();
        oss << "; ";
        if (condition) oss << condition->to_string();
        oss << "; ";
        if (update) oss << update->to_string();
        oss << ") " << body->to_string();
        return oss.str();
    }

    std::string ReturnStmt::to_string() const {
        if (return_value) {
            return "return " + return_value->to_string() + ";";
        }
        return "return;";
    }

    std::string InputStmt::to_string() const {
        return "input " + var_name + ";";
    }

    std::string OutputStmt::to_string() const {
        return "output " + expression->to_string() + ";";
    }

    std::string SwitchStmt::to_string() const {
        return "switch (" + condition->to_string() + ") " + body->to_string();
    }

    std::string CaseStmt::to_string() const {
        return "case " + std::to_string(value) + ":";
    }

    std::string VarDecl::to_string() const {
        std::ostringstream oss;
        oss << var_type->to_string() << " " << name;
        if (init_value) {
            oss << " = " << init_value->to_string();
        }
        oss << ";";
        return oss.str();
    }

    std::string ParamDecl::to_string() const {
        return param_type->to_string() + " " + name;
    }

    std::string FuncDecl::to_string() const {
        std::ostringstream oss;
        oss << return_type->to_string() << " " << name << "(";
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << parameters[i]->to_string();
        }
        oss << ")";
        if (body) {
            oss << " " << body->to_string();
        } else {
            oss << ";";
        }
        return oss.str();
    }

    std::string StructDecl::to_string() const {
        std::ostringstream oss;
        oss << "struct " << name << " {\n";
        for (const auto& field : fields) {
            oss << "  " << field->to_string() << "\n";
        }
        oss << "};";
        return oss.str();
    }

    std::string Program::to_string() const {
        std::ostringstream oss;
        for (const auto& decl : declarations) {
            oss << decl->to_string() << "\n\n";
        }
        return oss.str();
    }

}