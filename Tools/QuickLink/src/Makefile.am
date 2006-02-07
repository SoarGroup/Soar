###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX = ../../..

bin_PROGRAMS = QuickLink

QuickLink_SOURCES = QuickLink.cpp Reader.cpp

QuickLink_LDADD = \
$(BASEDIR_PREFIX)/Core/KernelSML/src/libSoarKernelSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la

INCLUDES = \
-I../include \
-I$(BASEDIR_PREFIX)/Core/ClientSML/include

CXXFLAGS = $(SOAR_CXXFLAGS)
