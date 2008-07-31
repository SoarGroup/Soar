dnl Process this file with autoconf to produce a configure script.

dnl Initialize autoconf with name and version of package to be built.
dnl PACKAGE and VERSION will be so defined in <config.h>.
AC_INIT(player,2.1.1)

dnl Player version information.  In an ideal world, we would just declare
dnl the following 3 values, then reuse them throughout.  Unfortunately,
dnl the substitution gets ugly if we use these in AC_INIT or
dnl PLAYER_VERSION_INFO.  So, just be damned sure that they all agree!
PLAYER_MAJOR_VERSION="2"
PLAYER_MINOR_VERSION="1"
PLAYER_MICRO_VERSION="1"
AC_SUBST(PLAYER_MAJOR_VERSION)
AC_SUBST(PLAYER_MINOR_VERSION)
AC_SUBST(PLAYER_MICRO_VERSION)

dnl this is used by libtool to assign version numbers
PLAYER_VERSION_INFO="-version-number 2:1:1"
AC_SUBST(PLAYER_VERSION_INFO)

AC_CONFIG_SRCDIR(server/server.cc)

AC_PREREQ(2.53)

dnl figure out build system (uname-style info)
AC_CANONICAL_SYSTEM

m4_pattern_allow(_PKG_ERRORS)

dnl Some parts of the build need to know that we're running on Darwin.
darwin=no
case $host in
  *-darwin*)
    darwin=yes ;;
esac
AM_CONDITIONAL(DARWIN, test "x$darwin" = "xyes")

dnl Initialize automake, specifying that the newer "pax" format should be
dnl used for making tarballs (it doesn't have the 99-character pathname
dnl limit present in older archive formats)
AM_INIT_AUTOMAKE([1.9 tar-ustar dist-bzip2])

dnl Need AC_PROG_MAKE_SET for recursive building
AC_PROG_MAKE_SET

dnl Generate a configuration header called <config.h> and put all the C
dnl preprocessor defines in there.
dnl
dnl Also create playerconfig.h, replacing a couple of C defines with the right
dnl values.  This will be included by player.h
dnl
AC_CONFIG_HEADERS(config.h libplayercore/playerconfig.h client_libs/libplayerc++/playerc++config.h client_libs/libplayerc/playercconfig.h)

dnl Find basic tools
AC_PROG_CC
AC_PROG_CXX
if test ! $GCC = yes -o ! $GXX = yes; then
  AC_MSG_ERROR([Player *requires* gcc and g++ to build.])
fi
AC_PROG_CPP
AC_PROG_INSTALL
AC_C_BIGENDIAN

AC_LANG(C++)

AC_ENABLE_SHARED
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

dnl by default, don't build any shared libs
dnl AC_DISABLE_SHARED
dnl AM_PROG_LIBTOOL

dnl if the user doesn't supply a prefix, we're headed for /usr/local
if test "x$prefix" != "xNONE"; then
  PREFIX=$prefix
else
  PREFIX=/usr/local
fi
AC_SUBST(PREFIX)

dnl Do we have pkg-config?
dnl If we're cross-compiling, pretend we don't have pkg-config, because it
dnl will do the wrong thing (i.e., identify packages on the build systems
dnl instead of the host system).
if test "$host" = "$build"; then
  AC_CHECK_PROG(have_pkg_config,pkg-config,yes,no)
else
  have_pkg_config=no
fi
if test "x$have_pkg_config" = "xno"; then
  AC_MSG_WARN([You should really install pkg-config])
  AC_MSG_WARN([Some components of Player will not be built])
else
  dnl Add prefix to the pkg-config path in case RTK or Gazebo was
  dnl installed there and the user didn't add it manually .
  dnl
  dnl Also, add /usr/local/lib/pkgconfig, because on
  dnl some platforms, pkg-config won't look there by default.
  export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig:/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
fi

dnl Do we have SWIG (used for language bindings)?
AC_CHECK_PROG(have_swig,swig,yes,no)

dnl Do we have Python?
AC_CHECK_PROG(have_python,python,yes,no)
if test "x$have_python" = "xno"; then
  AC_MSG_RESULT([Many parts of Player will not be built.])
  AC_MSG_RESULT([You should really install Python.])
fi
AM_CONDITIONAL(HAVE_PYTHON, test "x$have_python" = "xyes")

dnl Check for python c header
PYTHON_VERSION=`python -V 2>&1 | grep -o "[[0-9]][\.][[0-9]]"`
PYTHON_HEADER="python"$PYTHON_VERSION"/Python.h"
if test ! -f "/usr/include/$PYTHON_HEADER"; then
	PYTHON_HEADER=python/Python.h
fi

AC_CHECK_HEADER($PYTHON_HEADER,
dnl Do we have Python development tools?
AC_MSG_CHECKING([for the distutils Python package])
  ac_distutils_result=`python -c "import distutils" 2>&1 | grep ImportError`
  if test -z "$ac_distutils_result"; then
    AC_MSG_RESULT([yes])
    have_pythondev=yes
  else
    AC_MSG_RESULT([Python extensions will not be built])
    have_pythondev=no
  fi
,    AC_MSG_RESULT([no; Python extensions will not be built])
    have_pythondev=no)
AM_CONDITIONAL(HAVE_PYTHONDEV, test "x$have_pythondev" = "xyes")

dnl Where are the JNI headers?
if test "x$darwin" = "xyes"; then
  default_jnidir=/System/Library/Frameworks/JavaVM.framework/Versions/1.5.0/Headers/
  default_jni_mddir=/System/Library/Frameworks/JavaVM.framework/Versions/1.5.0/Headers/
else
  default_jnidir=/usr/lib/j2sdk1.5-sun/include
  default_jni_mddir=/usr/lib/j2sdk1.5-sun/include/linux
fi

AC_ARG_WITH(jnidir, [  --with-jnidir=<directory containing jni.h],,
with_jnidir=$default_jnidir)
AC_ARG_WITH(jni_mddir, [  --with-jni_mddir=<directory containing jni_md.h],,
with_jni_mddir=$default_jni_mddir)

dnl Look for the JNI headers.  I'm using AC_CHECK_FILE, rather than
dnl AC_CHECK_HEADER, because I can't figure out how to get the latter to
dnl successfully compile a test program that #include's <jni.h>
dnl Furthermore, AC_CHECK_FILE doesn't work when cross-compiling, so we'll
dnl check for that here (is there a better way to see if we're
dnl cross-compiling?)
if test "$host" = "$build"; then
  AC_CHECK_FILE($with_jnidir/jni.h, have_jni_h=yes, have_jni_h=no)
  AC_CHECK_FILE($with_jni_mddir/jni_md.h, have_jni_md_h=yes, have_jni_md_h=no)
else
  have_jni_h=no
  have_jni_md_h=no
fi

dnl Do we have a Java compiler?
AC_CHECK_PROG(have_java,javac,yes,no)

dnl Let the user disable building java wrappers
AC_ARG_ENABLE(jplayer, [  --disable-jplayer         Don't build Jplayer (the Java wrappers)],, enable_jplayer=yes)

dnl Must have Java and JNI headers to compile Jplayer
if test "x$have_java" = "xno" -o "x$have_jni_h" = "xno" -o "x$have_jni_md_h" = "xno"; then
  enable_jplayer=no
fi
JNI_CFLAGS="-I$with_jnidir -I$with_jni_mddir"
AC_SUBST(JNI_CFLAGS)

dnl Must have python, swig, javac, jni.h, and jni_md.h to generate code
dnl for Jplayer
build_jplayer=$enable_jplayer
if test "x$have_python" = "xno" -o "x$have_swig" = "xno" -o "x$have_java" = "xno" -o "x$have_jni_h" = "xno" -o "x$have_jni_md_h" = "xno"; then
  build_jplayer=no
fi
AM_CONDITIONAL(BUILD_JPLAYER, test "x$build_jplayer" = "xyes")

dnl A further test, only relevant to developers who have Java but not SWIG
dnl installed
if test "$host" = "$build" -a "x$build_jplayer" = "xno" -a "x$enable_jplayer" = "xyes"; then
  AC_CHECK_FILE(libplayercore/bindings/java/playercore_java_wrap.cc,,
                enable_jplayer=no)
fi

AM_CONDITIONAL(ENABLE_JPLAYER, test "x$enable_jplayer" = "xyes")

dnl TCP_NODELAY (low-latency) is not used by default
AC_ARG_ENABLE(tcpnodelay,   [  --disable-tcpnodelay      Turn on Nagel's buffering algorithm (may increase socket latency)],, enable_tcpnodelay=yes)
if test "x$enable_tcpnodelay" = "xyes"; then
  AC_DEFINE(ENABLE_TCP_NODELAY,1,[enable TCP_NODELAY])
fi

dnl client tests are compiled by default
AC_ARG_ENABLE(tests,   [  --disable-tests          Don't build client tests],,
enable_tests=yes)
AM_CONDITIONAL(WITH_TESTS, test "x$enable_tests" = "xyes")
AC_ARG_ENABLE(profile, [  --enable-profile        Compile with profiling information (with the -pg and -fprofile-arcs compile flags)],
    if test "x$enableval" = "xyes"; then
        CXXFLAGS="$CXXFLAGS -pg -fprofile-arcs"
        CFLAGS="$CFLAGS -pg -fprofile-arcs"
    fi,
)

AC_ARG_ENABLE(optimize, [  --disable-optimize      Compile without optimization (O2) ],
    if test "x$enableval" = "xyes"; then
        CXXFLAGS="$CXXFLAGS -O2"
        CFLAGS="$CFLAGS -O2"
    elif test "x$enableval" = "xno"; then
        CXXFLAGS=`echo $CXXFLAGS | sed -e s/-O.//g`
        CFLAGS=`echo $CFLAGS | sed -e s/-O.//g`
    else
        CXXFLAGS="$CXXFLAGS -$enableval"
        CFLAGS="$CFLAGS -$enableval"
    fi,
)

AC_ARG_ENABLE(debug,    [  --disable-debug         Compile without debugging information (-g)],
    if test "x$enableval" = "xyes"; then
        CXXFLAGS="$CXXFLAGS -g"
        CFLAGS="$CFLAGS -g"
    elif test "x$enableval" = "xno"; then
        CXXFLAGS=`echo $CXXFLAGS | sed -e s/-g//g`
        CFLAGS=`echo $CFLAGS | sed -e s/-g//g`
    fi,
)

AC_ARG_ENABLE(debug-level,
 [  --enable-debug-level[=[type]]
                      none   : no debugging code compiled
                      low    : some debugging code
                      medium : more debugging code
                      high   : verbose logging to console and system log],
 [ if   test "x$enableval" = "xnone" ; then
      AC_DEFINE(DEBUG_LEVEL, NONE, [no debugging code compiled])
   elif test "x$enableval" = "xlow" ; then
      AC_DEFINE(DEBUG_LEVEL, LOW, [some outputs])
   elif test "x$enableval" = "xmedium" ; then
      AC_DEFINE(DEBUG_LEVEL, MEDIUM, [more outputs])
   elif test "x$enableval" = "xhigh" ; then
      AC_DEFINE(DEBUG_LEVEL, HIGH, [verbose logging to console and system log])
   else
      echo
      echo "Error!"
      echo "Unknown debug level"
      exit -1
   fi
 ])

dnl use the old compiler
AC_ARG_WITH(glibc21, [  --with-glibc21          Use the glibc 2.1 compilers],,
with_glibc21=no)
if test "x$with_glibc21" = "xyes"; then
  CXX=i386-glibc21-linux-g++
  CC=i386-glibc21-linux-gcc
fi

AC_ARG_ENABLE(alldrivers,
              [  --disable-alldrivers         Don't build any drivers],,
              enable_alldrivers=yes)

dnl ************************************************************************
dnl Start of macros from acinclude.m4
dnl ************************************************************************

dnl Do the standard driver tests
PLAYER_DRIVERTESTS

dnl ************************************************************************
dnl End of macros from acinclude.m4
dnl ************************************************************************

dnl Set up GTK/GNOME for the various GUIs
dnl we do it before configuring drivers in case they have GUI bits
dnl The logic is: only use pkg-config
if test "x$have_pkg_config" = "xno"; then
  AC_MSG_WARN([Can't locate GTK/GNOME without pkg-config.])
  AC_MSG_WARN([No GTK/GNOME-based GUIs will be built.])
  with_gtk=no
  with_gnomecanvas=no
  with_gdkpixbuf=no
else
  dnl pkg-config is available, so we use it.
  dnl check for libgnomecanvas and gtk, which some utils
  dnl (e.g., playernav, playervcr) need
  PKG_CHECK_MODULES(GNOMECANVAS, libgnomecanvas-2.0,
                    with_gnomecanvas=yes,
                    with_gnomecanvas=no)
  PKG_CHECK_MODULES(GTK, gtk+-2.0,
                    with_gtk=yes,
                    with_gtk=no)
  PKG_CHECK_MODULES(GDK_PIXBUF, gdk-pixbuf-2.0,
                    with_gdkpixbuf=yes,
                    with_gdkpixbuf=no)
fi
AM_CONDITIONAL(WITH_GNOMECANVAS, test "x$with_gnomecanvas" = "xyes")
AM_CONDITIONAL(WITH_GTK, test "x$with_gtk" = "xyes")
AM_CONDITIONAL(WITH_GDKPIXBUF, test "x$with_gdkpixbuf" = "xyes")

dnl PlayerCc
AC_ARG_WITH(playercc,
            [  --with-playercc         Compile the C++ library],,
            with_playercc=yes)
if test "x$with_playercc" = "xyes"; then
  PLAYERCC_EXTRA_CFLAGS=""
  PLAYERCC_LIB="-lm"
  AC_CXX_NAMESPACES

  AX_BOOST_SIGNALS
  if test "x$ax_cv_boost_signal" = "xyes"; then
    PLAYERCC_EXTRA_CFLAGS="$PLAYERCC_EXTRA_CFLAGS"
    PLAYERCC_LIB="$PLAYERCC_LIB -l$BOOST_SIGNALS_LIB"
  fi
  AX_BOOST_THREAD
  if test "x$ax_cv_boost_thread" = "xyes"; then
      PLAYERCC_EXTRA_CFLAGS="$PLAYERCC_EXTRA_CFLAGS -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT"
      PLAYERCC_LIB="$PLAYERCC_LIB -l$BOOST_THREAD_LIB"
  fi
  if test "x$ax_cv_boost_signal" = "xyes"; then
    if test "x$ax_cv_boost_thread" = "xyes"; then
      ax_playercc_boost=yes
    fi
  fi
  AC_SUBST(PLAYERCC_EXTRA_CFLAGS)
  AC_SUBST(PLAYERCC_LIB)
fi
AM_CONDITIONAL(INCLUDE_BOOST_THREAD, test "x$ax_cv_boost_thread" = "xyes")
AM_CONDITIONAL(INCLUDE_BOOST_SIGNALS, test "x$ax_cv_boost_signal" = "xyes")
AM_CONDITIONAL(INCLUDE_PLAYERCC_BOOST, test "x$ax_playercc_boost" = "xyes")
AM_CONDITIONAL(INCLUDE_PLAYERCC, test "x$with_playercc" = "xyes")

dnl Include RTK gui support (for debugging)
AC_ARG_ENABLE(rtkgui,
[  --enable-rtkgui         Include RTK GUI in server (debugging)],, enable_rtkgui=no)
if test "x$with_gtk" = "xyes"; then
  if test "x$enable_rtkgui" = "xyes"; then
    AC_DEFINE(INCLUDE_RTKGUI, 1, [[include the RTK GUI in server (debugging)]])
  fi
else
  enable_rtkgui=no
fi
AM_CONDITIONAL(INCLUDE_RTKGUI, test "x$enable_rtkgui" = "xyes")

dnl Checks for header files.
AC_CHECK_HEADERS(stdint.h strings.h sys/soundcard.h sys/filio.h linux/joystick.h jpeglib.h zlib.h)

dnl Look for compress2 and compressBound in libz
AC_CHECK_LIB(z,compress2,
             have_libz=yes
             AC_DEFINE(HAVE_LIBZ, 1, [Define to 1 if you have the z library (-lz). ])
             LIBS="$LIBS -lz",
             have_libz=no)
AM_CONDITIONAL(HAVE_LIBZ, test "x$have_libz" = "xyes")
AC_CHECK_LIB(rpc,xdrmem_create,
             have_librpc=yes
             AC_DEFINE(HAVE_LIBRPC, 1, [Define to 1 if you have the rpc library (-lrpc). ])
             LIBS="$LIBS -lrpc",
             have_librpc=no)
AM_CONDITIONAL(HAVE_LIBRPC, test "x$have_librpc" = "xyes")
AC_CHECK_FUNC(compressBound, have_compressBound=yes, have_compressBound=no)
if test "x$have_compressBound" = "xno"; then
  AC_LIBOBJ(compressBound)
  AC_DEFINE(NEED_COMPRESSBOUND, 1, [Define to 1 if you do NOT have the compressBound function in libz])
fi

dnl Look for the dns_sd header.  If it's present, we conclude that we can
dnl build dns_sd code.
have_dnssd=no
AC_ARG_ENABLE(playersd, [  --enable-playersd         Build libplayersd (service discovery)],
[ AC_CHECK_HEADER(dns_sd.h,
                have_dnssd=yes
                AC_DEFINE(HAVE_PLAYERSD, 1, [Define to 1 if libplayersd is being built. ]),
                have_dnssd=no)
dnl Also look for DNSServiceRefDeallocate in libdns_sd.  Some systems
dnl require linking against this lib, others don't
AC_CHECK_LIB(dns_sd,DNSServiceRefDeallocate,
             EXTRA_DNS_SD_LIBS="-ldns_sd"
             AC_DEFINE(HAVE_LIBDNS_SD, 1, [Define to 1 if you have the dns_sd library (-ldns_sd). ])
             LIBS="$LIBS -ldns_sd",
             EXTRA_DNS_SD_LIBS="")
AC_SUBST(EXTRA_DNS_SD_LIBS) ],
enable_playersd=no)
AM_CONDITIONAL(HAVE_DNS_SD_H, test "x$have_dnssd" = "xyes")
AM_CONDITIONAL(HAVE_PLAYERSD, test "x$have_dnssd" = "xyes")


dnl Look for compress2 and compressBound in libz
AC_CHECK_LIB(geos_c,GEOSGeomFromWKB_buf,
             have_libgeos=yes
             AC_DEFINE(HAVE_GEOS, 1, [Define to 1 if you have the geos library (libgeos_c). ])
             AC_SUBST(GEOS_LIBS, -lgeos_c),
             have_libgeos=no)
AM_CONDITIONAL(HAVE_GEOS, test "x$have_libgeos" = "xyes")

dnl Test for socklen_t and define it to int if it's not there.
dnl Note that we can't simply use AC_CHECK_TYPE because socklen_t
dnl isn't in the standard headers (autoconf-2.5 has better methods for this)
AC_MSG_CHECKING(if socklen_t is defined)
AC_TRY_COMPILE(
     [#include <sys/types.h>
      #include <sys/socket.h>],
     [socklen_t len = 42; return len;],
        [AC_MSG_RESULT(yes)],
         [AC_MSG_RESULT(no)
          AC_DEFINE(socklen_t, int, must define this on BSD-based platforms)])

dnl Test for O_SYNC and define it to 0 if it's not there (it gets ORed with
dnl flags, so 0 is harmless).
dnl Note that we can't simply use AC_CHECK_TYPE because O_SYNC
dnl isn't in the standard headers (autoconf-2.5 has better methods for this)
AC_MSG_CHECKING(if O_SYNC is defined)
AC_TRY_COMPILE(
     [#include <sys/types.h>
      #include <sys/stat.h>
      #include <fcntl.h>],
     [int fd = open("foo",O_SYNC); return fd;],
        [AC_MSG_RESULT(yes)],
         [AC_MSG_RESULT(no)
          AC_DEFINE(O_SYNC, 0, must define this on some non-Linux platforms)])

dnl Checks for library functions.
AC_REPLACE_FUNCS(poll dirname cfmakeraw round)
AC_CHECK_FUNCS(sigblock getaddrinfo)
AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(socket, connect)

dnl Look for nanosleep in librt (real time)
dnl On some systems (Darwin) nanosleep is in libc instead
AC_CHECK_LIB(rt, nanosleep)

AC_CHECK_FUNC(clock_gettime, have_clock_gettime=yes, have_clock_gettime=no)
if test "x$have_clock_gettime" = "xno"; then
    AC_LIBOBJ(clock_gettime)
    AC_DEFINE(NEED_CLOCK_GETTIME, 1, [Define to 1 if you do NOT have the clock_gettime function from librt])
fi


AC_CHECK_LIB(ltdl, lt_dlopenext,
             have_libltdl=yes
             LIBLTDL_FLAG=-lltdl
             AC_DEFINE(HAVE_LIBLTDL, 1, [Define to 1 if you have the ltdl library (-lltdl). ])
             LIBS="$LIBS -lltdl -ldl",
             have_libltdl=no
             LIBLTDL_FLAG=,
             [-ldl])
AC_SUBST(LIBLTDL_FLAG)
AC_CHECK_LIB(c_r, pthread_create,
             [LIBS="-lc_r -lpthread $LIBS"],[LIBS="-lpthread $LIBS"])
AC_CHECK_LIB(jpeg,jpeg_read_header,
             have_libjpeg=yes
             AC_DEFINE(HAVE_LIBJPEG, 1, [Define to 1 if you have the jpeg library (-ljpeg). ])
             LIBS="$LIBS -ljpeg",
             have_libjpeg=no)
AM_CONDITIONAL(INCLUDE_JPEG, test "x$have_libjpeg" = "xyes")
AC_CHECK_LIB(GL,glBegin,
             have_libGL=yes
             AC_DEFINE(HAVE_LIBGL, 1, [Define to 1 if you have the GL library (-lGL). ])
             LIBS="$LIBS -lGL",
             have_libGL=no)
AM_CONDITIONAL(INCLUDE_GL, test "x$have_libGL" = "xyes")
AC_CHECK_LIB(GLU,main,
             have_libGLU=yes
             AC_DEFINE(HAVE_LIBGLU, 1, [Define to 1 if you have the GLU library (-lGLU). ])
             LIBS="$LIBS -lGLU",
             have_libGLU=no)
AM_CONDITIONAL(INCLUDE_GLU, test "x$have_libGLU" = "xyes")
AC_CHECK_LIB(glut,main,
             have_libglut=yes
             AC_DEFINE(HAVE_LIBGLUT, 1, [Define to 1 if you have the GLUT library (-lGglut). ])
             LIBS="$LIBS -lglut",
             have_libglut=no)
AM_CONDITIONAL(INCLUDE_GLUT, test "x$have_libglut" = "xyes")


dnl Check for OpenCV
if test "x$have_pkg_config" = "xyes"; then
   PKG_CHECK_MODULES(OPENCV, opencv, with_opencv=yes, with_opencv=no)
   if test "x$with_opencv" = "xyes"; then
      AC_DEFINE(HAVE_OPENCV, 1, [OpenCV library is present])
   fi
fi
AM_CONDITIONAL(WITH_OPENCV, test "x$with_opencv" = "xyes")

if test "x$have_pkg_config" = "xyes"; then
  PKG_CHECK_MODULES(IMAGEMAGICK, ImageMagick++,
                    with_imagemagick=yes,
                    with_imagemagick=no)
fi
AM_CONDITIONAL(WITH_IMAGEMAGICK, test "x$with_imagemagick" = "xyes")

dnl check for gsl
if test "x$have_pkg_config" = "xyes"; then
   PKG_CHECK_MODULES(GSL, gsl, with_gsl=yes, with_gsl=no)
   if test "x$with_gsl" = "xyes"; then
      AC_DEFINE(HAVE_GSL, 1, [GSL library is present])
   fi
fi
AM_CONDITIONAL(WITH_GSL, test "x$with_gsl" = "xyes")

dnl Do we have XMMS (used for xmms plugin)?
AC_CHECK_PROG(have_xmms,xmms-config,yes,no)
AM_CONDITIONAL(HAVE_XMMS, test "x$have_xmms" = "xyes")

dnl Optionally build python bindings to libplayerc (libplayerc-py)
AC_ARG_ENABLE(libplayerc-py,
     [  --disable-libplayerc-py    Don't compile python bindings for libplayerc],,
     enable_libplayerc_py=yes)
if test "x$enable_libplayerc_py" = "xno"; then
  enable_libplayerc_py=no
  reason_libplayerc_py="disabled by user"
elif test "x$have_swig" = "xno"; then
  enable_libplayerc_py=no
  reason_libplayerc_py="could not find swig"
elif test "x$have_python" = "xno"; then
  enable_libplayerc_py=no
  reason_libplayerc_py="could not find python"
elif test "x$have_pythondev" = "xno"; then
  enable_libplayerc_py=no
  reason_libplayerc_py="could not find python distutils"
elif test "x$have_libjpeg" = "xno"; then
  enable_libplayerc_py=no
  reason_libplayerc_py="could not find libjpeg"
fi
AM_CONDITIONAL(INCLUDE_LIBPLAYERC_PY, test "x$enable_libplayerc_py" = "xyes")

dnl Create the following Makefiles (from the Makefile.ams)
AC_OUTPUT(Makefile
          rtk2/Makefile
          replace/Makefile
          doc/Makefile
          doc/images/Makefile
          libplayerjpeg/Makefile
          libplayercore/Makefile
          libplayercore/bindings/Makefile
          libplayercore/bindings/java/Makefile
          libplayercore/playercore.pc
          libplayercore/playererror.pc
          libplayercore/playerutils.pc
          libplayerxdr/Makefile
          libplayerxdr/playerxdr.pc
          libplayertcp/Makefile
          libplayertcp/bindings/Makefile
          libplayertcp/bindings/java/Makefile
          libplayertcp/playertcp.pc
          libplayertcp/playerudp.pc
          libplayersd/Makefile
          libplayersd/playersd.pc
          server/Makefile
          server/libplayerdrivers/Makefile
          server/libplayerdrivers/playerdrivers.pc
          server/drivers/Makefile
          server/drivers/base/Makefile
          server/drivers/gps/Makefile
          server/drivers/shell/Makefile
          server/drivers/mixed/Makefile
          server/drivers/mixed/phidgetIFK/Makefile
          server/drivers/mixed/wbr/Makefile
          server/drivers/mixed/wbr/914/Makefile
          server/drivers/position/Makefile
          server/drivers/actarray/Makefile
          server/drivers/audio/Makefile
          server/drivers/camera/Makefile
          server/drivers/camera/v4l/Makefile
          server/drivers/camera/1394/Makefile
          server/drivers/camera/compress/Makefile
          server/drivers/camera/imageseq/Makefile
          server/drivers/camera/sphere/Makefile
          server/drivers/camera/uvc/Makefile
          server/drivers/camera/yarp/Makefile
          server/drivers/fiducial/Makefile
          server/drivers/ptz/Makefile
          server/drivers/laser/Makefile
          server/drivers/blobfinder/Makefile
          server/drivers/blobfinder/artoolkitplus/Makefile
          server/drivers/blobfinder/cmvision/Makefile
          server/drivers/blobfinder/upcbarcode/Makefile
          server/drivers/blobfinder/simpleshape/Makefile
          server/drivers/blobfinder/acts/Makefile
          server/drivers/blobfinder/shapetracker/Makefile
          server/drivers/wifi/Makefile
          server/drivers/speech/Makefile
          server/drivers/speech/recognition/Makefile
          server/drivers/mixed/chatterbox/Makefile
          server/drivers/mixed/cmucam2/Makefile
          server/drivers/mixed/mricp/Makefile
          server/drivers/mixed/p2os/Makefile
          server/drivers/mixed/erratic/Makefile
          server/drivers/mixed/reb/Makefile
          server/drivers/mixed/rflex/Makefile
          server/drivers/mixed/robotino/Makefile
          server/drivers/mixed/khepera/Makefile
          server/drivers/mixed/rmp/Makefile
          server/drivers/mixed/garcia/Makefile
          server/drivers/mixed/nomad/Makefile
          server/drivers/mixed/botrics/Makefile
          server/drivers/mixed/clodbuster/Makefile
          server/drivers/mixed/evolution/Makefile
          server/drivers/mixed/evolution/er1/Makefile
          server/drivers/mixed/irobot/Makefile
          server/drivers/mixed/irobot/roomba/Makefile
          server/drivers/mixed/irobot/create/Makefile
          server/drivers/mixed/sr3000/Makefile
          server/drivers/opaque/Makefile
          server/drivers/health/Makefile
          server/drivers/health/statgrab/Makefile
          server/drivers/planner/Makefile
          server/drivers/planner/wavefront/Makefile
          server/drivers/position/microstrain/Makefile
          server/drivers/position/ascension/Makefile
          server/drivers/position/bumpersafe/Makefile
          server/drivers/position/nd/Makefile
          server/drivers/position/isense/Makefile
          server/drivers/position/lasersafe/Makefile
          server/drivers/position/nav200/Makefile
          server/drivers/position/roboteq/Makefile
          server/drivers/position/vfh/Makefile
          server/drivers/position/mbicp/Makefile
          server/drivers/mcom/Makefile
          server/drivers/localization/Makefile
          server/drivers/localization/amcl/Makefile
          server/drivers/service_adv/Makefile
          server/drivers/map/Makefile
          server/drivers/joystick/Makefile
          server/drivers/rfid/Makefile
          server/drivers/wsn/Makefile
          server/drivers/imu/Makefile
          server/drivers/pointcloud3d/Makefile
          server/drivers/ranger/Makefile
          server/drivers/vectormap/Makefile
          server/drivers/blackboard/Makefile
          server/drivers/blackboard/localbb/Makefile
          client_libs/Makefile
          client_libs/libplayerc++/Makefile
          client_libs/libplayerc++/playerc++.pc
          client_libs/libplayerc++/test/Makefile
          client_libs/libplayerc/Makefile
          client_libs/libplayerc/doc/Makefile
          client_libs/libplayerc/playerc.pc
          client_libs/libplayerc/test/Makefile
          client_libs/libplayerc/bindings/Makefile
          client_libs/libplayerc/bindings/python/Makefile
          examples/Makefile
          examples/plugins/Makefile
          examples/plugins/exampledriver/Makefile
          examples/plugins/logplugin/Makefile
          examples/plugins/exampleinterface/Makefile
          examples/plugins/multidriver/Makefile
          examples/plugins/opaquedriver/Makefile
          examples/libplayerc/Makefile
          examples/libplayerc++/Makefile
          utils/Makefile
          utils/playercam/Makefile
          utils/playerwritemap/Makefile
          utils/playerv/Makefile
          utils/playerjoy/Makefile
          utils/playernav/Makefile
          utils/playerprint/Makefile
          utils/playervcr/Makefile
          utils/pmap/Makefile
          utils/dgps_server/Makefile
          utils/xmms-plugin/Makefile
          utils/logsplitter/Makefile
          utils/playerprop/Makefile
          config/Makefile)

dnl should use AC_MSG_NOTICE(), but that's apparently not defined in
dnl autoconf 2.13.
AC_MSG_RESULT([])
AC_MSG_RESULT([*************************************************************])
AC_MSG_RESULT([])
AC_MSG_RESULT([Player will be built on a $build system to run])
AC_MSG_RESULT([on a $host system, with the following tools:])
AC_MSG_RESULT([    C   compiler: $CC $CFLAGS])
AC_MSG_RESULT([    C++ compiler: $CXX $CXXFLAGS])

AC_MSG_RESULT([])
if test "x$have_libltdl" = "xyes"; then
  AC_MSG_RESULT([Support for plugin drivers will be included.])
  AC_MSG_RESULT([ ])
else
  AC_MSG_RESULT([Support for plugin drivers will NOT be included.])
  AC_MSG_RESULT([You need plugin support to use Stage.])
  AC_MSG_RESULT([To add plugin support, install libltdl, which is ])
  AC_MSG_RESULT([part of GNU libtool, then configure Player again.])
  AC_MSG_RESULT([ ])
fi

if test "x$PYTHON_INTERP" != "xno"; then
   AC_MSG_RESULT([To use the Python bindings, modify your PYTHONPATH variable to include])
   AC_MSG_RESULT([    $PREFIX/lib/python/\$PYTHON_VERSION/site-packages])
   AC_MSG_RESULT([For example:])
   AC_MSG_RESULT([    export PYTHONPATH=\$PYTHONPATH:$PREFIX/lib/python/\$PYTHON_VERSION/site-packages])
fi
AC_MSG_RESULT([])

dnl Expand this to include all client libs/bindings
dnl libplayerc python bindings
if test "x$enable_libplayerc_py" = "xyes"; then
  AC_MSG_RESULT([Python bindings to libplayerc will be built])
else
  AC_MSG_RESULT([Python bindings to libplayerc will not be built -- $reason_libplayerc_py])
fi
AC_MSG_RESULT([])

dnl show if libplayerc++ will or will not be built
if test "x$with_playercc" = "xyes"; then
  AC_MSG_RESULT([libplayerc++ will be built])
  if test "x$ax_cv_boost_signal" = "xyes"; then
    AC_MSG_RESULT([  signaling included])
  else
    AC_MSG_RESULT([  signaling NOT included])
  fi
  if test "x$ax_cv_boost_thread" = "xyes"; then
    AC_MSG_RESULT([  multithreading included])
  else
    AC_MSG_RESULT([  multithreading NOT included])
  fi
  if test "x$ax_playercc_boost" != "xyes"; then
    AC_MSG_RESULT([  Install BOOST (www.boost.org) libraries to enable multithreaded ])
    AC_MSG_RESULT([  and/or signaling in libplayerc++.])
  fi
else
  AC_MSG_RESULT([libplayerc++ will NOT be built])
fi
AC_MSG_RESULT([])

AC_MSG_RESULT([The following device drivers will be included:])
for player_lib in $PLAYER_DRIVERS; do
  if test -n $player_lib; then
    AC_MSG_RESULT([[+ $player_lib]])
  fi
done
AC_MSG_RESULT([])
AC_MSG_RESULT([The following device drivers will NOT be included:])
dnl Change the internal field seperator (IFS) to be a colon, so that the
dnl driver list will be parsed correctly.
old_IFS=$IFS
IFS=:
for player_lib in $PLAYER_NODRIVERS; do
  if test ! $player_lib = "" ; then
    AC_MSG_RESULT([[- $player_lib]])
  fi
done
dnl Change the IFS back to what it was.
IFS=$old_IFS

AC_MSG_RESULT([])
AC_MSG_RESULT([Player will be installed in:])
AC_MSG_RESULT([    $PREFIX/])
AC_MSG_RESULT([])
AC_MSG_RESULT([To see the configuration options, do:])
AC_MSG_RESULT([    ./configure --help])
AC_MSG_RESULT([])
AC_MSG_RESULT([When you're satisfied with the configuration, type 'make install'. ])
AC_MSG_RESULT([])
