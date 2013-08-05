CC=g++
DEBUG=-g
CFLAGS=$(DEBUG) -Wall
CXXFLAGS=$(DEBUG) -Wall

VER=1.2

OBJS=fspcc.o scanner.o parser.o symbols_table.o lts.o context.o utils.o callbacks.o circular_buffer.o serializer.o shell.o driver.o preproc.o

HDRS=callbacks.hpp circular_buffer.hpp context.hpp driver.hpp interface.hpp lts.hpp preproc.hpp serializer.hpp shell.hpp symbols_table.hpp utils.hpp parser.hpp

WCIN=callbacks.?pp context.?pp fspcc.cpp fsp.lex fsp.ypp input.fsp interface.hpp lts.?pp Makefile symbols_table.?pp utils.?pp circular_buffer.?pp serializer.?pp shell.?pp driver.?pp preproc.hpp preproc.lex ltsee csee.sh parser.diff
SOURCES=$(WCIN) fspcc.1 fsp.y

#REPORT=--report=all
REPORT=

all: fspcc ctags

fspcc: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o fspcc -lncurses

$(OBJS): $(HDRS)

parser.cpp parser.hpp: fsp.ypp fsp.y parser.diff
	bison $(REPORT) fsp.ypp
	patch parser.cpp < parser.diff

# This rule has been made explicit only to avoid compiler warnings (-Wall)
scanner.o: scanner.cpp
	$(CC) $(DEBUG) -c scanner.cpp

scanner.cpp: fsp.lex parser.hpp
	flex fsp.lex

preproc.o: preproc.cpp
	$(CC) $(DEBUG) -c preproc.cpp

preproc.cpp: preproc.lex
	flex preproc.lex

ctags: tags
	ctags -R

clean: cleanaur clc
	-rm *.o fspcc scanner.cpp parser.cpp parser.hpp *.out preproc.cpp location.hh position.hh stack.hh *.orig

testing: fspcc
	tests/test.sh

lines:
	wc -l $(WCIN)

aurlocal: fspcc-$(VER).tar.gz
	python create_pkgbuild.py local $(VER)

aur:
	-rm fspcc-$(VER).tar.gz
	wget "https://bitbucket.org/vmaffione/fspc/downloads/fspcc-$(VER).tar.gz"
	python create_pkgbuild.py remote $(VER)

fspcc-$(VER).tar.gz:
	tar -czf fspcc-$(VER).tar.gz $(SOURCES)

cleanaur:
	-rm *.tar.gz PKGBUILD

clc:
	-rm *.gv *.lts *.png .*.png *.bfsp *.pdf
