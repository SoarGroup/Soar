if WITH_GTK
PLAYERV = playerv
endif

EXTRA_DIST = playerv-laser-blobfinder-ptz.jpg playerv-sonar.jpg

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)/client_libs -I$(top_srcdir)/client_libs @GTK_CFLAGS@ 

bin_PROGRAMS = $(PLAYERV)

playerv_SOURCES = playerv.c \
                  playerv.h \
                  registry.c \
                  mainwnd.c \
                  error.h \
                  opt.c \
                  opt.h \
                  pv_dev_actarray.c \
                  pv_dev_aio.c \
                  pv_dev_blobfinder.c \
                  pv_dev_camera.c \
                  pv_dev_bumper.c \
                  pv_dev_dio.c \
                  pv_dev_fiducial.c \
                  pv_dev_ir.c \
                  pv_dev_laser.c \
                  pv_dev_map.c \
                  pv_dev_position2d.c \
                  pv_dev_power.c \
                  pv_dev_ptz.c \
                  pv_dev_sonar.c \
                  pv_dev_wifi.c \
                  pv_dev_gripper.c \
                  pv_dev_ranger.c \
                  pv_dev_vectormap.c
#pv_dev_localize.c

playerv_DEPENDENCIES = $(top_builddir)/client_libs/libplayerc/libplayerc.la \
                       $(top_builddir)/libplayerxdr/libplayerxdr.la \
                       $(top_builddir)/libplayercore/libplayererror.la
playerv_LDADD = $(top_builddir)/client_libs/libplayerc/libplayerc.la \
	$(top_builddir)/libplayerxdr/libplayerxdr.la \
	$(top_builddir)/libplayercore/libplayererror.la \
	$(top_builddir)/libplayercore/libplayerutils.la \
	-lrtk -L$(top_builddir)/rtk2 -lrtk @GTK_LIBS@ @GEOS_LIBS@
