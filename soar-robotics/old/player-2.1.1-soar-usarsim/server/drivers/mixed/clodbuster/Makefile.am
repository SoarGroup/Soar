AM_CPPFLAGS = -Wall -I$(top_srcdir) -I$(top_srcdir)/replace

noinst_LTLIBRARIES =
if INCLUDE_CLODBUSTER
noinst_LTLIBRARIES += libclodbuster.la
endif

libclodbuster_la_SOURCES = clodbuster.cc packet.cc clodbuster.h packet.h

