AM_CPPFLAGS = -Wall -I$(top_srcdir) @ARTOOLKITPLUS_CFLAGS@

noinst_LTLIBRARIES =
if INCLUDE_ARTOOLKITPLUS
noinst_LTLIBRARIES += libartoolkitplus.la
endif

libartoolkitplus_la_LIBADD = @ARTOOLKITPLUS_LIBS@
libartoolkitplus_la_SOURCES = artoolkitplus.cc

