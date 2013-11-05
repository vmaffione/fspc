VER=1.2

# Generated C++ source files (to be updated manually)
GENERATED=parser.cpp parser.hpp scanner.cpp preproc.cpp location.hh position.hh

# Non-generated C++ source files (to be updated manually).
NONGEN=context.hpp context.cpp fspcc.cpp interface.hpp lts.cpp lts.hpp symbols_table.cpp symbols_table.hpp utils.cpp utils.hpp circular_buffer.cpp circular_buffer.hpp serializer.cpp serializer.hpp shell.cpp shell.hpp driver.cpp driver.hpp tree.cpp tree.hpp preproc.hpp helpers.cpp helpers.hpp unresolved.cpp unresolved.hpp test-serializer.cpp

# All the C++ source files.
SOURCES=$(NONGEN) $(GENERATED)

# All the non-generated files.
WCIN=$(NONGEN) fsp.lex fsp.ypp Makefile preproc.lex ltsee csee.sh parser.diff fspcc.1

# The files included in the fspc tarball.
TAR_CONTENT=$(WCIN) fsp.y

#REPORT=--report=all
REPORT=


# all should be 'complete' for distributions and 'normal' for development.
all: complete

# Build the executable using the existing Makefile.gen.
normal: always $(GENERATED)
	make -f Makefile.gen

# First regenerate Makefile.gen (which depends on all the sources, including $(GENERATED))
# and then build the executable.
# The $(GENERATED) dependency is redundant: it has been added for clarity.
complete: always $(GENERATED) Makefile.gen deps.gv
	make -f Makefile.gen

# A target that can be used as a dependency so that a rule always fires.
always:

# Generate Makefile.gen and the GraphViz representation of the include dependencies.
deps.gv Makefile.gen: $(SOURCES)
	python find_deps.py

# Generate the parser with GNU Bison.
parser.cpp parser.hpp location.hh position.hh: fsp.ypp fsp.y parser.diff
	bison $(REPORT) fsp.ypp
	patch parser.cpp < parser.diff

# Generate the scanner with Flex.
scanner.cpp: fsp.lex parser.hpp
	flex fsp.lex

# Generate the preprocessor with Flex.
preproc.cpp: preproc.lex
	flex preproc.lex

# Blackbox test against the testset.
testing: normal
	tests/test.sh

ctags: tags
	ctags -R

clean:
	-rm *.o fspcc *.out *.orig

# Also remove the generated Makefile.gen.
cleandist: clean cleanaur clc
	-rm Makefile.gen $(GENERATED)

# Total number of lines of the non-generated source files.
lines:
	wc -l $(WCIN)

# Count the number of generated and non-generated C++ source files
count:
	echo "Number of C++ source files manually included"
	echo $(GENERATED) $(NONGEN) | wc -w
	echo "Number of C++ source files (check against the previous)"
	ls *.hpp *.cpp *.hh | wc -w

aurlocal: fspcc-$(VER).tar.gz
	python create_pkgbuild.py local $(VER)

aur:
	-rm fspcc-$(VER).tar.gz
	wget "https://bitbucket.org/vmaffione/fspc/downloads/fspcc-$(VER).tar.gz"
	python create_pkgbuild.py remote $(VER)

fspcc-$(VER).tar.gz:
	tar -czf fspcc-$(VER).tar.gz $(TAR_CONTENT)

cleanaur:
	-rm *.tar.gz PKGBUILD

clc:
	-rm *.gv *.lts *.png .*.png *.bfsp *.pdf
