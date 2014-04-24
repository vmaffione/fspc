BUILT_SOURCES = fsp_parser.hpp sh_parser.hpp
AM_YFLAGS = -d
AM_CXXFLAGS = -std=c++11

bin_PROGRAMS = fspcc
noinst_PROGRAMS = test-serializer

modules = 	circular_buffer.cpp 		\
		code_generation_framework.cpp	\
		code_generator.cpp		\
		context.cpp 		\
		fsp_driver.cpp 		\
		helpers.cpp		\
		java_developer.cpp	\
		lts.cpp 		\
		monitor_analyst.cpp	\
		serializer.cpp 		\
		sh_driver.cpp 		\
		shell.cpp 		\
		smart_pointers.cpp 	\
		symbols_table.cpp 	\
		tree.cpp 		\
		unresolved.cpp 		\
		utils.cpp		\
		fsp_scanner.lpp		\
		fsp_parser.ypp			\
		sh_scanner.lpp			\
		sh_parser.ypp			\
		preproc.lpp

EXTRA_DIST =	circular_buffer.hpp	\
		code_generation_framework.hpp	\
		code_generator.hpp	\
		context.hpp		\
		fspc_experts.hpp	\
		fsp_driver.hpp		\
		fsplex_declaration.hpp	\
		helpers.hpp		\
		interface.hpp		\
		java_developer.hpp	\
		java_templates.hpp	\
		lts.hpp			\
		monitor_analyst.hpp	\
		preproc.hpp		\
		scalable_visitor.hpp	\
		serializer.hpp		\
		sh_driver.hpp		\
		shell.hpp		\
		shlex_declaration.hpp	\
		smart_pointers.hpp	\
		symbols_table.hpp	\
		tree.hpp		\
		unresolved.hpp		\
		utils.hpp

fspcc_SOURCES = $(modules) fspcc.cpp

test_serializer_SOURCES = $(modules) test-serializer.cpp
