%option noyywrap nodefault
%option yylineno

%{
#include "parser.tab.hh"
#include <cstdlib>
#include <string>

#define YY_DECL yy::parser::symbol_type yylex()
%}

%%

";"  {  return yy::parser::make_EOL();  }

"int"  {  return yy::parser::make_INT();  }

"char"  {  return yy::parser::make_CHAR();  }

"void"  {  return yy::parser::make_VOID();  }

"input"  {  return yy::parser::make_INPUT();  }

"output"  {  return yy::parser::make_OUTPUT();  }

"return"  {  return yy::parser::make_RETURN();  }

"if"  {  return yy::parser::make_IF();  }

"else"  {  return yy::parser::make_ELSE();  }

"while"  {  return yy::parser::make_WHILE();  }

"for"  {  return yy::parser::make_FOR();  }

[A-Za-z]([A-Za-z]|[0-9])*  {  
	return yy::parser::make_IDENTIFIER(std::string(yytext));
}

[0-9]+	{
	return yy::parser::make_INTEGER(atoi(yytext));
}

\"[^\"]*\"  {
	return yy::parser::make_TEXT(std::string(yytext));
}

\'[^']\'  {
    return yy::parser::make_CHARACTER(yytext[1]);
}

"=="  {  return yy::parser::make_EQ();  }

"!="  {  return yy::parser::make_NE();  }

"<="  {  return yy::parser::make_LE();  }

"<"  {  return yy::parser::make_LT();  }

">="  {  return yy::parser::make_GE();  }

">"  {  return yy::parser::make_GT();  }

[ \t\r\n]|#.*

. 	{	return yy::parser::symbol_type(yytext[0]); }

%%

