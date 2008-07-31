AM_CPPFLAGS = -Wall -g -I$(top_builddir)/client_libs -I$(top_srcdir)/client_libs -I$(top_srcdir) @GNOMECANVAS_CFLAGS@ -fno-strict-aliasing

if WITH_GNOMECANVAS
bin_PROGRAMS = playernav
endif

EXTRA_DIST = playernav-example.jpg

playernav_SOURCES =  playernav.c gui.c playernav.h player.c parse.c

playernav_LDADD = $(top_builddir)/client_libs/libplayerc/libplayerc.la $(top_builddir)/libplayerxdr/libplayerxdr.la $(top_builddir)/libplayercore/libplayererror.la @GNOMECANVAS_LIBS@
