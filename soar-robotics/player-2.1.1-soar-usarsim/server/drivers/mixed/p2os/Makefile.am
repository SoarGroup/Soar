noinst_LTLIBRARIES =
if INCLUDE_P2OS
noinst_LTLIBRARIES += libp2os.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir)

libp2os_la_SOURCES = p2os.cc p2os.h \
                     packet.h packet.cc \
                     robot_params.cc robot_params.h \
                     sip.cc sip.h kinecalc.h kinecalc.cc
