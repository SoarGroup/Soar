#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([gSKI], [8.6.0], [soar-group@lists.sourceforge.net])
AM_INIT_AUTOMAKE([gSKI], [8.6.0])
AC_PREREQ(2.57)
AC_REVISION($Revision$)
AC_CONFIG_SRCDIR([src/gSKI.cpp])
AM_CONFIG_HEADER(config.h)
AC_PREFIX_DEFAULT(/usr/local/soar)

# Checks for programs.
AC_DISABLE_SHARED
AC_DEFINE([NO_MEGA_UNIT_TESTS], [1], [Defined when not using mega unit tests.])
AC_DEFINE([VALIDATE], [assert], [BUGBUG: not sure where this symbol is defined.])
AC_DEFINE([UNIX], [assert], [BUGBUG: Defined for now, needs to be removed.])
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_HEADER_STDBOOL
AC_HEADER_TIME
AC_CHECK_HEADERS([fcntl.h limits.h locale.h stddef.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_STAT
AC_CHECK_FUNCS([gethostname memmove memset mkdir regcomp rmdir setlocale strchr strerror])

SOAR_CXXFLAGS="-Wall -g"
AC_SUBST(SOAR_CXXFLAGS)

AC_OUTPUT(Makefile src/Makefile)
