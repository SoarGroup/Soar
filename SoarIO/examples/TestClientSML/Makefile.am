BASEDIR_PREFIX = ../../
bin_PROGRAMS = TestClientSML
TestClientSML_SOURCES = TestClientSML.cpp
LDADD = \
$(BASEDIR_PREFIX)ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)KernelSML/src/libKernelSML.la \
$(BASEDIR_PREFIX)CommandLineInterface/src/libCommandLineInterface.la \
$(BASEDIR_PREFIX)ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la \
$(BASEDIR_PREFIX)../gSKI/lib/libgSKI.so
TestClientSML_LDFLAGS = -static
INCLUDES = \
-I$(BASEDIR_PREFIX)ClientSML/include \
-I$(BASEDIR_PREFIX)ConnectionSML/include \
-I$(BASEDIR_PREFIX)ElementXML/include \
-I$(BASEDIR_PREFIX)KernelSML/include
AM_CXXFLAGS = $(CUSTOM_CXXFLAGS)
