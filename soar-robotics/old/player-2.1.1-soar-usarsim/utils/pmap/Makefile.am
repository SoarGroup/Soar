AM_CPPFLAGS = -Wall -g -I$(top_builddir)/client_libs -I$(top_srcdir)/client_libs -I$(top_srcdir)

if WITH_GSL
#if INCLUDE_GL
#if INCLUDE_GLU
#if INCLUDE_GLUT
bin_PROGRAMS = pmaptest
lib_LTLIBRARIES = libpmap.la liblodo.la liblododriver.la 
#endif
#endif
#endif
endif

#EXTRA_DIST = pmap.dox

libpmap_la_SOURCES = logfile.cpp omap.cpp pmap.cpp rmap.cpp slap.cpp logfile.h omap.h rmap.h pmap.h slap.h
libpmap_la_CFLAGS = @GSL_CFLAGS@ --fast-math

liblodo_la_SOURCES = lodo.cpp lodo.h
liblodo_la_CFLAGS = @GSL_CFLAGS@ --fast-math
liblodo_la_LIBADD = libpmap.la $(top_builddir)/libplayercore/libplayererror.la $(top_builddir)/client_libs/libplayerc/libplayerc.la $(top_builddir)/libplayerxdr/libplayerxdr.la

pmaptest_SOURCES = pmap_test.cpp
pmaptest_LDADD = @GSL_LIBS@ -lpmap -llodo

liblododriver_la_SOURCES = lodo_driver.cc
liblododriver_la_LIBADD = liblodo.la

doc:
	doxygen pmap.dox

