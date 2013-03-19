CC=g++


fspc: lex.yy.o fsp.tab.o
	$(CC) fsp.tab.o lex.yy.o -lfl -o fspc

fsp.tab.c fsp.tab.h: fsp.y
	bison -d fsp.y

lex.yy.c: fsp.lex fsp.tab.h
	flex fsp.lex

clean:
	-rm *.o fspc lex.yy.c fsp.tab.c fsp.tab.h
