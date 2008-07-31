AM_CPPFLAGS = -Wall -I$(top_srcdir)

noinst_LTLIBRARIES =
if INCLUDE_CAMERAV4L
noinst_LTLIBRARIES += libcamerav4l.la
endif

libcamerav4l_la_SOURCES = camerav4l.cc v4lcapture.h v4lcapture.c v4lframe.h v4lframe.c ccvt_c1.c ccvt.h

