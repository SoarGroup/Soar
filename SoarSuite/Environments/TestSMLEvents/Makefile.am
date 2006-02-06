###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX = ../..

bin_PROGRAMS = TestSMLEvents

TestSMLEvents_SOURCES = TestSMLEvents.cpp

TestSMLEvents_LDADD = \
$(BASEDIR_PREFIX)/Core/KernelSML/src/libSoarKernelSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la

INCLUDES = \
-I$(BASEDIR_PREFIX)/Core/ClientSML/include \
-I$(BASEDIR_PREFIX)/Core/ElementXML/include \
-I$(BASEDIR_PREFIX)/Core/ConnectionSML/include \
-I$(BASEDIR_PREFIX)/Core/KernelSML/include 

CXXFLAGS = $(SOAR_CXXFLAGS)
