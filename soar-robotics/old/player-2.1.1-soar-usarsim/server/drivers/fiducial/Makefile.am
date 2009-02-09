noinst_LTLIBRARIES = 
if INCLUDE_LASERBAR
noinst_LTLIBRARIES += liblaserbar.la
endif
if INCLUDE_LASERBARCODE
noinst_LTLIBRARIES += liblaserbarcode.la
endif
if INCLUDE_LASERVISUALBARCODE
noinst_LTLIBRARIES += liblaservisualbarcode.la
endif
if INCLUDE_LASERVISUALBW
noinst_LTLIBRARIES += liblaservisualbw.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir)

EXTRA_DIST = beacon.jpg laservisualbeacon.jpg

liblaserbar_la_SOURCES = laserbar.cc
liblaserbarcode_la_SOURCES = laserbarcode.cc
liblaservisualbarcode_la_SOURCES = laservisualbarcode.cc
liblaservisualbw_la_SOURCES = laservisualbw.cc
