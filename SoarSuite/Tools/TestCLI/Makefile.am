###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX = ../..

bin_PROGRAMS = TestCLI

TestCLI_SOURCES = cli_Test.cpp cli_Test.h

TestCLI_LDADD = \
$(BASEDIR_PREFIX)/Core/KernelSML/src/libSoarKernelSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la \
$(BASEDIR_PREFIX)/Core/ClientSML/src/libClientSML.la

INCLUDES = \
-I$(BASEDIR_PREFIX)/Core/ClientSML/include \
-I$(BASEDIR_PREFIX)/Core/ElementXML/include \
-I$(BASEDIR_PREFIX)/Core/ConnectionSML/include

CXXFLAGS = $(SOAR_CXXFLAGS)
