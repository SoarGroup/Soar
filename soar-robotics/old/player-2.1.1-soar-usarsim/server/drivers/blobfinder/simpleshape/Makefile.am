AM_CPPFLAGS = -Wall -I$(top_srcdir) @OPENCV_CFLAGS@

noinst_LTLIBRARIES =
if INCLUDE_SIMPLESHAPE
noinst_LTLIBRARIES += libsimpleshape.la
endif

EXTRA_DIST = simpleshape_h.gif simpleshape_output.gif

libsimpleshape_la_SOURCES = simpleshape.cc
libsimpleshape_la_LIBADD = @OPENCV_LIBS@

