#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([SoarIO], [8.6.0], [soar-group@lists.sourceforge.net])
AM_INIT_AUTOMAKE([SoarIO], [8.6.0])
AC_PREREQ(2.59)
AC_REVISION($Revision$)
AC_CONFIG_SRCDIR([ConnectionSML/src/sml_AnalyzeXML.cpp])
AM_CONFIG_HEADER(config.h)
AC_PREFIX_DEFAULT(/usr/local/soar)

# Checks for programs.
AC_DISABLE_SHARED
AC_DEFINE([LINUX_STATIC_LINK], [1], [Defined when statically linking in Linux])
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h pthread.h stdlib.h string.h strings.h sys/socket.h sys/time.h sys/types.h unistd.h],,
	[AC_MSG_ERROR([*** Cannot find required header!  See previous line.])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_C_VOLATILE

# Checks for library functions.
AC_CHECK_LIB(pthread, pthread_create)
AC_CHECK_LIB(tcl, Tcl_DeleteInterp)
AC_CHECK_LIB(tk, Tk_Init)
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([getcwd gethostbyname gethostname inet_ntoa memset pow select socket sqrt strcasecmp strdup],,
	[AC_MSG_ERROR([*** Cannot find required function!  See previous line.])])

SOAR_CXXFLAGS="-Wall -g"
AC_SUBST(SOAR_CXXFLAGS)

AC_OUTPUT(Makefile
		  ConnectionSML/Makefile 
		  ConnectionSML/src/Makefile 
		  ElementXML/Makefile 
		  ElementXML/src/Makefile 
		  CommandLineInterface/Makefile 
		  CommandLineInterface/src/Makefile 
		  ClientSML/Makefile 
		  ClientSML/src/Makefile
		  KernelSML/Makefile
		  KernelSML/src/Makefile
		  examples/Makefile
		  examples/TestCommandLineInterface/Makefile
		  examples/TestCommandLineInterface/src/Makefile
		  examples/towersSML/Makefile
		  examples/towers/Makefile
		  examples/TestClientSML/Makefile
		  examples/TestConnectionSML/Makefile)
