AM_CPPFLAGS = -I$(top_builddir)/client_libs \
              -I$(top_srcdir) \
              -I$(top_srcdir)/client_libs \
              -Wall -g $(PLAYERCC_EXTRA_CFLAGS)


test_DEPENDENCIES = $(top_builddir)/client_libs/libplayerc/libplayerc.la \
                    $(top_builddir)/client_libs/libplayerc++/libplayerc++.la \
                    $(top_builddir)/libplayerxdr/libplayerxdr.la \
                    $(top_builddir)/libplayercore/libplayererror.la

PLAYERCC_LIB_DIRS=-L${top_builddir}/libplayerxdr \
                  -L${top_builddir}/libplayercore \
                  -L${top_builddir}/client_libs/libplayerc \
                  -L${top_builddir}/client_libs/libplayerc++

test_LDADD = $(PLAYERCC_LIB_DIRS) -lplayerc++ $(PLAYERCC_LIB) -lplayerc -lplayererror -lplayerxdr


noinst_PROGRAMS =
if INCLUDE_PLAYERCC
if WITH_TESTS
  noinst_PROGRAMS += test
endif
endif

test_SOURCES = test.cc \
               test.h \
               test_camera.cc \
               test_rfid.cc \
               test_wsn.cc \
               test_gripper.cc \
               test_laser.cc \
               test_position2d.cc \
               test_ptz.cc \
               test_sonar.cc \
               test_power.cc \
               test_dio.cc \
               test_bumper.cc \
               test_position2d_holdsub.cc \
               test_gripper_holdsub.cc \
               test_actarray.cc \
               test_aio.cc \
               test_speech.cc \
               test_ranger.cc



# These tests are broken and need to be fixed or removed:
#test_wifi.cc 
#test_log.cc 
#test_lbd.cc 
#test_fiducialfinder.cc 
#test_blobfinder.cc 
#test_gps.cc
#test_idar.cc
#test_idarturret.cc
#test_misc.cc
#test_bps.cc
#test_motor.cc 
#test_position.cc 
#test_position3d.cc 
#test_speech.cc 
#test_truth.cc 
#test_vision.cc 
#test_position_control.cc 
#test_mcom.cc 
#test_lookup.cc 
#test_audiodsp.cc 
#test_localize.cc 
#test_audiomixer.cc 
#test_blinkenlight.cc
