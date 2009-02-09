SUBDIRS = drivers libplayerdrivers .

AM_CPPFLAGS = -Wall -g -I$(top_srcdir) -I$(top_srcdir)/server -I$(srcdir)/drivers/shell

if INCLUDE_RTKGUI
RTKGUI_LIBS=-L$(top_builddir)/rtk2 -lrtk @GTK_LIBS@
else
RTKGUI_LIBS=
endif

player_DEPENDENCIES = $(top_builddir)/server/libplayerdrivers/libplayerdrivers.la $(top_builddir)/libplayercore/libplayercore.la $(top_builddir)/libplayercore/libplayererror.la $(top_builddir)/libplayercore/libplayerutils.la $(top_builddir)/libplayertcp/libplayertcp.la $(top_builddir)/libplayertcp/libplayerudp.la $(top_builddir)/libplayerxdr/libplayerxdr.la
if HAVE_PLAYERSD
player_DEPENDENCIES += $(top_builddir)/libplayersd/libplayersd.la 
endif

# Note to everybody: please do NOT break the following assignment onto
# separate lines with backslashes.  It makes the link line look like shit and
# hard to parse.  - BPG
#
player_LDADD   = -lm $(top_builddir)/server/libplayerdrivers/libplayerdrivers.la $(top_builddir)/libplayercore/libplayercore.la $(top_builddir)/libplayercore/libplayererror.la $(top_builddir)/libplayercore/libplayerutils.la $(top_builddir)/libplayertcp/libplayertcp.la $(top_builddir)/libplayertcp/libplayerudp.la $(top_builddir)/libplayerxdr/libplayerxdr.la @PLAYER_DRIVER_EXTRA_LIBS@ $(RTKGUI_LIBS) $(top_builddir)/replace/libreplace.la @ARTOOLKITPLUS_LIBS@ @GEOS_LIBS@
if HAVE_PLAYERSD
player_LDADD += $(top_builddir)/libplayersd/libplayersd.la 
endif
#player_LDADD   = -lm $(top_builddir)/server/libplayerdrivers/.libs/libplayerdrivers.a $(top_builddir)/libplayercore/.libs/libplayercore.a $(top_builddir)/libplayercore/.libs/libplayererror.a $(top_builddir)/libplayertcp/.libs/libplayertcp.a $(top_builddir)/libplayerxdr/.libs/libplayerxdr.a @PLAYER_DRIVER_EXTRA_LIBS@ $(RTKGUI_LIBS) $(top_builddir)/replace/.libs/libreplace.a

player_LDFLAGS = -export-dynamic

bin_PROGRAMS = player

player_SOURCES = server.cc

