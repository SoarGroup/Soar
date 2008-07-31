AM_CPPFLAGS = -Wall -g -I$(top_srcdir)

exampledir = $(prefix)/share/player/examples/plugins/multidriver

example_LTLIBRARIES = libmultidriver.la 
libmultidriver_la_SOURCES = multidriver.cc

example_DATA = multidriver.cc multi.cfg Makefile.example Makefile.libtool Makefile.osx.example SConstruct
EXTRA_DIST = multi.cfg Makefile.example Makefile.libtool Makefile.osx.example SConstruct
