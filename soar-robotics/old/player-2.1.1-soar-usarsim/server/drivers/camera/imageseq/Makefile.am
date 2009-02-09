AM_CPPFLAGS = -Wall -I$(top_srcdir) @OPENCV_CFLAGS@

noinst_LTLIBRARIES =
if INCLUDE_IMAGESEQ
noinst_LTLIBRARIES += libimageseq.la
endif

libimageseq_la_SOURCES = imageseq.cc
libimageseq_la_LIBADD = @OPENCV_LIBS@
