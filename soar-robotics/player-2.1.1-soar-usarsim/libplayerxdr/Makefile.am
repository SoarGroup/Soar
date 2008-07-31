AM_CPPFLAGS = -g -Wall -I$(top_srcdir) -fno-strict-aliasing

# create the pkg-config entry for the server headers
pkgconfigdir = $(libdir)/pkgconfig
pkgincludedir = $(includedir)/player-$(PLAYER_MAJOR_VERSION).$(PLAYER_MINOR_VERSION)/libplayerxdr

BUILT_SOURCES = playerxdr.c playerxdr.h functiontable_gen.c

lib_LTLIBRARIES = libplayerxdr.la
pkgconfig_DATA = playerxdr.pc
pkginclude_HEADERS = playerxdr.h functiontable.h

dist_bin_SCRIPTS = playerxdrgen.py

EXTRA_DIST = playerxdr.pc.in playerxdr.html playerxdr.c playerxdr.h functiontable_gen.c

CLEANFILES = playerxdr.pc

functiontable_gen.c: $(top_srcdir)/libplayercore/playerinterfacegen.py $(top_srcdir)/libplayercore/interfaces/*
	python $(top_srcdir)/libplayercore/playerinterfacegen.py --functiontable $(top_srcdir)/libplayercore/interfaces/ > $(top_builddir)/libplayerxdr/functiontable_gen.c
#	if ! $(top_srcdir)/libplayercore/playerinterfacegen.py --functiontable $(top_srcdir)/libplayercore/interfaces/ > functiontable_gen.c; then rm functiontable_gen.c; fi

# This target duplicates one in libplayercore/Makefile.am, but we need it
# here because libplayerxdr gets built prior to libplayercore
$(top_builddir)/libplayercore/player_interfaces.h: $(top_srcdir)/libplayercore/interfaces/* $(top_srcdir)/libplayercore/playerinterfacegen.py
	python $(top_srcdir)/libplayercore/playerinterfacegen.py $(top_srcdir)/libplayercore/interfaces > $(top_builddir)/libplayercore/player_interfaces.h

playerxdr.h: playerxdr.c

playerxdr.c: playerxdrgen.py $(top_srcdir)/libplayercore/player.h $(top_builddir)/libplayercore/player_interfaces.h
	python $(srcdir)/playerxdrgen.py -distro $(top_srcdir)/libplayercore/player.h playerxdr.c playerxdr.h $(top_builddir)/libplayercore/player_interfaces.h

libplayerxdr_la_SOURCES = playerxdr.c playerxdr.h functiontable.h functiontable.c
# Can't use libplayererror here because of an unresolved circular build
# dependency
#libplayerxdr_la_LDFLAGS = $(PLAYER_VERSION_INFO) -rpath $(libdir) $(top_builddir)/libplayercore/libplayererror.la
libplayerxdr_la_LDFLAGS = $(PLAYER_VERSION_INFO) -rpath $(libdir)

# Can't use libplayererror here because of an unresolved circular build
# dependency
#libplayerxdr_la_DEPENDENCIES = $(top_builddir)/libplayercore/libplayererror.la
