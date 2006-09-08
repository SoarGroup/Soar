###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################

AC_PREREQ(2.57)
AC_INIT([SoarSuite], [8.6.2-r5], [soar-group@lists.sourceforge.net])
AC_CONFIG_HEADERS([Core/shared/config.h])
AC_CONFIG_SRCDIR([Core/SoarKernel/include/soarkernel.h])
AM_INIT_AUTOMAKE([foreign tar-ustar])
AC_REVISION($Revision$)
AC_PREFIX_DEFAULT([/usr/local/soar])
AC_LIBTOOL_DLOPEN

# Checks for programs.
AC_DEFINE([NO_MEGA_UNIT_TESTS], [1], [gSKI: Defined when not using mega unit tests.])
AC_DEFINE([VALIDATE], [assert], [BUGBUG: gSKI: not sure where this symbol is defined.])
AC_DEFINE([UNIX], [assert], [BUGBUG: gSKI: Defined for now, needs to be removed.])
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

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

# Tcl and Java switches for SWIG
AC_ARG_ENABLE(tcl,
     [  --enable-tcl        Turn on tcl support (needs swig >= 1.3.24 but not 1.3.28 and Tcl 8.4)],
     [case "${enableval}" in
	yes) tcl=true ;;
	no)  tcl=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-tcl) ;;
     esac],
     [tcl=false])
AM_CONDITIONAL(SWIGTCL, test x$tcl = xtrue)
if test "$tcl" = "true"
then
  if test "$enable_shared" = "no"
  then
    AC_MSG_ERROR([*** Shared libraries must be enabled for Tcl support!])
  fi
  AC_PROG_SWIG(1.3.24)
  SWIG_ENABLE_CXX
  AC_CHECK_HEADER([tcl.h],,[AC_MSG_ERROR([*** Cannot find required header!  See previous line. Debian users: please see the following URL for a workaround: https://winter.eecs.umich.edu/soar-bugzilla/show_bug.cgi?id=543])])
  AC_PATH_PROG(TCLSH,tclsh,[AC_MSG_ERROR([*** Cannot find required program!  See previous line.])])
fi

AC_ARG_ENABLE(java,
     [  --disable-java        Turn off java support (on by default).],
     [case "${enableval}" in
	yes) java=true ;;
	no)  java=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --disable-java) ;;
     esac],
     [java=true])
AM_CONDITIONAL(SWIGJAVA, test x$java = xtrue)
if test "$java" = "true"
then
  if test "$enable_shared" = "no"
  then
    AC_MSG_ERROR([*** Shared libraries must be enabled for Java support! Try adding --disable-java if this is ok.])
  fi
  AC_PROG_SWIG(1.3.24)
  SWIG_ENABLE_CXX
fi

AC_ARG_ENABLE(python,
     [  --enable-python     Turn on python support],
     [case "${enableval}" in
        yes) python=true ;;
        no)  python=false ;;
        *) AC_MSG_ERROR(bad value ${enableval} for --enable-python) ;;
     esac],
     [python=false])
AM_CONDITIONAL(SWIGPYTHON, test x$python = xtrue)
if test "$python" = "true"
then
  if test "$enable_shared" = "no"
  then
    AC_MSG_ERROR([*** Shared libraries must be enabled for python support!])
  fi
  AC_PROG_SWIG(1.3.24)
  AM_PATH_PYTHON
  SWIG_ENABLE_CXX
  SWIG_PYTHON
fi

# See comments in Core/ClientSML/src/sml_ClientKernel.cpp
if test "$enable_shared" = "no"
then
  AC_DEFINE([STATIC_LINKED], [1], [Defined when statically linking])
fi
AM_CONDITIONAL([STATIC_LINKED_CONDITIONAL], test x$enable_shared = xno)

# Checks for libraries.

# Checks for header files.
AC_PATH_X
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_HEADER_STDBOOL
AC_HEADER_TIME
# fail for these
AC_CHECK_HEADERS([arpa/inet.h dlfcn.h ctype.h fcntl.h limits.h locale.h math.h netdb.h netinet/in.h pthread.h signal.h sys/socket.h sys/types.h],,[AC_MSG_ERROR([*** Cannot find required header!  See previous line.])])
# work around these
AC_CHECK_HEADERS([assert.h errno.h malloc.h memory.h stddef.h stdlib.h string.h strings.h sys/param.h sys/resource.h sys/time.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_CHECK_LIB(pthread, pthread_create)
AC_CHECK_LIB(dl, dlopen)
AC_CHECK_LIB(m, sqrt)
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MEMCMP
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([endpwent getcwd gethostname gettimeofday inet_ntoa memmove memset mkdir modf regcomp rmdir select setlocale socket sqrt strcasecmp strchr strerror strtol strstr],,[AC_MSG_ERROR([*** Cannot find required function!  See previous line.])])

SOAR_CXXFLAGS="-Wall -g -O2 $SYMBOL_VISIBILITY"
AC_SUBST(SOAR_CXXFLAGS)

####################
# SWIG STUFF

# Set info about shared libraries.
AC_SUBST(SO)
AC_SUBST(LDSHARED)
AC_SUBST(CCSHARED)
AC_SUBST(CXXSHARED)
AC_SUBST(TRYLINKINGWITHCXX)
AC_SUBST(LINKFORSHARED)

# SO is the extension of shared libraries `(including the dot!)
AC_MSG_CHECKING(SO)
if test -z "$SO"
then
	case $host in
	*-*-hp*) SO=.sl;;
	*-*-darwin*) SO=.dylib;;
	*-*-cygwin* | *-*-mingw*) SO=.dll;;
	*) SO=.so;;
	esac
fi
AC_MSG_RESULT($SO)

# LDSHARED is the ld *command* used to create shared library
# -- "ld" on SunOS 4.x.x, "ld -G" on SunOS 5.x, "ld -shared" on IRIX 5
# (Shared libraries in this instance are shared modules to be loaded into
# Python, as opposed to building Python itself as a shared library.)
AC_MSG_CHECKING(LDSHARED)
if test -z "$LDSHARED"
then
	case $host in
	*-*-aix*) LDSHARED="\$(srcdir)/ld_so_aix \$(CC)";;
	*-*-cygwin* | *-*-mingw*)
            if test "$GCC" = yes; then
                LDSHARED="$CC -shared"
            else
                if test "cl" = $CC ;  then
                    # Microsoft Visual C++ (MSVC)
                    LDSHARED="$CC -nologo -LD"
                else
                    # Unknown compiler try gcc approach
                    LDSHARED="$CC -shared"
                fi
            fi ;;
	*-*-irix5*) LDSHARED="ld -shared";;
	*-*-irix6*) LDSHARED="ld ${SGI_ABI} -shared -all";;
	*-*-sunos4*) LDSHARED="ld";;
	*-*-solaris*) LDSHARED="ld -G";;
	*-*-hp*) LDSHARED="ld -b";;
	*-*-osf*) LDSHARED="ld -shared -expect_unresolved \"*\"";;
	*-sequent-sysv4) LDSHARED="ld -G";;
	*-*-next*)
		if test "$ns_dyld"
		then LDSHARED='$(CC) $(LDFLAGS) -bundle -prebind'
		else LDSHARED='$(CC) $(CFLAGS) -nostdlib -r';
		fi
                if test "$with_next_framework" ; then
		    LDSHARED="$LDSHARED \$(LDLIBRARY)"
		fi ;;
	*-*-linux*) LDSHARED="gcc -shared";;
	*-*-dgux*) LDSHARED="ld -G";;
	*-*-freebsd3*) LDSHARED="gcc -shared";;
	*-*-freebsd* | *-*-openbsd*) LDSHARED="ld -Bshareable";;
	*-*-netbsd*)
		if [[ "`$CC -dM -E - </dev/null | grep __ELF__`" != "" ]]
		then
			LDSHARED="cc -shared"
		else
			LDSHARED="ld -Bshareable"
		fi;;
	*-sco-sysv*) LDSHARED="cc -G -KPIC -Ki486 -belf -Wl,-Bexport";;
	*-*-darwin*) LDSHARED="cc -bundle -undefined suppress -flat_namespace";;
	*)	LDSHARED="ld";;
	esac
fi
AC_MSG_RESULT($LDSHARED)
# CXXSHARED is the ld *command* used to create C++ shared library
# -- "ld" on SunOS 4.x.x, "ld -G" on SunOS 5.x, "ld -shared" on IRIX 5
# (Shared libraries in this instance are shared modules to be loaded into
# Python, as opposed to building Python itself as a shared library.)
AC_MSG_CHECKING(CXXSHARED)
if test -z "$CXXSHARED"
then
	CXXSHARED="$LDSHARED"
fi
AC_MSG_RESULT($CXXSHARED)

#
AC_MSG_CHECKING(TRYLINKINGWITHCXX)
if test -z "$TRYLINKINGWITHCXX"
then
	case $host in
	*-*-solaris*) if test "$GCC" = yes;
             then TRYLINKINGWITHCXX="CXXSHARED= $CXX -Wl,-G";
             else TRYLINKINGWITHCXX="CXXSHARED= $CXX -G -L/opt/SUNWspro/lib -lCrun -lCstd";
             fi;;
        *-*-hp*) TRYLINKINGWITHCXX="CXXSHARED= $CXX +z ";;
        *-*-darwin*) TRYLINKINGWITHCXX="CXXSHARED= $CXX -bundle -undefined suppress -flat_namespace";;
        *-*-cygwin* | *-*-mingw*)
            if test "$GCC" = yes; then
                TRYLINKINGWITHCXX="CXXSHARED= $CXX -shared "
            else
                if test "cl" = $CXX ;  then
                    # Microsoft Visual C++ (MSVC)
                    TRYLINKINGWITHCXX="CXXSHARED= $CXX -nologo -LD"
                else
                    TRYLINKINGWITHCXX="#unknown Windows compiler"
                fi
            fi ;;
        *)       TRYLINKINGWITHCXX="CXXSHARED= $CXX -shared ";;
        esac
fi
AC_MSG_RESULT($TRYLINKINGWITHCXX)
# CCSHARED are the C *flags* used to create objects to go into a shared
# library (module) -- this is only needed for a few systems
AC_MSG_CHECKING(CCSHARED)
if test -z "$CCSHARED"
then
	case $host in
	*-*-hp*) if test "$GCC" = yes;
		 then CCSHARED="-fpic";
		 else CCSHARED="+z";
		 fi;;
	*-*-linux*) CCSHARED="-fpic";;
	*-*-freebsd* | *-*-openbsd*) CCSHARED="-fpic";;
	*-*-netbsd*) CCSHARED="-fPIC";;
	*-sco-sysv*) CCSHARED="-KPIC -dy -Bdynamic";;
	*-*-irix6*)  case $CC in
		   *gcc*) CCSHARED="-shared";;
		   *) CCSHARED="";;
		   esac;;
	esac
fi
AC_MSG_RESULT($CCSHARED)

# RPATH is the path used to look for shared library files.
AC_MSG_CHECKING(RPATH)
if test -z "$RPATH"
then
	case $host in
	*-*-solaris*) RPATH='-R. -R$(exec_prefix)/lib';;
        *-*-irix*) RPATH='-rpath .:$(exec_prefix)/lib';;
	*-*-linux*) RPATH='-Xlinker -rpath $(exec_prefix)/lib -Xlinker -rpath .';;
	*)	RPATH='';;
	esac
fi
AC_MSG_RESULT($RPATH)
AC_SUBST(RPATH)

# LINKFORSHARED are the flags passed to the $(CC) command that links
# the a few executables -- this is only needed for a few systems

AC_MSG_CHECKING(LINKFORSHARED)
if test -z "$LINKFORSHARED"
then
	case $host in
	*-*-aix*)	LINKFORSHARED='-Wl,-bE:$(srcdir)/python.exp -lld';;
	*-*-hp*)
	    LINKFORSHARED="-Wl,-E -Wl,+s -Wl,+b\$(BINLIBDEST)/lib-dynload";;
	*-*-linux*) LINKFORSHARED="-Xlinker -export-dynamic";;
	*-*-next*) LINKFORSHARED="-u libsys_s";;
	*-sco-sysv*) LINKFORSHARED="-Bdynamic -dy -Wl,-Bexport";;
	*-*-irix6*) LINKFORSHARED="-all";;
	esac
fi
AC_MSG_RESULT($LINKFORSHARED)

#----------------------------------------------------------------
# Look for java
#----------------------------------------------------------------

AC_ARG_WITH(java, [  --with-java=path        Set location of java executable],[JAVABIN="$withval"], [JAVABIN=])
AC_ARG_WITH(javac, [  --with-javac=path       Set location of javac executable],[JAVACBIN="$withval"], [JAVACBIN=])
AC_ARG_WITH(jar, [  --with-jar=path       Set location of jar executable],[JARBIN="$withval"], [JARBIN=])

if test "$java" = "true" ; then

  if test -z "$JAVABIN" ; then
    AC_CHECK_PROGS(JAVA, java, ["$JAVA_HOME/bin/java"])
  else
    JAVA="$JAVABIN"
  fi

  if test -z "$JAVACBIN" ; then
    AC_CHECK_PROGS(JAVAC, javac, ["$JAVA_HOME/bin/javac"])
  else
    JAVAC="$JAVACBIN"
  fi

  if test -z "$JARBIN" ; then
    AC_CHECK_PROGS(JAR, jar, ["$JAVA_HOME/bin/jar"])
  else
    JAR="$JARBIN"
  fi

  AC_MSG_CHECKING(for java include file jni.h)
  AC_ARG_WITH(javaincl, [  --with-javaincl=path    Set location of Java include directory], [JAVAINCDIR="$withval"], [JAVAINCDIR=])

  if test -z "$JAVAINCDIR"; then
    JAVAINCDIR="/opt/sun-jdk-*/include /opt/blackdown-jdk-*/include /usr/j2sdk*/include /usr/local/j2sdk*/include /usr/jdk*/include /usr/local/jdk*/include /opt/j2sdk*/include /opt/jdk*/include /usr/java/include /usr/java/j2sdk*/include /usr/java/jdk*/include /usr/local/java/include /opt/java/include /usr/include/java /usr/local/include/java /usr/lib/java/include /usr/include/kaffe /usr/local/include/kaffe /usr/include /usr/lib/j2sdk*/include"

    # Add in default installation directory on Windows for Cygwin
    case $host in
    *-*-cygwin* | *-*-mingw*) JAVAINCDIR="c:/Program*Files/Java/jdk*/include d:/Program*Files/Java/jdk*/include c:/j2sdk*/include d:/j2sdk*/include c:/jdk*/include d:/jdk*/include $JAVAINCDIR";;
    *-*-darwin*) JAVAINCDIR="/Library/Java/Home/include $JAVAINCDIR";;
    *);;
    esac
  fi

  JAVAINC=""
  for d in $JAVAINCDIR ; do
    if test -r "$d/jni.h" ; then
      AC_MSG_RESULT($d)
      JAVAINCDIR=$d
      JAVAINC=-I\"$d\"
      break
    fi
  done

  if test "$JAVAINC" = "" ; then
    AC_MSG_RESULT(not found)
    AC_MSG_ERROR([Cannot find jni.h, this is critical, consider --with-javaincl flag], 1)
  else
    # now look for <arch>/jni_md.h
    AC_MSG_CHECKING(for java include file jni_md.h)
    JAVAMDDIR=`find "$JAVAINCDIR" -follow -name jni_md.h -print`
    if test "$JAVAMDDIR" = "" ; then
      AC_MSG_RESULT(not found)
    else
      JAVAMDDIR=`dirname "$JAVAMDDIR"`
      JAVAINC="${JAVAINC} -I\"$JAVAMDDIR\""
      AC_MSG_RESULT($JAVAMDDIR)
    fi
  fi

# java.exe on Cygwin requires the Windows standard (Pascal) calling convention as it is a normal Windows executable and not a Cygwin built executable
  case $host in
  *-*-cygwin* | *-*-mingw*)
      if test "$GCC" = yes; then
          JAVADYNAMICLINKING=" -mno-cygwin -Wl,--add-stdcall-alias"
          JAVACFLAGS="-mno-cygwin"
      else
          JAVADYNAMICLINKING=""
          JAVACFLAGS=""
      fi ;;
  *-*-darwin*) 
          JAVADYNAMICLINKING="-dynamiclib -framework JavaVM"
          JAVACFLAGS=""
          ;;
  *)
          JAVADYNAMICLINKING=""
          JAVACFLAGS=""
          ;;
  esac

  # Java on Windows platforms including Cygwin doesn't use libname.dll, rather name.dll when loading dlls
  case $host in
  *-*-cygwin* | *-*-mingw*) JAVALIBRARYPREFIX="";;
  *)JAVALIBRARYPREFIX="lib";;
  esac

  # Java on Mac OS X tweaks
  case $host in
  *-*-darwin*) 
      JAVASO=".jnilib"
      JAVALDSHARED='$(CC)'
      JAVACXXSHARED='$(CXX)'
      ;;
  *)
      JAVASO=$SO
      JAVALDSHARED='$(LDSHARED)'
      JAVACXXSHARED='$(CXXSHARED)'
      ;;
  esac

  AC_SUBST(JAVA)
  AC_SUBST(JAVAC)
  AC_SUBST(JAVAINC)
  AC_SUBST(JAVADYNAMICLINKING)
  AC_SUBST(JAVALIBRARYPREFIX)
  AC_SUBST(JAVASO)
  AC_SUBST(JAVALDSHARED)
  AC_SUBST(JAVACXXSHARED)
  AC_SUBST(JAVACFLAGS)

  GCJ=$JAVAC
  GCJFLAGS=""
  AC_SUBST(GCJ)
  AC_SUBST(GCJFLAGS)

fi
# end of swig stuff
##########################

AC_OUTPUT([
	Makefile
	Core/Makefile
	Core/SoarKernel/Makefile
	Core/SoarKernel/src/Makefile
	Core/gSKI/Makefile
	Core/gSKI/src/Makefile
	Core/ConnectionSML/Makefile
	Core/ConnectionSML/src/Makefile
	Core/ElementXML/Makefile
	Core/ElementXML/src/Makefile
	Core/CLI/Makefile
	Core/CLI/src/Makefile
	Core/ClientSML/Makefile
	Core/ClientSML/src/Makefile
	Core/ClientSMLSWIG/Makefile
	Core/ClientSMLSWIG/Java/Makefile
	Core/ClientSMLSWIG/Tcl/Makefile
	Core/ClientSMLSWIG/Python/Makefile
	Core/KernelSML/Makefile
	Core/KernelSML/src/Makefile
	Environments/Makefile
	Tools/Makefile
	Tools/FilterC/Makefile
	Tools/TOHSML/Makefile
	Tools/TestCLI/Makefile
	Tools/TestClientSML/Makefile
	Tools/TestConnectionSML/Makefile
	Tools/TestMultiAgent/Makefile
	Tools/TestSMLEvents/Makefile
	Tools/TestSoarPerformance/Makefile
	])
