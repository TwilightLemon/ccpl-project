/* type of symbol */
#define SYM_UNDEF 0
#define SYM_VAR 1
#define SYM_FUNC 2
#define SYM_TEXT 3
#define SYM_INT 4
#define SYM_LABEL 5

/* type of tac */ 
#define TAC_UNDEF 0 /* undefine */
#define TAC_ADD 1 /* a=b+c */
#define TAC_SUB 2 /* a=b-c */
#define TAC_MUL 3 /* a=b*c */
#define TAC_DIV 4 /* a=b/c */
#define TAC_EQ 5 /* a=(b==c) */
#define TAC_NE 6 /* a=(b!=c) */
#define TAC_LT 7 /* a=(b<c) */
#define TAC_LE 8 /* a=(b<=c) */
#define TAC_GT 9 /* a=(b>c) */
#define TAC_GE 10 /* a=(b>=c) */
#define TAC_NEG 11 /* a=-b */
#define TAC_COPY 12 /* a=b */
#define TAC_GOTO 13 /* goto a */
#define TAC_IFZ 14 /* ifz b goto a */
#define TAC_BEGINFUNC 15 /* function begin */
#define TAC_ENDFUNC 16 /* function end */
#define TAC_LABEL 17 /* label a */
#define TAC_VAR 18 /* int a */
#define TAC_FORMAL 19 /* formal a */
#define TAC_ACTUAL 20 /* actual a */
#define TAC_CALL 21 /* a=call b */
#define TAC_RETURN 22 /* return a */
#define TAC_INPUT 23 /* input a */
#define TAC_OUTPUT 24 /* output a */

typedef struct sym
{
	/*	
		type:SYM_VAR name:abc value:98 offset:-1
		type:SYM_VAR name:bcd value:99 offset:4
		type:SYM_LABEL name:L1/max			
		type:SYM_INT value:1			
		type:SYM_FUNC name:max address:1234		
		type:SYM_TEXT name:"hello" label:10
	*/
	int type;
	int scope; /* 0:global, 1:local */
	char *name;
	int offset;
	int value;
	int label;
	struct tac *address; /* SYM_FUNC */	
	struct sym *next;
	void *etc;
} SYM;

typedef struct tac
{
	struct tac  *next;
	struct tac  *prev;
	int op;
	SYM *a;
	SYM *b;
	SYM *c;
	void *etc;
} TAC;

typedef struct exp
{
	struct exp *next; /* for argument list */
	TAC *tac; /* code */
	SYM *ret; /* return value */
	void *etc;
} EXP;

/* global var */
extern FILE *file_x, *file_s;
extern int yylineno, scope, next_tmp, next_label;
extern SYM *sym_tab_global, *sym_tab_local;
extern TAC *tac_first, *tac_last;

/* function */
void tac_init();
void tac_complete();
TAC *join_tac(TAC *c1, TAC *c2);
void out_str(FILE *f, const char *format, ...);
void out_sym(FILE *f, SYM *s);
void out_tac(FILE *f, TAC *i);
SYM *mk_label(char *name);
SYM *mk_tmp(void);
SYM *mk_const(int n);
SYM *mk_text(char *text);
TAC *mk_tac(int op, SYM *a, SYM *b, SYM *c);
EXP *mk_exp(EXP *next, SYM *ret, TAC *code);
char *mk_lstr(int i);
SYM *get_var(char *name); 
SYM *declare_func(char *name);
TAC *declare_var(char *name);
TAC *declare_para(char *name);
TAC *do_func(SYM *name,    TAC *args, TAC *code);
TAC *do_assign(SYM *var, EXP *exp);
TAC *do_output(SYM *var);
TAC *do_input(SYM *var);
TAC *do_call(char *name, EXP *arglist);
TAC *do_if(EXP *exp, TAC *stmt);
TAC *do_test(EXP *exp, TAC *stmt1, TAC *stmt2);
TAC *do_while(EXP *exp, TAC *stmt);
EXP *do_bin( int binop, EXP *exp1, EXP *exp2);
EXP *do_cmp( int binop, EXP *exp1, EXP *exp2);
EXP *do_un( int unop, EXP *exp);
EXP *do_call_ret(char *name, EXP *arglist);
void error(const char *format, ...);
