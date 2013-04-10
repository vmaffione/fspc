CC=g++
CFLAGS=-g -Wall


fspc: fspc.o lex.yy.o fsp.tab.o symbols_table.o lts.o context.o
	$(CC) -g fspc.o fsp.tab.o lex.yy.o symbols_table.o lts.o context.o -lfl -o fspc

fspc.o: symbols_table.hpp parser.hpp lts.cpp

symbols_table.o: symbols_table.hpp symbols_table.cpp

strings_set.o: strings_set.hpp strings_set.cpp

context.o: context.hpp symbols_table.hpp

fsp.tab.o: context.hpp symbols_table.hpp fsp.tab.cpp lts.hpp utils.hpp

fsp.tab.cpp fsp.tab.hpp: fsp.ypp fsp.y
	bison fsp.ypp

# This rule has been made explicit only to avoid compiler warnings (-Wall)
lex.yy.o: lex.yy.c
	$(CC) -c lex.yy.c

lex.yy.c: fsp.lex fsp.tab.hpp
	flex fsp.lex

clean:
	-rm *.o fspc lex.yy.c fsp.tab.cpp fsp.tab.hpp
