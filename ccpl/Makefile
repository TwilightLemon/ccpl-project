all:
	lex -o build/lex.yy.cc src/lexer.l
	bison -d -o build/parser.tab.cc src/parser.y
	g++ -std=c++17 -I build -I src -o build/ccpl build/parser.tab.cc build/lex.yy.cc src/main.cc src/modules/tac.cc src/modules/opt.cc src/modules/block.cc

run: ./test/$(test).m
	./build/ccpl $< > output/$(test).out

clean:
	rm -f build/*