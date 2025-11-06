all: asm-machine/build/asm asm-machine/build/machine ccpl/build/ccpl
	./ccpl/build/ccpl ccpl/test/${test}.m ${test}.s; \
	./asm-machine/build/asm ${test}.s; \
	./asm-machine/build/machine ${test}.o

test-all: asm-machine/build/asm asm-machine/build/machine ccpl/build/ccpl
	@echo "Testing all files in ccpl/test directory..."; \
	for test_file in ccpl/test/*.m; do \
		test_name=$$(basename $$test_file .m); \
		echo "\n========================================"; \
		echo "Testing: $$test_name"; \
		echo "========================================"; \
		./ccpl/build/ccpl $$test_file $$test_name.s && \
		./asm-machine/build/asm $$test_name.s && \
		./asm-machine/build/machine $$test_name.o; \
		if [ $$? -eq 0 ]; then \
			echo "✓ $$test_name passed"; \
		else \
			echo "✗ $$test_name failed"; \
		fi; \
	done; \
	echo "\n========================================"; \
	echo "All tests completed"; \
	echo "========================================"

clean:
	rm -rf *.s *.o