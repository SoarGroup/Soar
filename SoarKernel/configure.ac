#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([SoarKernel], [8.6.0])
AM_INIT_AUTOMAKE([SoarKernel], [8.6.0])
AC_CONFIG_SRCDIR([src/soar.h])
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_DEFINE([_DEBUG], [1], [BUGBUG: Defined for now, needs to be removed.])
AC_DEFINE([_USE_TCL], [1], [BUGBUG: Defined for now, needs to be removed.])
AC_DEFINE([UNIX], [1], [BUGBUG: Defined for now, needs to be removed.])
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([malloc.h memory.h stddef.h stdlib.h string.h sys/time.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_PATH_X
AC_STRUCT_TM
AC_TYPE_SIGNAL

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([endpwent getcwd gethostname gettimeofday memset modf sqrt])

AC_OUTPUT(Makefile src/Makefile)
