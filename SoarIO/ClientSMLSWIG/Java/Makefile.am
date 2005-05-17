BASEDIR_PREFIX=../../
SOAR_LIBRARY=$(BASEDIR_PREFIX)../soar-library
AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = Java_sml_ClientInterface.i

if SWIGJAVA
lib_LTLIBRARIES = libJava_sml_ClientInterface.la
else
lib_LTLIBRARIES = 
endif

libJava_sml_ClientInterface_la_LIBADD = \
$(BASEDIR_PREFIX)KernelSML/src/libSoarKernelSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la 

libJava_sml_ClientInterface_la_SOURCES = Java_sml_ClientInterface_wrap.cpp 

JAVASO = @JAVASO@
libJava_sml_ClientInterface_la_LDFLAGS = -shrext $(JAVASO)

SWIG=@SWIG@
JAVAINC= @JAVAINC@
JAVAC= @JAVAC@
JAR=@JAR@
INCLUDES   = -I../../ClientSML/include -I../../ElementXML/include -I../../ConnectionSML/include $(JAVAINC)
CXXFLAGS = $(SOAR_CXXFLAGS) -Wno-unused
CLEANFILES = build/*.java build/*.class Java_sml_ClientInterface_wrap.cpp

SWIGOPT    = -java -Wall -package sml $(INCLUDES) -outdir build

Java_sml_ClientInterface_wrap.cpp: Java_sml_ClientInterface.i
	$(SWIG) $(SWIGOPT) -o $@ $<
	$(JAVAC) build/*.java
	cp -a build $(SOAR_LIBRARY)/sml
	$(JAR) cvf $(SOAR_LIBRARY)/sml.jar -C $(SOAR_LIBRARY) sml
	rm -rf $(SOAR_LIBRARY)/sml

clean:
	rm -f Java_sml_ClientInterface_wrap.cpp
	rm -f build/*.java
	rm -f build/*.class
