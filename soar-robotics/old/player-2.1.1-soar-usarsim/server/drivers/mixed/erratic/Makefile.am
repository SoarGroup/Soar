noinst_LTLIBRARIES =
if INCLUDE_ERRATIC
noinst_LTLIBRARIES += liberratic.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir)

liberratic_la_SOURCES = erratic.cc erratic.h \
                     motorpacket.h motorpacket.cc \
                     packet.h packet.cc \
                     robot_params.cc robot_params.h \
                     sip.cc sip.h
