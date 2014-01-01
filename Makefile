VER=1.5

# Generated C++ source files (to be updated manually)
GENERATED=parser.cpp parser.hpp scanner.cpp preproc.cpp location.hh position.hh

# Non-generated C++ source files (to be updated manually).
NONGEN=context.hpp context.cpp fspcc.cpp interface.hpp lts.cpp lts.hpp symbols_table.cpp symbols_table.hpp utils.cpp utils.hpp circular_buffer.cpp circular_buffer.hpp serializer.cpp serializer.hpp shell.cpp shell.hpp driver.cpp driver.hpp tree.cpp tree.hpp preproc.hpp helpers.cpp helpers.hpp unresolved.cpp unresolved.hpp test-serializer.cpp

# All the C++ source files.
SOURCES=$(NONGEN) $(GENERATED)

# All the non-generated files.
WCIN=$(NONGEN) fsp.lex fsp.ypp Makefile preproc.lex ltsee csee.sh parser.diff fspcc.1 Makefile.ske

# The files included in the fspc tarball.
TAR_CONTENT=$(SOURCES) ltsee csee.sh parser.diff fspcc.1 Makefile.gen

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

Makefile.gen: Makefile.ske

# Generate the parser with GNU Bison.
parser.cpp parser.hpp location.hh position.hh: fsp.ypp fsp.y parser.diff
	bison -p fsp $(REPORT) fsp.ypp
	patch parser.cpp < parser.diff

# Generate the scanner with Flex.
scanner.cpp: fsp.lex parser.hpp
	flex -P fsp fsp.lex

# Generate the preprocessor with Flex.
preproc.cpp: preproc.lex
	flex -P fsp preproc.lex

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

# Create a tarball containing all the files necessary for the compilation
# but not the generating files (e.g. fsp.ypp, fsp.lex, ecc.)
fspcc-$(VER).tar.gz: $(TAR_CONTENT)
	mkdir release
	cp $(TAR_CONTENT) release
	mv release/Makefile.gen release/Makefile  # rename the Makefile
	tar -czvf fspcc-$(VER).tar.gz release
	rm -rf release

cleanaur:
	-rm *.tar.gz PKGBUILD

clc:
	-rm *.gv *.lts *.png .*.png *.bfsp *.pdf
