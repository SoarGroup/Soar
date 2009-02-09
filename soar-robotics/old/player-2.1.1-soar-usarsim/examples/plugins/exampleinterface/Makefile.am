PLAYERC_CFLAGS = -I$(top_srcdir) \
                 -I$(top_builddir)/client_libs/ \
                 -I$(top_srcdir)/client_libs/

PLAYERC_LIB_DIRS=-L$(top_builddir)/libplayerxdr \
                 -L$(top_builddir)/libplayercore \
                 -L$(top_builddir)/client_libs/libplayerc

PLAYERC_DEPS = $(top_builddir)/client_libs/libplayerc/libplayerc.la \
               $(top_builddir)/libplayerxdr/libplayerxdr.la \
               $(top_builddir)/libplayercore/libplayererror.la

AM_CPPFLAGS = -Wall -g $(PLAYERC_CFLAGS)


example_interface.h: 128_example.def
	python $(top_srcdir)/libplayercore/playerinterfacegen.py --plugin $< > $@
	
example_functiontable.c: 128_example.def
	python $(top_srcdir)/libplayercore/playerinterfacegen.py --plugin --functiontable $< > $@

example_xdr.c: $(srcdir)/example_interface.h $(top_srcdir)/libplayerxdr/playerxdrgen.py
	python $(top_srcdir)/libplayerxdr/playerxdrgen.py $(srcdir)/example_interface.h example_xdr.c example_xdr.h

example_xdr.h: $(srcdir)/example_interface.h $(top_srcdir)/libplayerxdr/playerxdrgen.py
	python $(top_srcdir)/libplayerxdr/playerxdrgen.py $(srcdir)/example_interface.h example_xdr.c example_xdr.h

BUILT_SOURCES = example_interface.h example_functiontable.c example_xdr.c example_xdr.h
CLEANFILES = $(BUILT_SOURCES)

exampledir = $(prefix)/share/player/examples/plugins/exampleinterface

example_PROGRAMS = example_client
example_DATA = eginterf_client.c example_functiontable.c eginterf_driver.cc 
example_HEADERS = example_interface.h example_xdr.h
EXTRA_DIST = Makefile.example example.cfg eginterf_client.h 128_example.def Makefile.example example.cfg


example_LTLIBRARIES = libeginterfdriver.la libeginterf.la
libeginterfdriver_la_SOURCES = example_interface.h example_xdr.h  eginterf_driver.cc
libeginterf_la_SOURCES = example_interface.h example_xdr.h eginterf_client.c example_functiontable.c example_xdr.c

example_client_SOURCES = example_client.c
example_client_LDADD = -lplayerc $(PLAYERC_LIB_DIRS) -lplayerxdr -lplayererror libeginterf.la
example_client_DEPENDENCIES = $(PLAYERC_DEPS) example_interface.h
