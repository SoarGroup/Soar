###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2005
###########################################
BASEDIR_PREFIX = ../../

bin_PROGRAMS = TestSoarPerformance

TestSoarPerformance_SOURCES= TestSoarPerformance.cpp

TestSoarPerformance_LDADD = \
$(BASEDIR_PREFIX)KernelSML/src/libSoarKernelSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la

INCLUDES = \
-I$(BASEDIR_PREFIX)ClientSML/include \
-I$(BASEDIR_PREFIX)ElementXML/include \
-I$(BASEDIR_PREFIX)ConnectionSML/include \
-I$(BASEDIR_PREFIX)../gSKI/include \
-I$(BASEDIR_PREFIX)CommandLineInterface/include

AM_CXXFLAGS = $(CUSTOM_CXXFLAGS)
