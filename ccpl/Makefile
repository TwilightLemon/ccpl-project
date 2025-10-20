all:
	lex -o build/lex.yy.cc src/lexer.l
	bison -d -o build/parser.tab.cc src/parser.y
	g++ -std=c++11 -I build -o build/ccpl build/parser.tab.cc build/lex.yy.cc src/main.cc

run: ./test/$(test).m
	./build/ccpl $<

clean:
	rm -f build/*