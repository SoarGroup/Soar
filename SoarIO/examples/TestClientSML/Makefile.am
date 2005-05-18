BASEDIR_PREFIX = ../../

bin_PROGRAMS = TestClientSML

TestClientSML_SOURCES = TestClientSML.cpp

TestClientSML_LDADD = \
$(BASEDIR_PREFIX)KernelSML/src/libSoarKernelSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la

INCLUDES = \
-I$(BASEDIR_PREFIX)ClientSML/include \
-I$(BASEDIR_PREFIX)ConnectionSML/include \
-I$(BASEDIR_PREFIX)ElementXML/include \
-I$(BASEDIR_PREFIX)KernelSML/include

CXXFLAGS = $(SOAR_CXXFLAGS)
