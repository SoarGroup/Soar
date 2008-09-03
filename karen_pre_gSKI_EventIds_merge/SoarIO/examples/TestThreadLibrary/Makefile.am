BASEDIR_PREFIX = ../../

bin_PROGRAMS = TestThreadLibrary

TestThreadLibrary_SOURCES = TestThreadLibrary.cpp

TestThreadLibrary_LDADD = \
$(BASEDIR_PREFIX)KernelSML/src/libKernelSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la

INCLUDES = \
-I$(BASEDIR_PREFIX)ConnectionSML/include \
-I$(BASEDIR_PREFIX)ElementXML/include 

CXXFLAGS = $(SOAR_CXXFLAGS)
