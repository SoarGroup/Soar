#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([SoarIO], [8.6.0], [soar-group@lists.sourceforge.net])
AM_INIT_AUTOMAKE([SoarIO], [8.6.0])
AC_PREREQ(2.57)
AC_REVISION($Revision$)
AC_CONFIG_SRCDIR([ConnectionSML/src/sml_AnalyzeXML.cpp])
AM_CONFIG_HEADER(config.h)
AC_PREFIX_DEFAULT(/usr/local/soar)

# Checks for programs.
#AC_DISABLE_SHARED
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

####################
# SWIG STUFF

AC_ARG_WITH(swig-suffix,
  [  --with-swig-suffix=SUFFIX    Attach SUFFIX to the swig binary. ],
  [swig_suffix="$withval"], [swig_suffix=""])
AC_SUBST(swig_suffix)
AC_DEFINE_UNQUOTED(RELEASE_SUFFIX, "swig_suffix", [Executable swig suffix for co-existence with older versions])

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
	*-*-darwin*) SO=.bundle;;
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

AC_ARG_WITH(java, [  --with-java=path        Set location of Java executable],[JAVABIN="$withval"], [JAVABIN=])
AC_ARG_WITH(javac, [  --with-javac=path       Set location of Javac executable],[JAVACBIN="$withval"], [JAVACBIN=])

if test -z "$JAVABIN" ; then
  AC_CHECK_PROGS(JAVA, java kaffe guavac)
else
  JAVA="$JAVABIN"
fi

if test -z "$JAVACBIN" ; then
  AC_CHECK_PROGS(JAVAC, javac)
else
  JAVAC="$JAVACBIN"
fi

AC_MSG_CHECKING(for java include file jni.h)
AC_ARG_WITH(javaincl, [  --with-javaincl=path    Set location of Java include directory], [JAVAINCDIR="$withval"], [JAVAINCDIR=])

if test -z "$JAVAINCDIR"; then
  JAVAINCDIR="/opt/blackdown-jdk-*/include /usr/j2sdk*/include /usr/local/j2sdk*/include /usr/jdk*/include /usr/local/jdk*/include /opt/j2sdk*/include /opt/jdk*/include /usr/java/include /usr/java/j2sdk*/include /usr/java/jdk*/include /usr/local/java/include /opt/java/include /usr/include/java /usr/local/include/java /usr/lib/java/include /usr/include/kaffe /usr/local/include/kaffe /usr/include"

  # Add in default installation directory on Windows for Cygwin
  case $host in
  *-*-cygwin* | *-*-mingw*) JAVAINCDIR="c:/Program*Files/Java/jdk*/include d:/Program*Files/Java/jdk*/include c:/j2sdk*/include d:/j2sdk*/include c:/jdk*/include d:/jdk*/include $JAVAINCDIR";;
  *-*-darwin*) JAVAINCDIR="/System/Library/Frameworks/JavaVM.framework/Headers $JAVAINCDIR";;
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

# end of swig stuff
##########################

AC_OUTPUT(Makefile
		  ConnectionSML/Makefile 
		  ConnectionSML/src/Makefile 
		  ElementXML/Makefile 
		  ElementXML/src/Makefile 
		  CommandLineInterface/Makefile 
		  CommandLineInterface/src/Makefile 
		  ClientSML/Makefile 
		  ClientSML/src/Makefile
		  ClientSMLSWIG/Makefile
		  ClientSMLSWIG/Java/Makefile
		  ClientSMLSWIG/Tcl/Makefile
		  KernelSML/Makefile
		  KernelSML/src/Makefile
		  examples/Makefile
		  examples/TestCommandLineInterface/Makefile
		  examples/towersSML/Makefile
		  examples/towers/Makefile
		  examples/TestClientSML/Makefile
		  examples/TestThreadLibrary/Makefile)
