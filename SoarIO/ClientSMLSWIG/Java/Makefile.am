BASEDIR_PREFIX=../../
AUTOMAKE_OPTIONS = gnu
EXTRA_DIST = Java_sml_ClientInterface.i
lib_LTLIBRARIES = libJava_sml_ClientInterface.la

libJava_sml_ClientInterface_la_LIBADD = \
$(BASEDIR_PREFIX)ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)KernelSML/src/libKernelSML.la \
$(BASEDIR_PREFIX)CommandLineInterface/src/libCommandLineInterface.la \
$(BASEDIR_PREFIX)ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la \
$(BASEDIR_PREFIX)../TgDI/src/libTgDI.la \
$(BASEDIR_PREFIX)../gSKI/src/libgSKI.la\
$(BASEDIR_PREFIX)../SoarKernel/src/libSoarKernel.la

libJava_sml_ClientInterface_la_SOURCES = Java_sml_ClientInterface_wrap.cpp 

JAVAINC= @JAVAINC@
JAVAC= @JAVAC@
INCLUDES   = -I../../ClientSML/include -I../../ElementXML/include -I../../ConnectionSML/include $(JAVAINC)
CXXFLAGS = $(SOAR_CXXFLAGS)
CLEANFILES = *.java *.class Java_sml_ClientInterface_wrap.cpp

SWIG       = swig@swig_suffix@
SWIGOPT    = -java -c++ -Wall -package sml

Java_sml_ClientInterface_wrap.cpp: Java_sml_ClientInterface.i
	$(SWIG) $(SWIGOPT) -o $@ $<
	$(JAVAC) *.java

clean:
	rm -f Java_sml_ClientInterface_wrap.cpp
	rm -f *.java
