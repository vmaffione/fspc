CC=g++


fspc: lex.yy.o fsp.tab.o
	$(CC) fsp.tab.o lex.yy.o -lfl -o fspc

fsp.tab.cpp fsp.tab.hpp: fsp.ypp
	bison -d fsp.ypp

lex.yy.c: fsp.lex fsp.tab.hpp
	flex fsp.lex

clean:
	-rm *.o fspc lex.yy.c fsp.tab.cpp fsp.tab.hpp
