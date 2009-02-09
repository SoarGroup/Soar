noinst_LTLIBRARIES = 
if INCLUDE_LINUXWIFI
noinst_LTLIBRARIES += liblinuxwifi.la
endif
if INCLUDE_IWSPY
noinst_LTLIBRARIES += libiwspy.la
endif
if INCLUDE_AODV
noinst_LTLIBRARIES += libaodv.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir)

liblinuxwifi_la_SOURCES = linuxwifi.cc
libaodv_la_SOURCES = aodv.cc
libiwspy_la_SOURCES = iwspy.cc
