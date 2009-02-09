AM_CPPFLAGS = -g -Wall -D PLAYER_INSTALL_PREFIX="\"@PREFIX@\"" -I$(top_srcdir)

# create the pkg-config entry for the server headers
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = playercore.pc playererror.pc playerutils.pc

dist_bin_SCRIPTS = playerinterfacegen.py

EXTRA_DIST = playercore.pc.in playererror.pc.in playerutils.pc.in interfaces/ADDING_INTERFACES $(shell find $(srcdir)/interfaces -name "*.def")

BUILT_SOURCES = playerconfig.h player_interfaces.h interface_table.c
CLEANFILES = playercore.pc playererror.pc playerutils.pc
DISTCLEANFIELS = $BUILT_SOURCES 

lib_LTLIBRARIES = libplayererror.la libplayerutils.la libplayercore.la

libplayercore_la_SOURCES = driver.cc driver.h \
                          device.cc device.h \
                          drivertable.h drivertable.cc \
                          devicetable.h devicetable.cc \
                          configfile.cc configfile.h \
                          playercommon.h player.h playerconfig.h.in \
                          message.h message.cc \
                          filewatcher.cc filewatcher.h \
                          wallclocktime.cc wallclocktime.h playertime.h \
                          plugins.cc plugins.h \
                          globals.cc globals.h \
                          property.cpp property.h
libplayercore_la_LDFLAGS = $(PLAYER_VERSION_INFO) -rpath $(libdir) $(top_builddir)/libplayercore/libplayererror.la $(top_builddir)/libplayercore/libplayerutils.la
libplayercore_la_DEPENDENCIES = $(top_builddir)/libplayercore/libplayererror.la $(top_builddir)/libplayercore/libplayerutils.la $(top_builddir)/libplayercore/player_interfaces.h

libplayererror_la_SOURCES = error.h error.c
libplayererror_la_LDFLAGS = $(PLAYER_VERSION_INFO) -rpath $(libdir)

libplayerutils_la_SOURCES = interface_util.c interface_util.h \
                            addr_util.c addr_util.h 
libplayerutils_la_LDFLAGS = $(PLAYER_VERSION_INFO) -rpath $(libdir)
libplayerutils_la_DEPENDENCIES = interface_table.c


coreincludedir = $(includedir)/player-$(PLAYER_MAJOR_VERSION).$(PLAYER_MINOR_VERSION)/libplayercore
coreinclude_HEADERS = configfile.h \
                      device.h \
                      devicetable.h \
                      driver.h \
                      drivertable.h \
                      filewatcher.h \
                      globals.h \
                      interface_util.h \
                      message.h \
                      player.h \
                      playercommon.h \
                      playerconfig.h \
                      playercore.h \
                      playertime.h \
                      plugins.h \
                      wallclocktime.h \
                      error.h \
                      addr_util.h \
                      property.h \
                      player_interfaces.h
                      
player_interfaces.h: $(srcdir)/interfaces/* playerinterfacegen.py
	python $(srcdir)/playerinterfacegen.py > $(builddir)/player_interfaces.h
#	if ! ./playerinterfacegen.py > player_interfaces.h; then rm player_interfaces.h; fi

interface_table.c: $(srcdir)/interfaces/* playerinterfacegen.py
	python $(srcdir)/playerinterfacegen.py --utils > $(top_builddir)/libplayercore/interface_table.c
#	if ! ./playerinterfacegen.py --utils > interface_table.c; then rm interface_table.c; fi

