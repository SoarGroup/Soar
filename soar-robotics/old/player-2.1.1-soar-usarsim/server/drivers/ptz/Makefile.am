noinst_LTLIBRARIES = 
if INCLUDE_SONYEVID30
noinst_LTLIBRARIES += libsonyevid30.la
endif
if INCLUDE_AMTECPOWERCUBE
noinst_LTLIBRARIES += libamtecpowercube.la
endif
if INCLUDE_PTU46
noinst_LTLIBRARIES += libptu46.la
endif
if INCLUDE_CANONVCC4
noinst_LTLIBRARIES += libcanonvcc4.la
endif

AM_CPPFLAGS = -Wall -I$(top_srcdir)

libsonyevid30_la_SOURCES = sonyevid30.cc
libamtecpowercube_la_SOURCES = amtecpowercube.cc
libptu46_la_SOURCES = ptu46.cc
libcanonvcc4_la_SOURCES = canonvcc4.cc


