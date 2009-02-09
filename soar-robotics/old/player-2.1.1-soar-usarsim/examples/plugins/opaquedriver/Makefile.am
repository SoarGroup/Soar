PLAYERC_CFLAGS = -I$(top_srcdir) \
                 -I$(top_builddir)/client_libs/ \
		 -I$(top_srcdir)/client_libs/

PLAYERC_LIB_DIRS=-L${top_builddir}/libplayerxdr \
                 -L${top_builddir}/libplayercore \
                 -L${top_builddir}/client_libs/libplayerc

PLAYERC_DEPS = $(top_builddir)/client_libs/libplayerc/libplayerc.la \
               $(top_builddir)/libplayerxdr/libplayerxdr.la \
               $(top_builddir)/libplayercore/libplayererror.la

AM_CPPFLAGS = $(PLAYERC_CFLAGS) -Wall -g -I$(top_srcdir)

exampledir = $(prefix)/share/player/examples/plugins/opaquedriver

example_LTLIBRARIES = libopaquedriver.la
example_PROGRAMS = opaque_client
example_DATA = opaquedriver.cc opaque.cfg
EXTRA_DIST = opaque.cfg

libopaquedriver_la_SOURCES = opaquedriver.cc sharedstruct.h

opaque_client_SOURCES = opaque.c sharedstruct.h
opaque_client_LDADD = $(PLAYERC_LIB_DIRS) -lplayerc -lm -lplayerxdr -lplayererror
opaque_client_DEPENDENCIES = $(PLAYERC_DEPS)
