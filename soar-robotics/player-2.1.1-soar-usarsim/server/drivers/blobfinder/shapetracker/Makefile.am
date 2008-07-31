AM_CPPFLAGS = -Wall -I$(top_srcdir) -I$(srcdir)/../.. @OPENCV_CFLAGS@

noinst_LTLIBRARIES =
if INCLUDE_SHAPETRACKER
noinst_LTLIBRARIES += libshapetracker.la
endif

libshapetracker_la_SOURCES = shapetracker.cc
libshapetracker_la_LIBADD = @OPENCV_LIBS@
