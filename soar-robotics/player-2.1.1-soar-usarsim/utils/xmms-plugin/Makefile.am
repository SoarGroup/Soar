AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)/client_libs -I$(top_srcdir)/client_libs

lib_LTLIBRARIES =
if HAVE_XMMS
lib_LTLIBRARIES += libplayerxmms.la
endif

libplayerxmms_la_CFLAGS = `xmms-config --cflags` 
libplayerxmms_la_LDFLAGS = `xmms-config --libs` $(top_builddir)/client_libs/libplayerc/libplayerc.la $(top_builddir)/libplayerxdr/libplayerxdr.la $(top_builddir)/libplayercore/libplayererror.la
libplayerxmms_la_SOURCES = playerxmms.c

