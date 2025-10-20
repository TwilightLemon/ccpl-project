asm: asm.l asm.y inst.h
	lex -o asm.l.c asm.l
	yacc -d -o asm.y.c asm.y
	gcc -g3 asm.l.c asm.y.c -o asm

machine: machine.c inst.h
	gcc -g3 machine.c -o machine