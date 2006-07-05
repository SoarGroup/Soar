###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX = ../..

bin_PROGRAMS = TestMultiAgent

TestMultiAgent_SOURCES = TestMultiAgent.cpp

if STATIC_LINKED_CONDITIONAL
TestMultiAgent_LDADD = \
$(BASEDIR_PREFIX)/Core/ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)/Core/ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la \
$(BASEDIR_PREFIX)/Core/KernelSML/src/libSoarKernelSML.la
else
TestMultiAgent_LDADD = \
$(BASEDIR_PREFIX)/Core/ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)/Core/ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la
endif

INCLUDES = \
-I$(BASEDIR_PREFIX)/Core/ClientSML/include \
-I$(BASEDIR_PREFIX)/Core/ConnectionSML/include \
-I$(BASEDIR_PREFIX)/Core/ElementXML/include

CXXFLAGS = $(SOAR_CXXFLAGS)
