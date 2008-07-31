bin_PROGRAMS =
if INCLUDE_PLAYERCC
 bin_PROGRAMS += playerprint
endif

PLAYERCC_CFLAGS = -I$(top_srcdir) \
                  -I$(top_builddir)/client_libs/ \
                  -I$(top_srcdir)/client_libs/ \
                  $(PLAYERCC_EXTRA_CFLAGS)

PLAYERCC_LIB_DIRS=-L${top_builddir}/libplayerxdr \
                  -L${top_builddir}/client_libs/libplayerc \
                  -L${top_builddir}/libplayercore \
                  -L${top_builddir}/client_libs/libplayerc++

PLAYERCC_DEPS = $(top_builddir)/client_libs/libplayerc/libplayerc.la \
                $(top_builddir)/libplayercore/libplayererror.la \
                $(top_builddir)/libplayerxdr/libplayerxdr.la \
                $(top_builddir)/client_libs/libplayerc++/libplayerc++.la

playerprint_CPPFLAGS     = $(PLAYERCC_CFLAGS)
playerprint_LDADD        = $(PLAYERCC_LIB_DIRS) $(PLAYERCC_LIB) -lplayerc++ -lplayerc -lplayerxdr -lplayererror
playerprint_DEPENDENCIES = $(PLAYERCC_DEPS)

playerprint_SOURCES = playerprint.cc
