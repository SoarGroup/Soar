BASEDIR_PREFIX = ../../../
bin_PROGRAMS = testcli
testcli_SOURCES = cli_Test.cpp
LDADD = \
$(BASEDIR_PREFIX)ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)KernelSML/src/libKernelSML.la \
$(BASEDIR_PREFIX)CommandLineInterface/src/libCommandLineInterface.la \
$(BASEDIR_PREFIX)ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la \
$(BASEDIR_PREFIX)../gSKI/lib/libgSKI.so
INCLUDES = -I$(BASEDIR_PREFIX)ClientSML/include -I$(BASEDIR_PREFIX)ElementXML/include -I$(BASEDIR_PREFIX)ConnectionSML/include -I$(BASEDIR_PREFIX)../gSKI/include -I$(BASEDIR_PREFIX)CommandLineInterface/include
AM_CXXFLAGS = $(CUSTOM_CXXFLAGS)
