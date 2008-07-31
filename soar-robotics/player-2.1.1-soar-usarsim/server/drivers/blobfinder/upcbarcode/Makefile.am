AM_CPPFLAGS = -Wall -I$(top_srcdir) @OPENCV_CFLAGS@

noinst_LTLIBRARIES =
if INCLUDE_UPCBARCODE
noinst_LTLIBRARIES += libupcbarcode.la
endif

EXTRA_DIST = upc.jpg

libupcbarcode_la_SOURCES = upcbarcode.cc
libupcbarcode_la_LIBADD = @OPENCV_LIBS@
