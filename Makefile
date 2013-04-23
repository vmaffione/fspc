CC=g++
CFLAGS=-g -Wall

COD=f125a32166dc

OBJS=fspc.o scanner.o parser.o symbols_table.o lts.o context.o translator.o utils.o callbacks.o

WCIN=callbacks.?pp context.?pp fspc.cpp fsp.lex fsp.ypp input.fsp interface.hpp lts.?pp Makefile scanner.hpp symbols_table.?pp translator.?pp utils.?pp see.sh csee.sh
SOURCES=$(WCIN) fsp.y

fspc: $(OBJS)
	$(CC) -g $(OBJS) -o fspc

fspc.o: symbols_table.hpp lts.cpp interface.hpp

symbols_table.o: symbols_table.hpp symbols_table.cpp

strings_set.o: strings_set.hpp strings_set.cpp

lts.o: lts.hpp symbols_table.hpp

context.o: context.hpp symbols_table.hpp

parser.o: context.hpp symbols_table.hpp parser.cpp lts.hpp utils.hpp scanner.hpp translator.hpp callbacks.o interface.hpp

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

tags:
	cscope -R

lines:
	wc -l $(WCIN)

aurlocal:
	tar -czf fspc-1.0.tar.gz $(SOURCES)
	python create_pkgbuild.py local

aur: $(COD).zip
	python create_pkgbuild.py $(COD)

$(COD).zip:
	wget https://bitbucket.org/lisztinf/fspc/get/$(COD).zip 


cleanaur:
	-rm *.tar.gz PKGBUILD *.zip

clean:
	-rm *.o fspc scanner.cpp parser.cpp parser.hpp

