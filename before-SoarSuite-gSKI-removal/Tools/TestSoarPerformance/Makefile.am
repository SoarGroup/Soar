###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX = ../..

bin_PROGRAMS = TestSoarPerformance

TestSoarPerformance_SOURCES= TestSoarPerformance.cpp

if STATIC_LINKED_CONDITIONAL
TestSoarPerformance_LDADD = \
$(BASEDIR_PREFIX)/Core/ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)/Core/ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la \
$(BASEDIR_PREFIX)/Core/KernelSML/src/libSoarKernelSML.la
else
TestSoarPerformance_LDADD = \
$(BASEDIR_PREFIX)/Core/ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)/Core/ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la
endif

INCLUDES = \
-I$(BASEDIR_PREFIX)/Core/ClientSML/include \
-I$(BASEDIR_PREFIX)/Core/ElementXML/include \
-I$(BASEDIR_PREFIX)/Core/ConnectionSML/include \
-I$(BASEDIR_PREFIX)/Core/shared

AM_CXXFLAGS = $(SOAR_CXXFLAGS)
