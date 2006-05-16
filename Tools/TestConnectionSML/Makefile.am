###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX = ../..

bin_PROGRAMS = TestConnectionSML

TestConnectionSML_SOURCES = TestConnectionSML.cpp

if STATIC_LINKED_CONDITIONAL
TestConnectionSML_LDADD = \
$(BASEDIR_PREFIX)/Core/ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la \
$(BASEDIR_PREFIX)/Core/KernelSML/src/libSoarKernelSML.la
else
TestConnectionSML_LDADD = \
$(BASEDIR_PREFIX)/Core/ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la
endif

INCLUDES = \
-I$(BASEDIR_PREFIX)/Core/ConnectionSML/include \
-I$(BASEDIR_PREFIX)/Core/ElementXML/include \
-I$(BASEDIR_PREFIX)/Core/shared/include

CXXFLAGS = $(SOAR_CXXFLAGS)
