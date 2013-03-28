CC=g++
CFLAGS=-g -Wall


fspc: fspc.o lex.yy.o fsp.tab.o strings_table.o lts.o context.o
	$(CC) fspc.o fsp.tab.o lex.yy.o strings_table.o lts.o context.o -lfl -o fspc

fspc.o: strings_table.hpp parser.hpp lts.cpp

strings_table.o: strings_table.hpp strings_table.cpp

strings_set.o: strings_set.hpp strings_set.cpp

context.o: context.hpp strings_table.hpp

fsp.tab.o: context.hpp strings_table.hpp fsp.tab.cpp lts.hpp 

fsp.tab.cpp fsp.tab.hpp: fsp.ypp fsp.y
	bison -d fsp.ypp

# This rule has been made explicit only to avoid compiler warnings (-Wall)
lex.yy.o: lex.yy.c
	$(CC) -c lex.yy.c

lex.yy.c: fsp.lex fsp.tab.hpp
	flex fsp.lex

clean:
	-rm *.o fspc lex.yy.c fsp.tab.cpp fsp.tab.hpp
