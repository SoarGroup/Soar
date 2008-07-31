AM_CPPFLAGS = -Wall -I$(top_srcdir)

noinst_LTLIBRARIES =
if INCLUDE_CAMERACOMPRESS
noinst_LTLIBRARIES += libcameracompress.la libcamerauncompress.la
endif

libcameracompress_la_SOURCES = cameracompress.cc
libcamerauncompress_la_SOURCES = camerauncompress.cc
