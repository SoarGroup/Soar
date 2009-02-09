AM_CPPFLAGS = -g -Wall -I$(top_srcdir)

# create the pkg-config entry for the server headers
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = playertcp.pc playerudp.pc

EXTRA_DIST = playertcp.pc.in playerudp.pc.in
CLEANFILES = playertcp.pc playerudp.pc

lib_LTLIBRARIES = libplayertcp.la libplayerudp.la

libplayertcp_la_DEPENDENCIES = $(top_builddir)/libplayercore/libplayercore.la $(top_builddir)/libplayercore/libplayererror.la $(top_builddir)/libplayercore/libplayerutils.la $(top_builddir)/libplayerxdr/libplayerxdr.la

libplayertcp_la_SOURCES = socket_util.c socket_util.h \
                         playertcp.cc playertcp.h \
			 remote_driver.cc remote_driver.h

libplayertcp_la_LIBADD = $(top_builddir)/replace/*.lo
libplayertcp_la_LDFLAGS = $(PLAYER_VERSION_INFO) -rpath $(libdir) $(top_builddir)/libplayercore/libplayercore.la $(top_builddir)/libplayercore/libplayererror.la $(top_builddir)/libplayercore/libplayerutils.la $(top_builddir)/libplayerxdr/libplayerxdr.la

libplayerudp_la_DEPENDENCIES = $(top_builddir)/libplayercore/libplayercore.la $(top_builddir)/libplayercore/libplayererror.la $(top_builddir)/libplayercore/libplayerutils.la $(top_builddir)/libplayerxdr/libplayerxdr.la

libplayerudp_la_SOURCES = playerudp.cc playerudp.h
#		  remote_driver.cc remote_driver.h

libplayerudp_la_LIBADD = $(top_builddir)/replace/*.lo
libplayerudp_la_LDFLAGS = $(PLAYER_VERSION_INFO) -rpath $(libdir) $(top_builddir)/libplayercore/libplayercore.la $(top_builddir)/libplayercore/libplayererror.la $(top_builddir)/libplayercore/libplayerutils.la $(top_builddir)/libplayerxdr/libplayerxdr.la

pkgincludedir = $(includedir)/player-$(PLAYER_MAJOR_VERSION).$(PLAYER_MINOR_VERSION)/libplayertcp
pkginclude_HEADERS = playertcp.h playerudp.h
