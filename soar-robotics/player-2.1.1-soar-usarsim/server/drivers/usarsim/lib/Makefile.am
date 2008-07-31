noinst_LTLIBRARIES =
if INCLUDE_USARSIM
noinst_LTLIBRARIES += libusarsim.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir)

libusarsim_la_SOURCES = ../src/us_bot.cc ../src/us_bot.h \
					../src/us_parser.cc ../src/us_parser.h \
					../src/us_position.cc ../src/us_position.h \
                         ../src/us_fakelocalize.cc ../src/us_fakelocalize.h \
					../src/us_position3d.cc ../src/us_position3d.h \
                         ../src/temp_laser3d_structures.h \
					../src/us_laser.cc ..src/us_laser.h \
                         ../src/us_laser3d.cc ..src/us_laser3d.h \
					../src/us_sonar.cc ../src/us_sonar.h \
					../src/us_ptz.cc ../src/us_ptz.h \
					../src/us_fiducial.cc ../src/us_fiducial.h \
					../src/us_victfiducial.cc ../us_victfiducial.h \
					../src/rh_ir.cc ../src/rh_ir.h \
					../src/rh_pyro.cc ../src/rh_pyro.h 
