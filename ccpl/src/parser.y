%language "c++"
%defines
%define api.value.type variant
%define api.token.constructor
%define parse.error verbose

%code requires {
    #include <string>
    #include <memory>
    #include <vector>
    #include "global.hh"
    using namespace twlm::ccpl::abstraction;
}

%code {
    #include <iostream>
    #include <cstdlib>
    #include "global.hh"
    
    yy::parser::symbol_type yylex();
    extern int yylineno;
}

%token EOL
%token INT CHAR VOID STRUCT
%token EQ NE LT LE GT GE UMINUS DEREF
%token IF ELSE FOR WHILE INPUT OUTPUT RETURN
%token BREAK CONTINUE
%token SWITCH CASE DEFAULT

%token <std::string> IDENTIFIER TEXT
%token <int> INTEGER
%token <char> CHARACTER

%type <std::shared_ptr<Declaration>> func_decl struct_decl decl_item
%type <std::vector<std::shared_ptr<Declaration>>> decl_list

%type <std::shared_ptr<Statement>> statement block expr_stmt if_stmt input_stmt output_stmt return_stmt switch_stmt while_stmt for_stmt
%type <std::vector<std::shared_ptr<Statement>>> stmt_list

%type <std::vector<std::shared_ptr<VarDecl>>> var_decl_list struct_field_list

%type <std::shared_ptr<ParamDecl>> param_decl
%type <std::vector<std::shared_ptr<ParamDecl>>> param_list param_decl_list

%type <std::shared_ptr<Expression>> expression const_expr func_call_expr assign_expr
%type <std::vector<std::shared_ptr<Expression>>> arg_list arg_list_nonempty

%type <std::shared_ptr<Type>> type_spec 
%type <std::string> func_name
%type <std::pair<std::shared_ptr<Type>, std::string>> var_declarator direct_declarator

%right '='
%left EQ NE LT LE GT GE
%left '+' '-'
%left '*' '/'
%right UMINUS '&' DEREF

%%

program: decl_list
{
    std::clog << "Parsing completed successfully." << std::endl;
    for (auto& decl : $1) {
        ast_builder.add_declaration(decl);
    }
    ast_builder.complete();
}
;

decl_list:decl_item
{
    $$.push_back($1);
}
|decl_list decl_item
{
    $$ = $1;
    $$.push_back($2);
}
| decl_list type_spec var_decl_list EOL
{
    // global variables
    $$ = $1;
    for (auto& var_decl : $3) {
        $$.push_back(var_decl);
    }
}
| type_spec var_decl_list EOL
{
    for (auto& var_decl : $2) {
        $$.push_back(var_decl);
    }
}
;

decl_item: func_decl
{
    $$ = $1;
}
| struct_decl
{
    $$ = $1;
}
;

func_decl: type_spec func_name '(' param_list ')' block
{
    auto body = std::dynamic_pointer_cast<BlockStmt>($6);
    if (!body) {
        body = std::make_shared<BlockStmt>();
        if ($6) body->statements.push_back($6);
    }
    $$ = ast_builder.make_func_decl($1, $2, $4, body);
}
;

struct_decl: STRUCT IDENTIFIER '{' struct_field_list '}' EOL
{
    $$ = ast_builder.make_struct_decl($2, $4);
}
;

struct_field_list: type_spec var_decl_list EOL
{
    $$ = $2;
}
| struct_field_list type_spec var_decl_list EOL
{
    $$ = $1;
    for (auto& var : $3) {
        $$.push_back(var);
    }
}
;

func_name: IDENTIFIER
{
    $$ = $1;
}
;

type_spec: INT
{
    ast_builder.set_current_type(DATA_TYPE::INT);
    $$ = ast_builder.make_basic_type(DATA_TYPE::INT);
}
| CHAR
{
    ast_builder.set_current_type(DATA_TYPE::CHAR);
    $$ = ast_builder.make_basic_type(DATA_TYPE::CHAR);
}
| VOID
{
    ast_builder.set_current_type(DATA_TYPE::VOID);
    $$ = ast_builder.make_basic_type(DATA_TYPE::VOID);
}
| STRUCT IDENTIFIER
{
    $$ = ast_builder.make_struct_type($2);
}
;

var_declarator: '*' var_declarator
{
    auto pointer_type = ast_builder.make_pointer_type($2.first);
    $$ = std::make_pair(pointer_type, $2.second);
}
| direct_declarator
{
    $$ = $1;
}
;

direct_declarator: IDENTIFIER
{
    $$ = std::make_pair(ast_builder.get_current_type(), $1);
}
| direct_declarator '[' INTEGER ']'
{
    auto array_type = ast_builder.make_array_type($1.first, $3);
    $$ = std::make_pair(array_type, $1.second);
}
;

var_decl_list: var_declarator
{
    auto var = ast_builder.make_var_decl($1.first, $1.second);
    $$.push_back(var);
}
| var_decl_list ',' var_declarator
{
    $$ = $1;
    auto var = ast_builder.make_var_decl($3.first, $3.second);
    $$.push_back(var);
}
;

param_list: /* empty */
{
    $$ = std::vector<std::shared_ptr<ParamDecl>>();
}
| param_decl_list
{
    $$ = $1;
}
;

param_decl_list: param_decl
{
    $$.push_back($1);
}
| param_decl_list ',' param_decl
{
    $$ = $1;
    $$.push_back($3);
}
;

param_decl: type_spec IDENTIFIER
{
    $$ = ast_builder.make_param_decl($1, $2);
}
| type_spec '*' IDENTIFIER
{
    auto pointer_type = ast_builder.make_pointer_type($1);
    $$ = ast_builder.make_param_decl(pointer_type, $3);
}
;

block: '{' stmt_list '}'
{
    auto block = std::make_shared<BlockStmt>();
    block->statements = $2;
    $$ = block;
}
;

stmt_list: /* empty */
{
    $$ = std::vector<std::shared_ptr<Statement>>();
}
| stmt_list statement
{
    $$ = $1;
    if ($2) {
        $$.push_back($2);
    }
}
| stmt_list type_spec var_decl_list EOL
{
    $$ = $1;
    // local variables
    for (auto& var_decl : $3) {
        $$.push_back(var_decl);
    }
}
;

statement: if_stmt
{
    $$ = $1;
}
| while_stmt
{
    $$ = $1;
}
| for_stmt
{
    $$ = $1;
}
| expr_stmt EOL
{
    $$ = $1;
}
| input_stmt EOL
{
    $$ = $1;
}
| output_stmt EOL
{
    $$ = $1;
}
| return_stmt EOL
{
    $$ = $1;
}
| BREAK EOL
{
    $$ = ast_builder.make_break_stmt();
}
| CONTINUE EOL
{
    $$ = ast_builder.make_continue_stmt();
}
| switch_stmt
{
    $$ = $1;
}
| CASE INTEGER ':'
{
    $$ = ast_builder.make_case_stmt($2);
}
| DEFAULT ':'
{
    $$ = ast_builder.make_default_stmt();
}
;

expr_stmt: expression
{
    $$ = ast_builder.make_expr_stmt($1);
}
;

if_stmt: IF '(' expression ')' block
{
    $$ = ast_builder.make_if_stmt($3, $5);
}
| IF '(' expression ')' block ELSE block
{
    $$ = ast_builder.make_if_stmt($3, $5, $7);
}
;

while_stmt: WHILE '(' expression ')' block
{
    $$ = ast_builder.make_while_stmt($3, $5);
}
;

for_stmt: FOR '(' expr_stmt EOL expression EOL expr_stmt ')' block
{
    $$ = ast_builder.make_for_stmt($3, $5, $7 ? std::dynamic_pointer_cast<ExprStmt>($7)->expression : nullptr, $9);
}
;

switch_stmt: SWITCH '(' expression ')' block
{
    $$ = ast_builder.make_switch_stmt($3, $5);
}
;

input_stmt: INPUT IDENTIFIER
{
    $$ = ast_builder.make_input_stmt($2);
}
;

output_stmt: OUTPUT expression
{
    $$ = ast_builder.make_output_stmt($2);
}
;

return_stmt: RETURN expression
{
    $$ = ast_builder.make_return_stmt($2);
}
| RETURN
{
    $$ = ast_builder.make_return_stmt();
}
;

const_expr: INTEGER
{
    $$ = ast_builder.make_const_int($1);
}
| CHARACTER
{
    $$ = ast_builder.make_const_char($1);
}
| TEXT
{
    $$ = ast_builder.make_string_literal($1);
}
;

func_call_expr: IDENTIFIER '(' arg_list ')'
{
    $$ = ast_builder.make_func_call($1, $3);
}
;

assign_expr: IDENTIFIER '=' expression
{
    auto target = ast_builder.make_identifier($1);
    $$ = ast_builder.make_assign(target, $3);
}
| expression '[' expression ']' '=' expression
{
    auto array_access = ast_builder.make_array_access($1, $3);
    $$ = ast_builder.make_assign(array_access, $6);
}
| expression '.' IDENTIFIER '=' expression
{
    auto member = ast_builder.make_member_access($1, $3, false);
    $$ = ast_builder.make_assign(member, $5);
}
| expression '[' expression ']'
{
    $$ = ast_builder.make_array_access($1, $3);
}
| expression '.' IDENTIFIER
{
    $$ = ast_builder.make_member_access($1, $3, false);
}
;

expression: const_expr
{
    $$ = $1;
}
| func_call_expr
{
    $$ = $1;
}
| assign_expr
{
    $$ = $1;
}
| IDENTIFIER
{
    $$ = ast_builder.make_identifier($1);
}
| expression '+' expression
{
    $$ = ast_builder.make_binary_op(TAC_OP::ADD, $1, $3);
}
| expression '-' expression
{
    $$ = ast_builder.make_binary_op(TAC_OP::SUB, $1, $3);
}
| expression '*' expression
{
    $$ = ast_builder.make_binary_op(TAC_OP::MUL, $1, $3);
}
| expression '/' expression
{
    $$ = ast_builder.make_binary_op(TAC_OP::DIV, $1, $3);
}
| expression EQ expression
{
    $$ = ast_builder.make_binary_op(TAC_OP::EQ, $1, $3);
}
| expression NE expression
{
    $$ = ast_builder.make_binary_op(TAC_OP::NE, $1, $3);
}
| expression LT expression
{
    $$ = ast_builder.make_binary_op(TAC_OP::LT, $1, $3);
}
| expression LE expression
{
    $$ = ast_builder.make_binary_op(TAC_OP::LE, $1, $3);
}
| expression GT expression
{
    $$ = ast_builder.make_binary_op(TAC_OP::GT, $1, $3);
}
| expression GE expression
{
    $$ = ast_builder.make_binary_op(TAC_OP::GE, $1, $3);
}
| '-' expression %prec UMINUS
{
    $$ = ast_builder.make_unary_op(TAC_OP::NEG, $2);
}
| '&' expression %prec DEREF
{
    $$ = ast_builder.make_address_of($2);
}
| '*' IDENTIFIER %prec DEREF
{
    auto id = ast_builder.make_identifier($2);
    $$ = ast_builder.make_dereference(id);
}
| '*' IDENTIFIER '=' expression
{
    auto id = ast_builder.make_identifier($2);
    auto deref = ast_builder.make_dereference(id);
    $$ = ast_builder.make_assign(deref, $4);
}
| '(' expression ')'
{
    $$ = $2;
}
;

arg_list: /* empty */
{
    $$ = std::vector<std::shared_ptr<Expression>>();
}
| arg_list_nonempty
{
    $$ = $1;
}
;

arg_list_nonempty: expression
{
    $$.push_back($1);
}
| arg_list_nonempty ',' expression
{
    $$ = $1;
    $$.push_back($3);
}
;

%%

void yy::parser::error(const std::string& msg) {
    std::cerr << "Parse error: " << msg << " in line " << yylineno << std::endl;
}