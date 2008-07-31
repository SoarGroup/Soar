#
# Here we do a little hack to copy all the driver object files into one
# directory and put them together into an archive lib.  The java
# libplayercore bindings also refer to *.o in this directory in order to
# suck in all the driver objects.
#

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = playerdrivers.pc
CLEANFILES = playerdrivers.pc

lib_LTLIBRARIES = libplayerdrivers.la

AM_CPPFLAGS = -Wall -I$(top_srcdir)

libplayerdrivers_la_SOURCES = driverregistry.cc driverregistry.h
libplayerdrivers_la_LIBADD = @PLAYER_DRIVER_EXTRA_LIBS@ $(top_builddir)/libplayercore/libplayercore.la $(top_builddir)/libplayercore/libplayererror.la

# This is ugly, but it works.  
# We link with *.lo in server/drivers, which
# are the libtool objects built from each driver.  We depend on *.c* and
# *.h in the same directory, which are the sources for the drivers.  This
# means that we depend on source for some drivers that we don't build, but
# that doesn't matter.
libplayerdrivers_la_LDFLAGS = `find $(top_builddir)/server/drivers -name "*.lo"` $(PLAYER_VERSION_INFO) -rpath $(libdir)
libplayerdrivers_la_DEPENDENCIES = $(shell find $(top_builddir)/server/drivers -name "*.c*") $(shell find $(top_builddir)/server/drivers -name "*.h*")

if INCLUDE_JPEG
libplayerdrivers_la_LIBADD += $(top_builddir)/libplayerjpeg/libplayerjpeg.la
endif

pkgincludedir = $(includedir)/player-$(PLAYER_MAJOR_VERSION).$(PLAYER_MINOR_VERSION)/libplayerdrivers
pkginclude_HEADERS = driverregistry.h
