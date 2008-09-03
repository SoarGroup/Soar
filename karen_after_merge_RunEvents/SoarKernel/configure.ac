###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2005
###########################################
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([SoarKernel], [8.6.2], [soar-group@lists.sourceforge.net])
AC_CONFIG_SRCDIR([include/soarkernel.h])
AM_INIT_AUTOMAKE([foreign])
AC_REVISION($Revision$)
AM_CONFIG_HEADER(config.h)
AC_PREFIX_DEFAULT(/usr/local/soar)

# Checks for programs.
AC_DISABLE_SHARED
#AC_DEFINE([_DEBUG], [1], [BUGBUG: Defined for now, needs to be removed.])
#AC_DEFINE([USE_TCL], [1], [BUGBUG: Defined for now, needs to be removed.])
#AC_DEFINE([UNIX], [1], [BUGBUG: Defined for now, needs to be removed.])
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_PATH_X
AC_HEADER_STDC
# fail for these
AC_CHECK_HEADERS([ctype.h math.h signal.h],,[AC_MSG_ERROR([*** Cannot find required header!  See previous line.])])
# work around these
AC_CHECK_HEADERS([assert.h errno.h malloc.h memory.h stddef.h stdlib.h string.h sys/param.h sys/resource.h sys/time.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([endpwent getcwd gethostname gettimeofday memset modf sqrt strchr strtol])

SOAR_CXXFLAGS="-Wall -g"
AC_SUBST(SOAR_CXXFLAGS)
AC_OUTPUT(Makefile src/Makefile)
