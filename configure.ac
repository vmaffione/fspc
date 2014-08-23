AC_INIT([fspc], [1.8], [v.maffione@gmail.com])
AM_INIT_AUTOMAKE([foreign -Wall])

AC_CONFIG_SRCDIR([src/lts.cpp])
AC_PROG_CXX
AC_PROG_LEX
AC_PROG_YACC

AC_CHECK_LIB([ncurses], [printw])
AC_CHECK_HEADER([ncurses.h])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
