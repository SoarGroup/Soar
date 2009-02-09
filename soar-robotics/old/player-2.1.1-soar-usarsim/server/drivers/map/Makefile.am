AM_CPPFLAGS = -Wall -I$(top_srcdir) @GDK_PIXBUF_CFLAGS@

noinst_LTLIBRARIES = 
if INCLUDE_MAPFILE
noinst_LTLIBRARIES += libmapfile.la 
endif
if INCLUDE_MAPCSPACE
noinst_LTLIBRARIES += libmapcspace.la
endif
if INCLUDE_MAPSCALE
noinst_LTLIBRARIES += libmapscale.la
endif
if INCLUDE_VMAPFILE
noinst_LTLIBRARIES += libvmapfile.la
endif

libvmapfile_la_SOURCES = vmapfile.cc 
libmapfile_la_SOURCES = mapfile.cc 
libmapcspace_la_SOURCES = maptransform.cc mapcspace.cc maptransform.h
libmapscale_la_SOURCES = maptransform.cc mapscale.cc maptransform.h
