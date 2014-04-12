VER=1.7

# Generated C++ source files (to be updated manually)
GENERATED=fsp_parser.cpp fsp_parser.hpp fsp_scanner.cpp preproc.cpp fsp_location.hh fsp_position.hh sh_parser.cpp sh_parser.hpp sh_scanner.cpp

# Non-generated C++ source files (to be updated manually).
NONGEN=context.hpp context.cpp fspcc.cpp interface.hpp lts.cpp lts.hpp symbols_table.cpp symbols_table.hpp utils.cpp utils.hpp circular_buffer.cpp circular_buffer.hpp serializer.cpp serializer.hpp shell.cpp shell.hpp fsp_driver.cpp fsp_driver.hpp tree.cpp tree.hpp preproc.hpp helpers.cpp helpers.hpp unresolved.cpp unresolved.hpp test-serializer.cpp smart_pointers.hpp smart_pointers.cpp code_generator.cpp code_generator.hpp shlex_declaration.hpp fsplex_declaration.hpp java.hpp sh_driver.cpp sh_driver.hpp

# All the C++ source files.
SOURCES=$(NONGEN) $(GENERATED)

# All the non-generated files.
WCIN=$(NONGEN) fsp.lex fsp.ypp Makefile preproc.lex ltsee csee.sh fsp_parser.diff fspcc.1 Makefile.ske sh.lex sh.ypp

# The files included in the fspc tarball.
TAR_CONTENT=$(SOURCES) ltsee csee.sh fsp_parser.diff fspcc.1 Makefile.gen

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

# Generate the fsp_parser with GNU Bison and apply a simple patch.
# Unfortunately I couldn't find a way to tell Bison to generate
# the 'location.hh' and 'position.hh' files into files with different
# names: For this reason it's necessary to manually rename the files
# and replace include directives.
fsp_parser.cpp fsp_parser.hpp fsp_location.hh fsp_position.hh: fsp.ypp fsp.y fsp_parser.diff
	bison -p fsp $(REPORT) fsp.ypp
	patch fsp_parser.cpp < fsp_parser.diff
	mv location.hh fsp_location.hh
	mv position.hh fsp_position.hh
	sed -i 's|\<location.hh|fsp_location.hh|g' *.hpp *.cpp *.hh
	sed -i 's|\<position.hh|fsp_position.hh|g' *.hpp *.cpp *.hh

# Generate the fsp_scanner with Flex.
fsp_scanner.cpp: fsp.lex fsp_parser.hpp
	flex -P fsp fsp.lex

# Generate the preprocessor with Flex.
preproc.cpp: preproc.lex
	flex -P fsp preproc.lex

# Generate the shell fsp_parser with GNU Bison.
sh_parser.cpp sh_parser.hpp: sh.ypp sh.ypp
	bison -p sh $(REPORT) sh.ypp

# Generate the shell fsp_scanner with Flex.
sh_scanner.cpp: sh.lex sh_parser.hpp
	flex -P sh sh.lex

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
	-rm *.gv *.lts *.png .*.png *.bfsp *.pdf $(GENERATED)
