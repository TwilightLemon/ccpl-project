all: asm-machine/build/asm asm-machine/build/machine ccpl/build/ccpl
	./ccpl/build/ccpl ccpl/test/${test}.m ${test}.s; \
	./asm-machine/build/asm ${test}.s; \
	./asm-machine/build/machine ${test}.o

clean:
	rm -rf *.s *.o