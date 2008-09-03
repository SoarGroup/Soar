BASEDIR_PREFIX = ../../
bin_PROGRAMS = TestThreadLibrary
TestThreadLibrary_SOURCES = TestThreadLibrary.cpp
LDADD = \
$(BASEDIR_PREFIX)ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)KernelSML/src/libKernelSML.la \
$(BASEDIR_PREFIX)CommandLineInterface/src/libCommandLineInterface.la \
$(BASEDIR_PREFIX)ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la \
$(BASEDIR_PREFIX)../gSKI/src/libgSKI.la\
$(BASEDIR_PREFIX)../SoarKernel/src/libSoarKernel.la
INCLUDES = \
-I$(BASEDIR_PREFIX)ConnectionSML/include \
-I$(BASEDIR_PREFIX)ElementXML/include 
CXXFLAGS = $(SOAR_CXXFLAGS)
