CC=g++
CFLAGS=-g -Wall


fspc: fspc.o lex.yy.o fsp.tab.o strings_table.o lts.o
	$(CC) fspc.o fsp.tab.o lex.yy.o strings_table.o lts.o -lfl -o fspc

fspc.o: parser.hpp strings_table.hpp

fsp.tab.cpp fsp.tab.hpp: fsp.ypp
	bison -d fsp.ypp

lex.yy.c: fsp.lex fsp.tab.hpp
	flex fsp.lex

clean:
	-rm *.o fspc lex.yy.c fsp.tab.cpp fsp.tab.hpp
