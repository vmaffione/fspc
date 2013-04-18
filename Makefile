CC=g++
CFLAGS=-g -Wall

OBJS=fspc.o scanner.o parser.o symbols_table.o lts.o context.o translator.o utils.o callbacks.o

fspc: $(OBJS)
	$(CC) -g $(OBJS) -o fspc

fspc.o: symbols_table.hpp lts.cpp

symbols_table.o: symbols_table.hpp symbols_table.cpp

strings_set.o: strings_set.hpp strings_set.cpp

context.o: context.hpp symbols_table.hpp

parser.o: context.hpp symbols_table.hpp parser.cpp lts.hpp utils.hpp scanner.hpp translator.hpp callbacks.o

translator.o: translator.hpp

utils.o: utils.hpp

callbacks.o: callbacks.hpp utils.hpp context.hpp translator.hpp lts.hpp

parser.cpp parser.hpp: fsp.ypp fsp.y
	bison fsp.ypp

# This rule has been made explicit only to avoid compiler warnings (-Wall)
scanner.o: scanner.cpp
	$(CC) -c scanner.cpp

scanner.cpp: fsp.lex parser.hpp
	flex fsp.lex

clean:
	-rm *.o fspc scanner.cpp parser.cpp parser.hpp

tags:
	cscope -R

lines:
	wc -l callbacks.?pp context.?pp fspc.cpp fsp.lex fsp.y lts.?pp Makefile scanner.hpp symbols_table.?pp translator.?pp utils.?pp see.sh
