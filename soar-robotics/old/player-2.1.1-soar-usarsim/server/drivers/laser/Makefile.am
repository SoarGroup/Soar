noinst_LTLIBRARIES = 

if INCLUDE_PBSLASER
noinst_LTLIBRARIES += libpbslaser.la
endif

if INCLUDE_SICKLMS200
noinst_LTLIBRARIES += libsicklms200.la
endif

if INCLUDE_SICKLMS400
noinst_LTLIBRARIES += libsicklms400.la
endif

if INCLUDE_SICKS3000
noinst_LTLIBRARIES += libsicks3000.la
endif

if INCLUDE_LASERCSPACE
noinst_LTLIBRARIES += liblasercspace.la
endif

if INCLUDE_LASERPOSEINTERPOLATOR
noinst_LTLIBRARIES += liblaserposeinterpolator.la
endif

if INCLUDE_LASERRESCAN
noinst_LTLIBRARIES += liblaserrescan.la
endif

if INCLUDE_LASERCUTTER
noinst_LTLIBRARIES += liblasercutter.la
endif

if INCLUDE_URGLASER
noinst_LTLIBRARIES += liburglaser.la
endif

if INCLUDE_RS4LEUZE
noinst_LTLIBRARIES += librs4leuze.la
endif

if INCLUDE_BUMPER2LASER
noinst_LTLIBRARIES += libbumper2laser.la
endif

EXTRA_DIST = lasercspace-1.jpg lasercspace-2.jpg

AM_CPPFLAGS = -Wall -I$(top_srcdir)

libpbslaser_la_SOURCES = pbs_driver.cc
libsicklms200_la_SOURCES = sicklms200.cc
libsicklms400_la_SOURCES = lms400_cola.h lms400_cola.cc sicklms400.cc
libsicks3000_la_SOURCES = sicks3000.cc
liblaserposeinterpolator_la_SOURCES = laserposeinterpolator.cc
liblasercspace_la_SOURCES = lasercspace.cc lasertransform.h lasertransform.cc
liblaserrescan_la_SOURCES = laserrescan.cc lasertransform.h lasertransform.cc
liblasercutter_la_SOURCES = lasercutter.cc lasertransform.h lasertransform.cc
liburglaser_la_SOURCES = urg_laser.cc urg_laser.h urglaserdriver.cc
librs4leuze_la_SOURCES = RS4LeuzeLaserDriver.cc RS4Leuze_laser.cc RS4Leuze_laser.h
libbumper2laser_la_SOURCES = bumper2laser.cc
