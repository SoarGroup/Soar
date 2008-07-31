AM_CPPFLAGS = -Wall -I$(top_srcdir)

noinst_LTLIBRARIES =
if INCLUDE_STATGRAB
noinst_LTLIBRARIES += libstatgrab.la
endif

libstatgrab_la_SOURCES = statgrab_health.cpp statgrab_health.h
#libstatgrab_la_LIBADD += -lstatgrab
