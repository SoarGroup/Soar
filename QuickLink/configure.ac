###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################

AC_PREREQ(2.57)
AC_INIT([QuickLink], [8.6.2], [soar-group@lists.sourceforge.net])
AC_CONFIG_SRCDIR([src/QL_Controller.cpp])
AM_INIT_AUTOMAKE([foreign tar-ustar])
AC_REVISION($Revision: 5272 $)
AM_CONFIG_HEADER([config.h])
AC_PREFIX_DEFAULT([/usr/local/soar])
AC_LIBTOOL_DLOPEN

# Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_CXX

# check if the compiler support -fvisibility=hidden (GCC >= 4)
saved_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -fvisibility=hidden"
AC_MSG_CHECKING([for -fvisibility=hidden])
AC_COMPILE_IFELSE([char foo;],
    [	AC_MSG_RESULT([yes])
	SYMBOL_VISIBILITY="-fvisibility=hidden" 
	visibility=true],
    [	AC_MSG_RESULT([no])
	visibility=false])
CFLAGS="$saved_CFLAGS"
AC_SUBST(SYMBOL_VISIBILITY)
AM_CONDITIONAL(VISIBILITY_AVAILABLE, test x$visibility = xtrue)
# end check for visibility compiler support

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])

AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK

SOAR_CXXFLAGS="-Wall -g -O2 -fno-strict-aliasing $SYMBOL_VISIBILITY"
AC_SUBST(SOAR_CXXFLAGS)

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
