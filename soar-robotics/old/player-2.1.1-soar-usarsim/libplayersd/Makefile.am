AM_CPPFLAGS = -g -Wall -I$(top_srcdir)

lib_LTLIBRARIES =
pkgconfig_DATA = 
pkginclude_HEADERS =

if HAVE_DNS_SD_H
lib_LTLIBRARIES += libplayersd.la
pkgconfig_DATA += playersd.pc
pkginclude_HEADERS += playersd.h
endif

# create the pkg-config entry for the server headers
pkgconfigdir = $(libdir)/pkgconfig
pkgincludedir = $(includedir)/player-$(PLAYER_MAJOR_VERSION).$(PLAYER_MINOR_VERSION)/libplayersd

EXTRA_DIST = playersd.pc.in
CLEANFILES = playersd.pc

libplayersd_la_SOURCES = playersd_mdns.c playersd.h playersd.c

libplayersd_la_LDFLAGS = $(PLAYER_VERSION_INFO) -rpath $(libdir) $(top_builddir)/libplayercore/libplayererror.la $(top_builddir)/libplayercore/libplayerutils.la
