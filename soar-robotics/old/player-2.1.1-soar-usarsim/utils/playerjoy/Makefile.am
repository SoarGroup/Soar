bin_PROGRAMS =
if INCLUDE_PLAYERCC
 bin_PROGRAMS += playerjoy
endif

PLAYERCC_CFLAGS = -I$(top_srcdir) \
                  -I$(top_builddir)/client_libs/ \
                  -I$(top_srcdir)/client_libs/ \
                  $(PLAYERCC_EXTRA_CFLAGS)

PLAYERCC_LIB_DIRS=-L${top_builddir}/libplayerxdr \
                  -L${top_builddir}/libplayercore \
                  -L${top_builddir}/client_libs/libplayerc \
                  -L${top_builddir}/client_libs/libplayerc++

PLAYERCC_DEPS = $(top_builddir)/client_libs/libplayerc/libplayerc.la \
                $(top_builddir)/libplayerxdr/libplayerxdr.la \
                $(top_builddir)/libplayercore/libplayererror.la \
                $(top_builddir)/client_libs/libplayerc++/libplayerc++.la

playerjoy_CPPFLAGS     = $(PLAYERCC_CFLAGS)
playerjoy_LDADD        = $(PLAYERCC_LIB_DIRS) $(PLAYERCC_LIB) -lplayerc -lplayerc++ -lplayerxdr -lplayererror
playerjoy_DEPENDENCIES = $(PLAYERCC_DEPS)


playerjoy_SOURCES = playerjoy.cc

