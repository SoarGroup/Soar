AM_CPPFLAGS = -Wall -g -I$(top_srcdir)

exampledir = $(prefix)/share/player/examples/plugins/exampledriver

example_LTLIBRARIES = libexampledriver.la 
libexampledriver_la_SOURCES = exampledriver.cc

example_DATA = exampledriver.cc Makefile.example Makefile.libtool Makefile.osx.example example.cfg SConstruct
EXTRA_DIST = Makefile.example Makefile.libtool Makefile.osx.example example.cfg SConstruct
