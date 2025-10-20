#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inst.h"

#define REGMAX 16
#define MEMMAX (256 * 256)
#define R_FLAG 0
#define R_IP 1
#define FLAG_EZ 0
#define FLAG_LZ 1
#define FLAG_GZ 2

int reg[REGMAX]; /* registers */
unsigned char mem[MEMMAX]; /* memory */
int op, rx, ry, constant; /* opcode, register1, register2, immediate constant */
int cycle, mem_r, mem_w, mul_div;

// get instruction from addr
void instruction(int addr)
{
	op = (*(int*)&(mem[addr])) & 0xffff; // 16 bit
	rx = mem[addr+2]; // 8 bit
	ry = mem[addr+3]; // 8 bit
	constant = *(int*)&(mem[addr+4]); // 32 bit
}

int main(int argc, char *argv[])
{
	if(argc!=2) {
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		exit(0);		
	}

	FILE * input=fopen( argv[1], "rb" );
	if( input ==  NULL )
	{
		fprintf(stderr, "error: open %s failed\n", argv[1] );
		exit(0);
	}

	int i, ch, t, t1, t2;

	/* init reg */
	for( i=0; i < REGMAX; i++ ) 
		reg[i]=0;

	/* init mem */
	for( i=0; (ch=fgetc(input)) != EOF; i++ ) 
		mem[i]=(char)ch;
	for( ; i < MEMMAX; i++ ) 
		mem[i]=0;

	/* run machine */
	cycle = mem_r = mem_w = 0;
	for(;;)
	{
		cycle++;
		instruction(reg[R_IP]);
		
		switch(op)
		{
			case I_END:
			printf("\n");
			printf("------------------------------\n");
			printf("CLOCK CYCLES : %d\n", cycle);
			printf("MUL DIV : %d\n", mul_div);
			printf("MEM READ : %d\n", mem_r);
			printf("MEM WRITE : %d\n", mem_w);
			printf("------------------------------\n");
			exit(0);

			case I_NOP:
			break;

			case I_OTC:
			printf( "%c", reg[15] ); /* Print out reg[15] in ASCII */
			break;

			case I_OTI:
			printf( "%d", reg[15] ); /* Print int in reg[15] */
			break;

			case I_OTS:
			printf( "%s", &mem[reg[15]] ); /* Print string pointed by reg[15] */
			break;

			case I_ITC:
			/* Input char to reg[15] */
			reg[15] = ' ';
			while(reg[15]==' ' || reg[15]=='\t' || reg[15]=='\r' || reg[15]=='\n')
			{
				if(scanf("%c", (char*)&reg[15]) != 1)
				{
					if(getchar() == EOF) break;
				}

			}
			break;

			case I_ITI:
			/* Input int to reg[15] */
			reg[15] = 0;
			while(scanf("%d", &reg[15]) != 1)
			{
				if(getchar() == EOF) break;
			}
			break;

			case I_ADD_0:
			reg[rx]=reg[rx] + constant;
			break;

			case I_ADD_1:
			reg[rx]=reg[rx] + reg[ry];
			break;

			case I_SUB_0:
			reg[rx]=reg[rx] - constant;
			break;

			case I_SUB_1:
			reg[rx]=reg[rx] - reg[ry];
			break;

			case I_MUL_0:
			cycle += 4;
			mul_div++;
			reg[rx]=reg[rx] * constant;
			break;

			case I_MUL_1:
			cycle += 4;
			mul_div++;
			reg[rx]=reg[rx] * reg[ry];
			break;

			case I_DIV_0:
			cycle += 4;
			mul_div++;
			if( constant == 0 ) 
			{
				fprintf(stderr, "error: divide by zero\n");
				exit(0);
			} 
			else 
			{
				reg[rx]=reg[rx] / constant;
			}			
			break;

			case I_DIV_1:
			cycle += 4;
			mul_div++;
			if( reg[ry] == 0 ) 
			{
				fprintf(stderr, "error: divide by zero\n");
				exit(0);
			} 
			else 
			{
				reg[rx]=reg[rx] / reg[ry];
			}			
			break;

			case I_LOD_0:
			reg[rx]=constant;
			break;

			case I_LOD_1:
			reg[rx]=reg[ry];
			break;

			case I_LOD_2:
			reg[rx]=reg[ry] + constant;
			break;

			case I_LOD_3:
			cycle += 9;
			mem_r++;
			reg[rx] = *(int*)&(mem[constant]);
			break;

			case I_LDC_3:
			cycle += 9;
			mem_r++;
			reg[rx] = mem[constant];
			break;

			case I_LOD_4:
			cycle += 9;
			mem_r++;
			reg[rx] = *(int*)&(mem[reg[ry]]);
			break;

			case I_LDC_4:
			cycle += 9;
			mem_r++;
			reg[rx] = mem[reg[ry]];
			break;

			case I_LOD_5:
			cycle += 9;
			mem_r++;
			reg[rx] = *(int*)&(mem[reg[ry] + constant]);
			break;

			case I_LDC_5:
			cycle += 9;
			mem_r++;
			reg[rx] = mem[reg[ry] + constant];
			break;

			case I_STO_0:
			cycle += 9;
			mem_w++;
			*(int*)&(mem[reg[rx]]) = constant;
			break;

			case I_STC_0:
			cycle += 9;
			mem_w++;
			mem[reg[rx]] = constant;
			break;

			case I_STO_1:
			cycle += 9;
			mem_w++;
			*(int*)&(mem[reg[rx]]) = reg[ry];
			break;

			case I_STC_1:
			cycle += 9;
			mem_w++;
			mem[reg[rx]] = reg[ry];
			break;

			case I_STO_2:
			cycle += 9;
			mem_w++;
			*(int*)&(mem[reg[rx]]) = reg[ry]+constant;
			break;

			case I_STC_2:
			cycle += 9;
			mem_w++;
			mem[reg[rx]] = reg[ry]+constant;
			break;

			case I_STO_3:
			cycle += 9;
			mem_w++;
			*(int*)&(mem[reg[rx] + constant]) = reg[ry];
			break;

			case I_STC_3:
			cycle += 9;
			mem_w++;
			mem[reg[rx] + constant] = reg[ry];
			break;

			case I_TST_0:
			t=reg[rx];
			if(t==0) reg[R_FLAG]=FLAG_EZ;
			else if(t<0) reg[R_FLAG]=FLAG_LZ;
			else if(t>0) reg[R_FLAG]=FLAG_GZ;
			break;

			case I_JMP_0:
			reg[R_IP]=constant;
			continue;

			case I_JMP_1:
			reg[R_IP]=reg[rx];
			continue;

			case I_JEZ_0:
			if(reg[R_FLAG]==FLAG_EZ) { reg[R_IP]=constant; continue; }
			else break;
			
			case I_JEZ_1:
			if(reg[R_FLAG]==FLAG_EZ) { reg[R_IP]=reg[rx]; continue; }
			else break;

			case I_JLZ_0:
			if(reg[R_FLAG]==FLAG_LZ) { reg[R_IP]=constant; continue; }
			else break;

			case I_JLZ_1:
			if(reg[R_FLAG]==FLAG_LZ) { reg[R_IP]=reg[rx]; continue; }
			else break;

			case I_JGZ_0:
			if(reg[R_FLAG]==FLAG_GZ) { reg[R_IP]=constant; continue; }
			else break;

			case I_JGZ_1:
			if(reg[R_FLAG]==FLAG_GZ) { reg[R_IP]=reg[rx]; continue; }
			else break;

			default:
			fprintf(stderr, "error: invalid opcode %02x\n", op);
			exit(0);
		}
		
		reg[R_IP]=reg[R_IP]+8; /* next instruction */
	}
	
	return 0;
}


