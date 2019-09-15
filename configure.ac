AC_PREREQ([2.69])

AC_INIT([mypdfsearch], [1.5.0], [vincenzolocicero68@gmail.com])

AC_CONFIG_AUX_DIR([build-aux])

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SRCDIR([src/main.c])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile src/Makefile])


AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

# Impostiamo qui i flag del compilatore, tramite CFLAGS chiamato dopo AC_INIT e prima di AC_PROG_CC.
# Altrimenti, se non usiamo qui CFLAGS e usiamo mypdfsearch_CFLAGS su src/Makefile.am,
# autotools aggiunge i flags '-g -O2' dopo quelli nostri(vanificandoli):

#CFLAGS="-Wall -W -pedantic -O3 -Wno-overlength-strings"
AC_SUBST([CFLAGS], ["-Wall -W -pedantic -O3 -Wno-overlength-strings"])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AM_PROG_CC_C_O

# Checks for libraries.
AC_SEARCH_LIBS([iconv_open], [iconv])
AC_SEARCH_LIBS([inflate], [z])
AC_SEARCH_LIBS([ceil], [m])
#AC_SEARCH_LIBS([pthread_create], [pthread])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_OUTPUT
