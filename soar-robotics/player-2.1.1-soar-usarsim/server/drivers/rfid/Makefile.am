noinst_LTLIBRARIES = 
if INCLUDE_INSIDEM300
noinst_LTLIBRARIES += libinsideM300.la
endif
if INCLUDE_SKYETEKM1
noinst_LTLIBRARIES += libskyetekM1.la
endif
if INCLUDE_SICKRFI341
noinst_LTLIBRARIES += libsickRFI341.la
endif
if INCLUDE_PHIDGETRFID
noinst_LTLIBRARIES += libphidgetRFID.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir)

libinsideM300_la_SOURCES = insideM300.cc
libskyetekM1_la_SOURCES = skyetekM1.cc
libsickRFI341_la_SOURCES = rfi341_protocol.h rfi341_protocol.cc sickrfi341.cc
libphidgetRFID_la_SOURCES = phidgetRFID.cc
