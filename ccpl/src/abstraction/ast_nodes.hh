#pragma once
#include <string>
#include <memory>
#include <vector>
#include "tac_definitions.hh"

namespace twlm::ccpl::abstraction
{
    // Forward declarations
    struct ASTNode;
    struct Expression;
    struct Statement;
    struct Declaration;
    struct Type;

    // ============ Type System ============
    
    enum class TypeKind {
        BASIC,      // int, char, void
        POINTER,    // T*
        ARRAY,      // T[n]
        FUNCTION,   // T(params)
        STRUCT      // struct {...}
    };

    struct Type {
        TypeKind kind;
        DATA_TYPE basic_type;  // For BASIC types
        std::shared_ptr<Type> base_type;  // For POINTER, ARRAY, FUNCTION
        
        // For ARRAY
        int array_size;
        
        // For FUNCTION
        std::vector<std::shared_ptr<Type>> param_types;
        std::shared_ptr<Type> return_type;
        
        // For STRUCT
        std::string struct_name;
        std::vector<std::pair<std::string, std::shared_ptr<Type>>> fields;
        
        Type(TypeKind k = TypeKind::BASIC, DATA_TYPE dt = DATA_TYPE::UNDEF)
            : kind(k), basic_type(dt), array_size(0) {}
        
        static std::shared_ptr<Type> make_basic(DATA_TYPE dt) {
            return std::make_shared<Type>(TypeKind::BASIC, dt);
        }
        
        static std::shared_ptr<Type> make_pointer(std::shared_ptr<Type> base) {
            auto t = std::make_shared<Type>(TypeKind::POINTER);
            t->base_type = base;
            return t;
        }
        
        static std::shared_ptr<Type> make_array(std::shared_ptr<Type> base, int size) {
            auto t = std::make_shared<Type>(TypeKind::ARRAY);
            t->base_type = base;
            t->array_size = size;
            return t;
        }
        
        static std::shared_ptr<Type> make_function(
            std::shared_ptr<Type> ret_type,
            const std::vector<std::shared_ptr<Type>>& params) {
            auto t = std::make_shared<Type>(TypeKind::FUNCTION);
            t->return_type = ret_type;
            t->param_types = params;
            return t;
        }
        
        std::string to_string() const;
        bool is_pointer() const { return kind == TypeKind::POINTER; }
        bool is_array() const { return kind == TypeKind::ARRAY; }
        bool is_function() const { return kind == TypeKind::FUNCTION; }
        bool is_struct() const { return kind == TypeKind::STRUCT; }
    };

    // ============ Base AST Node ============
    
    enum class ASTNodeKind {
        // Expressions
        CONST_INT,
        CONST_CHAR,
        STRING_LITERAL,
        IDENTIFIER,
        BINARY_OP,
        UNARY_OP,
        ASSIGN,
        FUNC_CALL,
        ARRAY_ACCESS,
        MEMBER_ACCESS,
        ADDRESS_OF,
        DEREFERENCE,
        
        // Statements
        EXPR_STMT,
        IF_STMT,
        WHILE_STMT,
        FOR_STMT,
        RETURN_STMT,
        BREAK_STMT,
        CONTINUE_STMT,
        BLOCK_STMT,
        INPUT_STMT,
        OUTPUT_STMT,
        SWITCH_STMT,
        CASE_STMT,
        DEFAULT_STMT,
        
        // Declarations
        VAR_DECL,
        FUNC_DECL,
        PARAM_DECL,
        STRUCT_DECL,
        
        // Program
        PROGRAM
    };

    struct ASTNode {
        ASTNodeKind kind;
        int line_number;
        
        ASTNode(ASTNodeKind k) : kind(k), line_number(0) {}
        virtual ~ASTNode() = default;
        virtual std::string to_string() const = 0;
    };

    // ============ Expressions ============
    
    struct Expression : public ASTNode {
        std::shared_ptr<Type> expr_type;
        
        Expression(ASTNodeKind k) : ASTNode(k) {}
    };

    struct ConstIntExpr : public Expression {
        int value;
        
        ConstIntExpr(int val) : Expression(ASTNodeKind::CONST_INT), value(val) {
            expr_type = Type::make_basic(DATA_TYPE::INT);
        }
        
        std::string to_string() const override {
            return std::to_string(value);
        }
    };

    struct ConstCharExpr : public Expression {
        char value;
        
        ConstCharExpr(char val) : Expression(ASTNodeKind::CONST_CHAR), value(val) {
            expr_type = Type::make_basic(DATA_TYPE::CHAR);
        }
        
        std::string to_string() const override {
            return "'" + std::string(1, value) + "'";
        }
    };

    struct StringLiteralExpr : public Expression {
        std::string value;
        
        StringLiteralExpr(const std::string& val) 
            : Expression(ASTNodeKind::STRING_LITERAL), value(val) {
            // String is char*
            expr_type = Type::make_pointer(Type::make_basic(DATA_TYPE::CHAR));
        }
        
        std::string to_string() const override {
            return "\"" + value + "\"";
        }
    };

    struct IdentifierExpr : public Expression {
        std::string name;
        
        IdentifierExpr(const std::string& n) 
            : Expression(ASTNodeKind::IDENTIFIER), name(n) {}
        
        std::string to_string() const override {
            return name;
        }
    };

    struct BinaryOpExpr : public Expression {
        TAC_OP op;
        std::shared_ptr<Expression> left;
        std::shared_ptr<Expression> right;
        
        BinaryOpExpr(TAC_OP operation, 
                     std::shared_ptr<Expression> l, 
                     std::shared_ptr<Expression> r)
            : Expression(ASTNodeKind::BINARY_OP), op(operation), left(l), right(r) {}
        
        std::string to_string() const override;
    };

    struct UnaryOpExpr : public Expression {
        TAC_OP op;
        std::shared_ptr<Expression> operand;
        
        UnaryOpExpr(TAC_OP operation, std::shared_ptr<Expression> expr)
            : Expression(ASTNodeKind::UNARY_OP), op(operation), operand(expr) {}
        
        std::string to_string() const override;
    };

    struct AssignExpr : public Expression {
        std::shared_ptr<Expression> target;  // Can be Identifier, ArrayAccess, Dereference, etc.
        std::shared_ptr<Expression> value;
        
        AssignExpr(std::shared_ptr<Expression> t, std::shared_ptr<Expression> v)
            : Expression(ASTNodeKind::ASSIGN), target(t), value(v) {}
        
        std::string to_string() const override;
    };

    struct FuncCallExpr : public Expression {
        std::string func_name;
        std::vector<std::shared_ptr<Expression>> arguments;
        
        FuncCallExpr(const std::string& name) 
            : Expression(ASTNodeKind::FUNC_CALL), func_name(name) {}
        
        std::string to_string() const override;
    };

    struct ArrayAccessExpr : public Expression {
        std::shared_ptr<Expression> array;
        std::shared_ptr<Expression> index;
        
        ArrayAccessExpr(std::shared_ptr<Expression> arr, std::shared_ptr<Expression> idx)
            : Expression(ASTNodeKind::ARRAY_ACCESS), array(arr), index(idx) {}
        
        std::string to_string() const override;
    };

    struct MemberAccessExpr : public Expression {
        std::shared_ptr<Expression> object;
        std::string member_name;
        bool is_pointer_access;  // true for ->, false for .
        
        MemberAccessExpr(std::shared_ptr<Expression> obj, 
                         const std::string& member, 
                         bool is_ptr = false)
            : Expression(ASTNodeKind::MEMBER_ACCESS), 
              object(obj), member_name(member), is_pointer_access(is_ptr) {}
        
        std::string to_string() const override;
    };

    struct AddressOfExpr : public Expression {
        std::shared_ptr<Expression> operand;
        
        AddressOfExpr(std::shared_ptr<Expression> expr)
            : Expression(ASTNodeKind::ADDRESS_OF), operand(expr) {}
        
        std::string to_string() const override;
    };

    struct DereferenceExpr : public Expression {
        std::shared_ptr<Expression> operand;
        
        DereferenceExpr(std::shared_ptr<Expression> expr)
            : Expression(ASTNodeKind::DEREFERENCE), operand(expr) {}
        
        std::string to_string() const override;
    };

    // ============ Statements ============
    
    struct Statement : public ASTNode {
        Statement(ASTNodeKind k) : ASTNode(k) {}
    };

    struct ExprStmt : public Statement {
        std::shared_ptr<Expression> expression;
        
        ExprStmt(std::shared_ptr<Expression> expr)
            : Statement(ASTNodeKind::EXPR_STMT), expression(expr) {}
        
        std::string to_string() const override;
    };

    struct BlockStmt : public Statement {
        std::vector<std::shared_ptr<Statement>> statements;
        
        BlockStmt() : Statement(ASTNodeKind::BLOCK_STMT) {}
        
        std::string to_string() const override;
    };

    struct IfStmt : public Statement {
        std::shared_ptr<Expression> condition;
        std::shared_ptr<Statement> then_branch;
        std::shared_ptr<Statement> else_branch;  // Can be nullptr
        
        IfStmt(std::shared_ptr<Expression> cond,
               std::shared_ptr<Statement> then_stmt,
               std::shared_ptr<Statement> else_stmt = nullptr)
            : Statement(ASTNodeKind::IF_STMT), 
              condition(cond), then_branch(then_stmt), else_branch(else_stmt) {}
        
        std::string to_string() const override;
    };

    struct WhileStmt : public Statement {
        std::shared_ptr<Expression> condition;
        std::shared_ptr<Statement> body;
        
        WhileStmt(std::shared_ptr<Expression> cond, std::shared_ptr<Statement> b)
            : Statement(ASTNodeKind::WHILE_STMT), condition(cond), body(b) {}
        
        std::string to_string() const override;
    };

    struct ForStmt : public Statement {
        std::shared_ptr<Statement> init;  // Can be ExprStmt or VarDecl
        std::shared_ptr<Expression> condition;
        std::shared_ptr<Expression> update;
        std::shared_ptr<Statement> body;
        
        ForStmt(std::shared_ptr<Statement> i,
                std::shared_ptr<Expression> cond,
                std::shared_ptr<Expression> upd,
                std::shared_ptr<Statement> b)
            : Statement(ASTNodeKind::FOR_STMT), 
              init(i), condition(cond), update(upd), body(b) {}
        
        std::string to_string() const override;
    };

    struct ReturnStmt : public Statement {
        std::shared_ptr<Expression> return_value;  // Can be nullptr for void return
        
        ReturnStmt(std::shared_ptr<Expression> val = nullptr)
            : Statement(ASTNodeKind::RETURN_STMT), return_value(val) {}
        
        std::string to_string() const override;
    };

    struct BreakStmt : public Statement {
        BreakStmt() : Statement(ASTNodeKind::BREAK_STMT) {}
        
        std::string to_string() const override {
            return "break";
        }
    };

    struct ContinueStmt : public Statement {
        ContinueStmt() : Statement(ASTNodeKind::CONTINUE_STMT) {}
        
        std::string to_string() const override {
            return "continue";
        }
    };

    struct InputStmt : public Statement {
        std::string var_name;
        
        InputStmt(const std::string& name)
            : Statement(ASTNodeKind::INPUT_STMT), var_name(name) {}
        
        std::string to_string() const override;
    };

    struct OutputStmt : public Statement {
        std::shared_ptr<Expression> expression;
        
        OutputStmt(std::shared_ptr<Expression> expr)
            : Statement(ASTNodeKind::OUTPUT_STMT), expression(expr) {}
        
        std::string to_string() const override;
    };

    struct SwitchStmt : public Statement {
        std::shared_ptr<Expression> condition;
        std::shared_ptr<Statement> body;  // BlockStmt containing cases
        
        SwitchStmt(std::shared_ptr<Expression> cond, std::shared_ptr<Statement> b)
            : Statement(ASTNodeKind::SWITCH_STMT), condition(cond), body(b) {}
        
        std::string to_string() const override;
    };

    struct CaseStmt : public Statement {
        int value;
        
        CaseStmt(int val) : Statement(ASTNodeKind::CASE_STMT), value(val) {}
        
        std::string to_string() const override;
    };

    struct DefaultStmt : public Statement {
        DefaultStmt() : Statement(ASTNodeKind::DEFAULT_STMT) {}
        
        std::string to_string() const override {
            return "default";
        }
    };

    // ============ Declarations ============
    
    struct Declaration : public Statement {
        Declaration(ASTNodeKind k) : Statement(k) {}
    };

    struct VarDecl : public Declaration {
        std::shared_ptr<Type> var_type;
        std::string name;
        std::shared_ptr<Expression> init_value;  // Can be nullptr
        
        VarDecl(std::shared_ptr<Type> type, 
                const std::string& n,
                std::shared_ptr<Expression> init = nullptr)
            : Declaration(ASTNodeKind::VAR_DECL), 
              var_type(type), name(n), init_value(init) {}
        
        std::string to_string() const override;
    };

    struct ParamDecl : public Declaration {
        std::shared_ptr<Type> param_type;
        std::string name;
        
        ParamDecl(std::shared_ptr<Type> type, const std::string& n)
            : Declaration(ASTNodeKind::PARAM_DECL), param_type(type), name(n) {}
        
        std::string to_string() const override;
    };

    struct FuncDecl : public Declaration {
        std::shared_ptr<Type> return_type;
        std::string name;
        std::vector<std::shared_ptr<ParamDecl>> parameters;
        std::shared_ptr<BlockStmt> body;  // Can be nullptr for forward declaration
        
        FuncDecl(std::shared_ptr<Type> ret_type,
                 const std::string& n,
                 const std::vector<std::shared_ptr<ParamDecl>>& params,
                 std::shared_ptr<BlockStmt> b = nullptr)
            : Declaration(ASTNodeKind::FUNC_DECL),
              return_type(ret_type), name(n), parameters(params), body(b) {}
        
        std::string to_string() const override;
    };

    struct StructDecl : public Declaration {
        std::string name;
        std::vector<std::shared_ptr<VarDecl>> fields;
        
        StructDecl(const std::string& n)
            : Declaration(ASTNodeKind::STRUCT_DECL), name(n) {}
        
        std::string to_string() const override;
    };

    // ============ Program (Root) ============
    
    struct Program : public ASTNode {
        std::vector<std::shared_ptr<Declaration>> declarations;
        
        Program() : ASTNode(ASTNodeKind::PROGRAM) {}
        
        std::string to_string() const override;
    };

} // namespace twlm::ccpl::abstraction
