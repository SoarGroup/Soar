noinst_LTLIBRARIES =
if INCLUDE_LOCALBB
noinst_LTLIBRARIES += liblocalbb.la
dnl noinst_PROGRAMS = test
endif

AM_CXXFLAGS = -Wall -I$(top_srcdir) -I$(top_srcdir)/client_libs

liblocalbb_la_SOURCES = localbb.cpp

#test_SOURCES = test.cpp
#test_LDADD = liblocalbb.la $(top_srcdir)/client_libs/libplayerc/.libs/libplayerc.la
