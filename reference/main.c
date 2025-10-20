#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "tac.h"
#include "mini.y.h"
#include "obj.h"

FILE *file_x, *file_s;

void error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
	exit(0);
}

void tac_list()
{
	out_str(file_x, "\n# tac list\n\n");
	
	TAC * cur;
	for(cur = tac_first; cur !=NULL; cur=cur->next)
	{
		out_str(file_x, "%p\t", cur);
		out_tac(file_x, cur);
		out_str(file_x, "\n");
	}
}

int main(int argc,   char *argv[])
{
	if(argc != 2) error("usage: %s filename\n", argv[0]);
	
	char *input = argv[1];
	if(input[strlen(input)-1]!='m') error("%s does not end with .m\n", input);

	if(freopen(input, "r", stdin)==NULL) error("open %s failed\n", input);

	char *output=strdup(input);

	output[strlen(output)-1]='x';
	if((file_x=fopen(output,"w"))==NULL) error("open %s failed\n", output);

	output[strlen(output)-1]='s';
	if((file_s=fopen(output,"w"))==NULL) error("open %s failed\n", output);

	tac_init();
	yyparse();
	tac_list();
	tac_obj();

	fclose(file_s);
	fclose(file_x);

	return 0;
}
