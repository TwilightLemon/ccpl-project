%language "c++"
%defines
%define api.value.type variant
%define api.token.constructor
%define parse.error verbose

%code requires {
    #include <string>
    #include <memory>
    #include "tac.hh"
}

%code {
    #include <iostream>
    #include <cstdlib>
    #include "tac.hh"
    
    yy::parser::symbol_type yylex();
    extern int yylineno;
}

%token EOL
%token INT CHAR VOID
%token EQ NE LT LE GT GE UMINUS
%token IF ELSE FOR WHILE INPUT OUTPUT RETURN

%token <std::string> IDENTIFIER TEXT
%token <int> INTEGER
%token <char> CHARACTER

%type <std::shared_ptr<TAC>> func_decl_list func_decl function decl decl_list block stmt_list statement
%type <std::shared_ptr<TAC>> expr_stmt if_stmt while_stmt for_stmt input_stmt output_stmt return_stmt
%type <std::shared_ptr<TAC>> param_list param_decl_list param_decl
%type <std::shared_ptr<EXP>> expression const_expr func_call_expr assign_expr arg_list arg_list_nonempty
%type <std::shared_ptr<SYM>> func_header
%type <DATA_TYPE> type_spec

%right '='
%right UMINUS
%left '+' '-'
%left '*' '/'
%left EQ NE LT LE GT GE

%%

program: func_decl_list
{
    tac_gen.complete();
    std::clog << "Parsing completed successfully." << std::endl;
}
;

func_decl_list: func_decl
{
    $$ = $1;
}
| func_decl_list func_decl
{
    $$ = tac_gen.join_tac($1, $2);
}
;

func_decl: function
{
    $$ = $1;
}
| decl
{
    $$ = $1;
}
;

type_spec: INT
{
    tac_gen.set_current_type(DATA_TYPE::INT);
    $$ = DATA_TYPE::INT;
}
| CHAR
{
    tac_gen.set_current_type(DATA_TYPE::CHAR);
    $$ = DATA_TYPE::CHAR;
}
| VOID
{
    tac_gen.set_current_type(DATA_TYPE::VOID);
    $$ = DATA_TYPE::VOID;
}
;

decl: type_spec decl_list EOL
{
    $$ = $2;
}
;

decl_list: IDENTIFIER
{
    $$ = tac_gen.declare_var($1, tac_gen.get_current_type());
}
| decl_list ',' IDENTIFIER
{
    $$ = tac_gen.join_tac($1, tac_gen.declare_var($3, tac_gen.get_current_type()));
}
;

function: func_header '(' param_list ')' block
{
    $$ = tac_gen.do_func($1, $3, $5);
    tac_gen.leave_scope();
}
;

func_header: type_spec IDENTIFIER
{
    $$ = tac_gen.declare_func($2, $1);
    tac_gen.enter_scope();
}
;

param_list: /* empty */
{
    $$ = nullptr;
}
| param_decl_list
{
    $$ = $1;
}
;

param_decl_list: param_decl
{
    $$ = $1;
}
| param_decl_list ',' param_decl
{
    $$ = tac_gen.join_tac($1, $3);
}
;

param_decl: type_spec IDENTIFIER
{
    $$ = tac_gen.declare_para($2, $1);
}
;

block: '{' stmt_list '}'
{
    $$ = $2;
}
;

stmt_list: /* empty */
{
    $$ = nullptr;
}
| stmt_list statement
{
    $$ = tac_gen.join_tac($1, $2);
}
;

statement: decl
{
    $$ = $1;
}
| if_stmt
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
;

expr_stmt: expression
{
    $$ = $1->code;
}
;

if_stmt: IF '(' expression ')' block
{
    $$ = tac_gen.do_if($3, $5);
}
| IF '(' expression ')' block ELSE block
{
    $$ = tac_gen.do_if_else($3, $5, $7);
}
;

while_stmt: WHILE '(' expression ')' block
{
    $$ = tac_gen.do_while($3, $5);
}
;

for_stmt: FOR '(' expr_stmt EOL expression EOL expr_stmt ')' block
{
    $$ = tac_gen.do_for($3, $5, $7, $9);
}
;

input_stmt: INPUT IDENTIFIER
{
    $$ = tac_gen.do_input(tac_gen.get_var($2));
}
;

output_stmt: OUTPUT expression
{
    auto output_tac = tac_gen.do_output($2->place);
    $$ = tac_gen.join_tac($2->code, output_tac);
}
;

return_stmt: RETURN expression
{
    $$ = tac_gen.do_return($2);
}
| RETURN
{
    $$ = tac_gen.do_return(nullptr);
}
;

const_expr: INTEGER
{
    auto sym = tac_gen.mk_const($1);
    auto exp = tac_gen.mk_exp(sym, nullptr);
    exp->data_type = DATA_TYPE::INT;
    $$ = exp;
}
| CHARACTER
{
    auto sym = tac_gen.mk_const_char($1);
    auto exp = tac_gen.mk_exp(sym, nullptr);
    exp->data_type = DATA_TYPE::CHAR;
    $$ = exp;
}
| TEXT
{
    auto sym = tac_gen.mk_text($1);
    auto exp = tac_gen.mk_exp(sym, nullptr);
    exp->data_type = DATA_TYPE::CHAR;  // String is array of char
    $$ = exp;
}
;

func_call_expr: IDENTIFIER '(' arg_list ')'
{
    $$ = tac_gen.do_call_ret($1, $3);
}
;

assign_expr: IDENTIFIER '=' expression
{
    auto assign_tac = tac_gen.do_assign(tac_gen.get_var($1), $3);
    // Return expression with the assigned variable as place
    $$ = tac_gen.mk_exp(tac_gen.get_var($1), assign_tac);
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
    auto var = tac_gen.get_var($1);
    auto exp = tac_gen.mk_exp(var, nullptr);
    if (var) {
        exp->data_type = var->data_type;
    }
    $$ = exp;
}
| expression '+' expression
{
    $$ = tac_gen.do_bin(TAC_OP::ADD, $1, $3);
}
| expression '-' expression
{
    $$ = tac_gen.do_bin(TAC_OP::SUB, $1, $3);
}
| expression '*' expression
{
    $$ = tac_gen.do_bin(TAC_OP::MUL, $1, $3);
}
| expression '/' expression
{
    $$ = tac_gen.do_bin(TAC_OP::DIV, $1, $3);
}
| expression EQ expression
{
    $$ = tac_gen.do_bin(TAC_OP::EQ, $1, $3);
}
| expression NE expression
{
    $$ = tac_gen.do_bin(TAC_OP::NE, $1, $3);
}
| expression LT expression
{
    $$ = tac_gen.do_bin(TAC_OP::LT, $1, $3);
}
| expression LE expression
{
    $$ = tac_gen.do_bin(TAC_OP::LE, $1, $3);
}
| expression GT expression
{
    $$ = tac_gen.do_bin(TAC_OP::GT, $1, $3);
}
| expression GE expression
{
    $$ = tac_gen.do_bin(TAC_OP::GE, $1, $3);
}
| '-' expression %prec UMINUS
{
    $$ = tac_gen.do_un(TAC_OP::NEG, $2);
}
| '(' expression ')'
{
    $$ = $2;
}
;

arg_list: /* empty */
{
    $$ = nullptr;
}
| arg_list_nonempty
{
    $$ = $1;
}
;

arg_list_nonempty: expression
{
    $$ = $1;
}
| arg_list_nonempty ',' expression
{
    $3->next = $1;
    $$ = $3;
}
;

%%

void yy::parser::error(const std::string& msg) {
    std::cerr << "Parse error: " << msg << " in line " << yylineno << std::endl;
}