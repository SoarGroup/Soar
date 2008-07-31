noinst_LTLIBRARIES = 
if INCLUDE_SERIALSTREAM
noinst_LTLIBRARIES += libserialstream.la
endif
if INCLUDE_TCPSTREAM
noinst_LTLIBRARIES += libtcpstream.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir)

libserialstream_la_SOURCES = serialstream.cc
libtcpstream_la_SOURCES = tcpstream.cc



