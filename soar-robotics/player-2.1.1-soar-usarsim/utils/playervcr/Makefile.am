AM_CPPFLAGS = -Wall -g -I$(top_builddir)/client_libs -I$(top_srcdir)/client_libs -I$(top_srcdir) @GTK_CFLAGS@

if WITH_GTK
bin_PROGRAMS = playervcr
endif

EXTRA_DIST = playervcr-readlog.jpg playervcr-writelog.jpg

playervcr_SOURCES =  playervcr.c

playervcr_DEPENDENCIES = $(top_builddir)/client_libs/libplayerc/libplayerc.la $(top_builddir)/libplayercore/libplayererror.la $(top_builddir)/libplayerxdr/libplayerxdr.la
playervcr_LDADD = $(top_builddir)/client_libs/libplayerc/libplayerc.la $(top_builddir)/libplayercore/libplayererror.la $(top_builddir)/libplayerxdr/libplayerxdr.la @GTK_LIBS@
