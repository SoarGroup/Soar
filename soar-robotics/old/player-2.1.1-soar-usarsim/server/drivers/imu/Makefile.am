noinst_LTLIBRARIES = 
if INCLUDE_XSENSMT
noinst_LTLIBRARIES += libXSensMT.la
endif
if INCLUDE_NIMU
noinst_LTLIBRARIES += libnimu.la
endif
AM_CPPFLAGS = -Wall -I$(top_srcdir)

libXSensMT_la_SOURCES = XSensMT.cc MTComm.cpp MTComm.h

libnimu_la_SOURCES = nimu.h nimu.cpp nimuplayer.cpp nimu.h
