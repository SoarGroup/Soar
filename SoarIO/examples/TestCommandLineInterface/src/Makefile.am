BASEDIR_PREFIX = ../../../
noinst_PROGRAMS = testcli
testcli_SOURCES = cli_Test.cpp
testcli_LDADD = $(BASEDIR_PREFIX)ClientSML/lib/libClientSML.la $(BASEDIR_PREFIX)ConnectionSML/lib/libConnectionSML.la $(BASEDIR_PREFIX)ElementXML/lib/libElementXML.la $(BASEDIR_PREFIX)../gSKI/lib/libgSKI.so
INCLUDES = -I$(BASEDIR_PREFIX)ClientSML/include -I$(BASEDIR_PREFIX)ElementXML/include -I$(BASEDIR_PREFIX)ConnectionSML/include -I$(BASEDIR_PREFIX)../gSKI/include -I$(BASEDIR_PREFIX)CommandLineInterface/include
