all: asm machine

asm: asm.l asm.y inst.h
	lex -o build/asm.l.c asm.l
	yacc -d -o build/asm.y.c asm.y
	gcc -g3 -I. build/asm.l.c build/asm.y.c -o build/asm

machine: machine.c inst.h
	gcc -g3 machine.c -o build/machine
