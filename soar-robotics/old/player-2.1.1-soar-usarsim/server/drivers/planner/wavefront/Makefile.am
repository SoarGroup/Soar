noinst_LTLIBRARIES = 
if INCLUDE_WAVEFRONT
noinst_LTLIBRARIES += libwavefront.la
endif

AM_CPPFLAGS = -Wall -g -I$(top_srcdir) `pkg-config --cflags gdk-pixbuf-2.0`

libwavefront_la_SOURCES = plan.c plan_plan.c plan_waypoint.c plan.h \
                          plan_control.c wavefront.cc heap.h heap.c
