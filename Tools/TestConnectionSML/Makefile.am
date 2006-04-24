###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX = ../..

bin_PROGRAMS = TestConnectionSML

TestConnectionSML_SOURCES = TestConnectionSML.cpp

TestConnectionSML_LDADD = \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la\
$(BASEDIR_PREFIX)/Core/ConnectionSML/src/libConnectionSML.la

INCLUDES = \
-I$(BASEDIR_PREFIX)/Core/ConnectionSML/include \
-I$(BASEDIR_PREFIX)/Core/ElementXML/include

CXXFLAGS = $(SOAR_CXXFLAGS)
