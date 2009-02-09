AM_CPPFLAGS = -Wall -I$(top_srcdir) -I$(srcdir)/../v4l

noinst_LTLIBRARIES =
if INCLUDE_SPHERE
noinst_LTLIBRARIES += libsphere.la
endif

libsphere_la_SOURCES = sphere_mixed.cc setpwc_api.c pwc-ioctl.h setpwc_api.h sphere_mixed.h
