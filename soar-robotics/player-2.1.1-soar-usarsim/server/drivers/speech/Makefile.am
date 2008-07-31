SUBDIRS = recognition

noinst_LTLIBRARIES = 
if INCLUDE_FESTIVAL
noinst_LTLIBRARIES += libfestival.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir)

libfestival_la_SOURCES = festival.cc