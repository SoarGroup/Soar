noinst_LTLIBRARIES =
if INCLUDE_POSTGIS
noinst_LTLIBRARIES += libpostgis.la
endif
if INCLUDE_VEC2MAP
noinst_LTLIBRARIES += libvec2map.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir) $(LIBPQXX_CFLAGS)

libpostgis_la_SOURCES = postgis.cc dbconn.cc dbconn.h
libvec2map_la_SOURCES = vec2map.cc
