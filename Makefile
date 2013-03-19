

snazzle: lex.yy.c
	g++ lex.yy.c -lfl -o snazzle

lex.yy.c: snazzle.lex
	flex snazzle.lex

clean:
	-rm *.o snazzle
	rm lex.yy.c
