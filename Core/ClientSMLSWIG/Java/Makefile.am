###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX=../../..
SOAR_LIBRARY=$(BASEDIR_PREFIX)/SoarLibrary
EXTRA_DIST = Java_sml_ClientInterface.i JavaCallbackByHand.h build/readme.txt

if SWIGJAVA
lib_LTLIBRARIES = libJava_sml_ClientInterface.la
else
lib_LTLIBRARIES = 
endif

libJava_sml_ClientInterface_la_LIBADD = \
$(BASEDIR_PREFIX)/Core/KernelSML/src/libSoarKernelSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la 

nodist_libJava_sml_ClientInterface_la_SOURCES = Java_sml_ClientInterface_wrap.cpp 

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
	cp -pRP build $(SOAR_LIBRARY)/bin/sml
	$(JAR) cf $(SOAR_LIBRARY)/bin/sml.jar -C $(SOAR_LIBRARY)/bin sml
	rm -rf $(SOAR_LIBRARY)/bin/sml

clean:
	rm -f Java_sml_ClientInterface_wrap.cpp
	rm -f build/*.java
	rm -f build/*.class
