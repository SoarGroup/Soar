noinst_LTLIBRARIES =

if INCLUDE_LASERTORANGER
noinst_LTLIBRARIES += liblasertoranger.la
endif

if INCLUDE_SONARTORANGER
noinst_LTLIBRARIES += libsonartoranger.la
endif

if INCLUDE_GBXSICKACFR
noinst_LTLIBRARIES += libgbxsickacfr.la
endif

if INCLUDE_URG_NZ
noinst_LTLIBRARIES += liburg_nz.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir)

liblasertoranger_la_SOURCES = toranger.cc lasertoranger.cc

libsonartoranger_la_SOURCES = toranger.cc sonartoranger.cc

libgbxsickacfr_la_SOURCES = gbxsickacfr.cc
libgbxsickacfr_la_CXXFLAGS = @GBXSICKACFR_CFLAGS@
libgbxsickacfr_la_LIBADD = @GBXSICKACFR_LIBS@

liburg_nz_la_SOURCES = urg_nz.cc
liburg_nz_la_CXXFLAGS = @URG_NZ_CFLAGS@
liburg_nz_la_LIBADD = @URG_NZ_LIBS@

