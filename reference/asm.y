%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opcode.h"

#define LABNUM 100

extern int yylineno;

int pass, ip;

struct label
{
	int addr;
	char *name;
} label[LABNUM];

int yylex();
void yyerror(char* msg);
void byte1(int  n);
void byte2(int  n);
void byte4(int n);

int number(char * name)
{
	int index=0;

	while(label[index].name != NULL)
	{
		if(!strcmp(label[index].name, name)) 
			return index;
		index++;
	}

	if(index>=LABNUM)
	{
		fprintf(stderr, "error: too many label");
		exit(0);
	}

	label[index].name=name;
	return index;
}

void byte1(int  n)
{
	if(pass==2)
		putchar(n);
	ip++;
}

void byte2(int  n)
{
	if(pass==2)
	{
		putchar(n);
		putchar(n>>8);
	}
	ip+=2;
}	

void byte4(int n)
{
	if(pass==2)
	{
		putchar(n);
		putchar(n>>8);
		putchar(n>>16);
		putchar(n>>24);
	}
	ip+=4;
}

%}

%union
{
	int number;
	char *string;
}

%token ADD SUB MUL DIV TST STO STC LOD LDC JMP JEZ JLZ JGZ DBN DBS ITC ITI OTC OTI OTS NOP END
%token <number> INTEGER REG
%token <string> LABEL

%%

start : program
;

program : program statement
|
;

statement : nop_stmt
| add_stmt
| sub_stmt
| mul_stmt
| div_stmt
| tst_stmt
| lab_stmt
| jmp_stmt
| jez_stmt
| jlz_stmt
| jgz_stmt
| lod_stmt
| sto_stmt
| output_stmt
| input_stmt
| end_stmt
| dbn_stmt
| dbs_stmt
;

nop_stmt : NOP	{ byte2(I_NOP); byte1(0); byte1(0); byte4(0);}
;

add_stmt : ADD REG ',' INTEGER
{
	byte2(I_ADD_0) ;
	byte1($2);
	byte1(0);
	byte4($4);
}
| ADD REG ',' LABEL
{
	byte2(I_ADD_0);
	byte1($2);
	byte1(0);
	byte4(label[number($4)].addr);
}
| ADD REG ',' REG
{
	byte2(I_ADD_1);
	byte1($2);
	byte1($4);
	byte4(0);
}
;

sub_stmt : SUB REG ',' INTEGER
{
	byte2(I_SUB_0) ;
	byte1($2);
	byte1(0);
	byte4($4);
}
| SUB REG ',' LABEL
{
	byte2(I_SUB_0);
	byte1($2);
	byte1(0);
	byte4(label[number($4)].addr);
}
| SUB REG ',' REG
{
	byte2(I_SUB_1);
	byte1($2);
	byte1($4);
	byte4(0);
}
;

mul_stmt : MUL REG ',' INTEGER
{
	byte2(I_MUL_0) ;
	byte1($2);
	byte1(0);
	byte4($4);
}
| MUL REG ',' LABEL
{
	byte2(I_MUL_0);
	byte1($2);
	byte1(0);
	byte4(label[number($4)].addr);
}
| MUL REG ',' REG
{
	byte2(I_MUL_1);
	byte1($2);
	byte1($4);
	byte4(0);
}
;

div_stmt : DIV REG ',' INTEGER
{
	byte2(I_DIV_0) ;
	byte1($2);
	byte1(0);
	byte4($4);
}
| DIV REG ',' LABEL
{
	byte2(I_DIV_0);
	byte1($2);
	byte1(0);
	byte4(label[number($4)].addr);
}
| DIV REG ',' REG
{
	byte2(I_DIV_1);
	byte1($2);
	byte1($4);
	byte4(0);
}
;

tst_stmt : TST REG
{
	byte2(I_TST_0) ;
	byte1($2);
	byte1(0);
	byte4(0);
}
;

lab_stmt : LABEL ':'
{
	if(pass==1)
	{
		if(label[number($1)].addr==0)
		{
			label[number($1)].addr=ip;
		}
		else
		{
			fprintf(stderr, "error: label %s already exist\n", label[number($1)].name);
			exit(0);
		}
	}
}
;

jmp_stmt : JMP LABEL
{
	byte2(I_JMP_0);
	byte1(0);
	byte1(0);
	byte4(label[number($2)].addr);
}
| JMP REG
{
	byte2(I_JMP_1);
	byte1($2);		
	byte1(0);	
	byte4(0);				
}
;

jez_stmt : JEZ LABEL
{
	byte2(I_JEZ_0);
	byte1(0);
	byte1(0);
	byte4(label[number($2)].addr);
}
| JEZ REG
{
	byte2(I_JEZ_1) ;
	byte1($2);		
	byte1(0);	
	byte4(0);	
}
;

jlz_stmt : JLZ LABEL
{
	byte2(I_JLZ_0);
	byte1(0);
	byte1(0);
	byte4(label[number($2)].addr);
}
| JLZ REG
{
	byte2(I_JLZ_1) ;
	byte1( $2 ) ;		
	byte1(0);	
	byte4(0);	
}
;

jgz_stmt : JGZ LABEL
{
	byte2(I_JGZ_0);
	byte1(0);
	byte1(0);
	byte4(label[number($2)].addr);
}
| JGZ REG
{
	byte2(I_JGZ_1) ;
	byte1( $2 ) ;		
	byte1(0);	
	byte4(0);	
}
;			

lod_stmt : LOD REG ',' INTEGER
{
	byte2(I_LOD_0) ;
	byte1($2);
	byte1(0);
	byte4($4);
}
| LOD REG ',' LABEL
{
	byte2(I_LOD_0);
	byte1($2);
	byte1(0);
	byte4(label[number($4)].addr);
}
| LOD REG ',' REG
{
	byte2(I_LOD_1);
	byte1($2);
	byte1($4);
	byte4(0);
}
| LOD REG ',' REG '+' INTEGER
{
	byte2(I_LOD_2);
	byte1($2);
	byte1($4);
	byte4($6);
}
| LOD REG ',' REG '-' INTEGER
{
	byte2(I_LOD_2);
	byte1($2);
	byte1($4);
	byte4(-($6));
}
| LOD REG ',' '(' INTEGER ')'
{
	byte2(I_LOD_3);
	byte1($2);
	byte1(0);
	byte4($5);
}
| LDC REG ',' '(' INTEGER ')'
{
	byte2(I_LDC_3);
	byte1($2);
	byte1(0);
	byte4($5);
}
| LOD REG ',' '(' LABEL ')'
{
	byte2(I_LOD_3);
	byte1($2);
	byte1(0);
	byte4(label[number($5)].addr);
}
| LOD REG ',' '(' REG ')'
{
	byte2(I_LOD_4);
	byte1($2);
	byte1($5);
	byte4(0);
}
| LDC REG ',' '(' REG ')'
{
	byte2(I_LDC_4);
	byte1($2);
	byte1($5);
	byte4(0);
}
| LOD REG ',' '(' REG '+' INTEGER ')'
{
	byte2(I_LOD_5);
	byte1($2);
	byte1($5);
	byte4($7);
}
| LDC REG ',' '(' REG '+' INTEGER ')'
{
	byte2(I_LDC_5);
	byte1($2);
	byte1($5);
	byte4($7);
}
| LOD REG ',' '(' REG '-' INTEGER ')'
{
	byte2(I_LOD_5);
	byte1($2);
	byte1($5);
	byte4(-($7));
}
| LDC REG ',' '(' REG '-' INTEGER ')'
{
	byte2(I_LDC_5);
	byte1($2);
	byte1($5);
	byte4(-($7));
}
;

sto_stmt : STO '(' REG ')' ',' INTEGER
{
	byte2(I_STO_0);
	byte1($3);
	byte1(0);
	byte4($6);
}
| STC '(' REG ')' ',' INTEGER
{
	byte2(I_STC_0);
	byte1($3);
	byte1(0);
	byte4($6);
}
| STO '(' REG ')' ',' LABEL
{
	byte2(I_STO_0);
	byte1($3);
	byte1(0);
	byte4(label[number($6)].addr);
}
| STO '(' REG ')' ',' REG
{
	byte2( I_STO_1 ) ;
	byte1($3);
	byte1($6);
	byte4(0);
}
| STC '(' REG ')' ',' REG
{
	byte2( I_STC_1 ) ;
	byte1($3);
	byte1($6);
	byte4(0);
}
| STO '(' REG ')' ',' REG '+' INTEGER
{
	byte2( I_STO_2 ) ;
	byte1($3);
	byte1($6);
	byte4($8);
}
| STC '(' REG ')' ',' REG '+' INTEGER
{
	byte2( I_STC_2 ) ;
	byte1($3);
	byte1($6);
	byte4($8);
}
| STO '(' REG ')' ',' REG '-' INTEGER
{
	byte2( I_STO_2 ) ;
	byte1($3);
	byte1($6);
	byte4(-($8));
}
| STC '(' REG ')' ',' REG '-' INTEGER
{
	byte2( I_STC_2 ) ;
	byte1($3);
	byte1($6);
	byte4(-($8));
}
| STO '(' REG '+' INTEGER ')' ',' REG
{
	byte2( I_STO_3 ) ;
	byte1($3);
	byte1($8);
	byte4($5);
}
| STC '(' REG '+' INTEGER ')' ',' REG
{
	byte2( I_STC_3 ) ;
	byte1($3);
	byte1($8);
	byte4($5);
}
| STO '(' REG '-' INTEGER ')' ',' REG
{
	byte2( I_STO_3 ) ;
	byte1($3);
	byte1($8);
	byte4(-($5));
}
| STC '(' REG '-' INTEGER ')' ',' REG
{
	byte2( I_STC_3 ) ;
	byte1($3);
	byte1($8);
	byte4(-($5));
}
;

input_stmt : ITC { byte2(I_ITC); byte1(0); byte1(0); byte4(0);}
| ITI { byte2(I_ITI); byte1(0); byte1(0); byte4(0);}
;

output_stmt : OTC { byte2(I_OTC); byte1(0); byte1(0); byte4(0);}
| OTI { byte2(I_OTI); byte1(0); byte1(0); byte4(0);}
| OTS { byte2(I_OTS); byte1(0); byte1(0); byte4(0);}
;

end_stmt : END { byte2(I_END); byte1(0); byte1(0); byte4(0);}
;

dbn_stmt : DBN INTEGER ',' INTEGER
{
	int n = $4;
	while(n-- > 0)
		byte1($2);
}
;

dbs_stmt : dbs_stmt ',' INTEGER { byte1($3); }
| DBS INTEGER { byte1($2); }
;

%%

void yyerror(char* msg) 
{
	fprintf(stderr, "%s: line %d\n", msg, yylineno);
	exit(0);
}

int main(int argc,   char *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		exit(0);
	}
	
	char *input, *output;

	input = argv[1];
	if(freopen(input, "r", stdin)==NULL)
	{
		fprintf(stderr, "error: open %s failed\n", input);
		return 0;
	}

	output=strdup(input);
	output[strlen(output)-1]='o';

	if(freopen(output, "w", stdout)==NULL)
	{
		fprintf(stderr, "error: open %s failed\n", output);
		return 0;
	}

	int i=sizeof(label);
	bzero(label, i);

	/* First pass, set up labels */
	pass=1;
	ip=0;
	yyparse();

	/* Second pass, generate code */
	pass=2;
	ip=0;
	rewind(stdin) ;
	yyparse();

	return 0;
}

