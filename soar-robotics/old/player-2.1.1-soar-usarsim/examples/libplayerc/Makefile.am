PLAYERC_CFLAGS = -I$(top_srcdir) \
		 -I$(top_builddir)/client_libs/ \
                 -I$(top_srcdir)/client_libs/

PLAYERC_LIB_DIRS=-L${top_builddir}/libplayerxdr \
                 -L${top_builddir}/libplayercore \
                 -L${top_builddir}/libplayersd \
                 -L${top_builddir}/client_libs/libplayerc

PLAYERC_DEPS = $(top_builddir)/client_libs/libplayerc/libplayerc.la \
               $(top_builddir)/libplayerxdr/libplayerxdr.la \
               $(top_builddir)/libplayercore/libplayererror.la

AM_CPPFLAGS = $(PLAYERC_CFLAGS) -Wall -g

exampledir = $(prefix)/share/player/examples/libplayerc

example_PROGRAMS = vmap simple speech_c_client
if HAVE_PLAYERSD
example_PROGRAMS += service_discovery
endif
example_DATA = vmap.c simple.c speech_c_client.c service_discovery.c

vmap_SOURCES = vmap.c
vmap_LDADD = -lplayerc $(PLAYERC_LIB_DIRS) -lm -lplayerxdr -lplayererror
vmap_DEPENDENCIES = $(PLAYERC_DEPS)

simple_SOURCES = simple.c
simple_LDADD = -lplayerc $(PLAYERC_LIB_DIRS) -lm -lplayerxdr -lplayererror
simple_DEPENDENCIES = $(PLAYERC_DEPS)

speech_c_client_SOURCES = speech_c_client.c
speech_c_client_LDADD = -lplayerc $(PLAYERC_LIB_DIRS) -lm -lplayerxdr -lplayererror
speech_c_client_DEPENDENCIES = $(PLAYERC_DEPS)

service_discovery_SOURCES = service_discovery.c
service_discovery_LDADD = -lplayerc $(PLAYERC_LIB_DIRS) -lm -lplayerxdr -lplayererror -lplayersd -lplayerutils
service_discovery_DEPENDENCIES = $(PLAYERC_DEPS) $(top_builddir)/libplayersd/libplayersd.la
