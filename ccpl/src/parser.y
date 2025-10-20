%language "c++"
%defines
%define api.value.type variant
%define api.token.constructor
%define parse.error verbose

%code requires {
    #include <string>
}

%code {
    #include <iostream>
    #include <cstdlib>
    
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


%right '='
%right UMINUS
%left '+' '-'
%left '*' '/'
%left EQ NE LT LE GT GE

%%

program: func_decl_list
{
    std::cout << "Parsing completed successfully." << std::endl;
    std::exit(0);
}
;

func_decl_list: func_decl
    | func_decl_list func_decl
;

func_decl: function
    | decl
;

type_spec: INT
    | CHAR
    | VOID
;

decl: type_spec decl_list EOL
;

decl_list: IDENTIFIER
    | decl_list ',' IDENTIFIER
;

function: func_header '(' param_list ')' block
;
func_header: type_spec IDENTIFIER
;
param_list: /* empty */
    | param_decl_list
;
param_decl_list: param_decl
    | param_decl_list ',' param_decl
;
param_decl: type_spec IDENTIFIER
;
block: '{' stmt_list '}'
;
stmt_list: /* empty */
    | stmt_list statement
;
statement: decl
    | if_stmt
    | while_stmt
    | for_stmt
    | expr_stmt EOL
    | input_stmt EOL
    | output_stmt EOL
    | return_stmt EOL
;

expr_stmt: expression
;

if_stmt: IF '(' expression ')' block
    | IF '(' expression ')' block ELSE block
;

while_stmt: WHILE '(' expression ')' block
;
for_stmt: FOR '(' expr_stmt EOL expression EOL expr_stmt ')' block
;

input_stmt: INPUT IDENTIFIER
;
output_stmt: OUTPUT expression
;
return_stmt: RETURN expression
| RETURN
;

const_expr: INTEGER
    | CHARACTER
    | TEXT
;

func_call_expr: IDENTIFIER '(' arg_list ')'
;

assign_expr: IDENTIFIER '=' expression
;

expression: const_expr
    | func_call_expr
    | assign_expr
    | IDENTIFIER
    | expression '+' expression
    | expression '-' expression
    | expression '*' expression
    | expression '/' expression
    | expression EQ expression
    | expression NE expression
    | expression LT expression
    | expression LE expression
    | expression GT expression
    | expression GE expression
    | '-' expression %prec UMINUS
    | '(' expression ')'
;
arg_list: /* empty */
    | arg_list_nonempty
;
arg_list_nonempty: expression
    | arg_list_nonempty ',' expression
;

%%

void yy::parser::error(const std::string& msg) {
    std::cerr << "Parse error: " << msg << " in line " << yylineno << std::endl;
}