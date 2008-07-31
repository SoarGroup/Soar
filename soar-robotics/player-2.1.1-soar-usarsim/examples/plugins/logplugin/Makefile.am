AM_CPPFLAGS = -Wall -g -I$(top_srcdir)

exampledir = $(prefix)/share/player/examples/plugins/logplugin

example_LTLIBRARIES = liblogplugin.la 
liblogplugin_la_SOURCES = logplugin.cc

example_DATA = logplugin.cc Makefile.example Makefile.libtool Makefile.osx.example log.cfg
EXTRA_DIST = Makefile.example Makefile.libtool Makefile.osx.example log.cfg
