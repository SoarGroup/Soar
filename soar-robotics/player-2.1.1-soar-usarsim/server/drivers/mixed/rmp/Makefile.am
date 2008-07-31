noinst_LTLIBRARIES = 
if INCLUDE_SEGWAYRMP
noinst_LTLIBRARIES += libsegwayrmp.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir) @SEGWAYRMP_EXTRA_CPPFLAGS@

libsegwayrmp_la_SOURCES = segwayrmp.h segwayrmp.cc rmp_frame.h \
                      canio.h canio_kvaser.h canio_kvaser.cc 

