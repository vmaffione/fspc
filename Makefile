CC=g++
CFLAGS=-g -Wall


fspc: fspc.o lex.yy.o fsp.tab.o strings_table.o strings_set.o lts.o
	$(CC) fspc.o fsp.tab.o lex.yy.o strings_table.o lts.o strings_set.o -lfl -o fspc

fspc.o: strings_table.hpp parser.hpp lts.cpp

strings_table.o: strings_table.hpp

strings_set.o: strings_set.hpp

fsp.tab.o: strings_set.hpp strings_table.hpp

fsp.tab.cpp fsp.tab.hpp: fsp.ypp
	bison -d fsp.ypp

lex.yy.c: fsp.lex fsp.tab.hpp
	flex fsp.lex

clean:
	-rm *.o fspc lex.yy.c fsp.tab.cpp fsp.tab.hpp
