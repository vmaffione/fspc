CC=g++
CFLAGS=-g -Wall

VER=1.0

OBJS=fspc.o scanner.o parser.o symbols_table.o lts.o context.o translator.o utils.o callbacks.o circular_buffer.o

WCIN=callbacks.?pp context.?pp fspc.cpp fsp.lex fsp.ypp input.fsp interface.hpp lts.?pp Makefile scanner.hpp symbols_table.?pp translator.?pp utils.?pp circular_buffer.?pp see.sh csee.sh
SOURCES=$(WCIN) fsp.y

#REPORT=--report=all
REPORT=

fspc: $(OBJS)
	$(CC) -g $(OBJS) -o fspc

fspc.o: symbols_table.hpp lts.cpp interface.hpp

symbols_table.o: symbols_table.hpp symbols_table.cpp

strings_set.o: strings_set.hpp strings_set.cpp

lts.o: lts.hpp symbols_table.hpp

context.o: context.hpp symbols_table.hpp

parser.o: context.hpp symbols_table.hpp parser.cpp lts.hpp utils.hpp scanner.hpp translator.hpp callbacks.o interface.hpp circular_buffer.hpp

translator.o: translator.hpp

utils.o: utils.hpp

callbacks.o: callbacks.hpp utils.hpp context.hpp translator.hpp lts.hpp

circular_buffer.o: circular_buffer.hpp

parser.cpp parser.hpp: fsp.ypp fsp.y parser.diff
	bison $(REPORT) fsp.ypp
	patch parser.cpp < parser.diff

# This rule has been made explicit only to avoid compiler warnings (-Wall)
scanner.o: scanner.cpp
	$(CC) -c scanner.cpp

scanner.cpp: fsp.lex parser.hpp
	flex fsp.lex

tags:
	cscope -R

clean:
	-rm *.o fspc scanner.cpp parser.cpp parser.hpp *.gv

testing:
	tests/test.sh

lines:
	wc -l $(WCIN)

aurlocal: fspc-$(VER).tar.gz
	python create_pkgbuild.py local $(VER)

aur:
	-rm fspc-$(VER).tar.gz
	wget "https://bitbucket.org/lisztinf/fspc/downloads/fspc-$(VER).tar.gz"
	python create_pkgbuild.py remote $(VER)

fspc-$(VER).tar.gz:
	tar -czf fspc-$(VER).tar.gz $(SOURCES)

cleanaur:
	-rm *.tar.gz PKGBUILD


