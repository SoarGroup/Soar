TOP        = ..
SWIG       = /usr/local/bin/swig
CXXSRCS    = Java_sml_ClientInterface.cxx
TARGET     = Java_sml_ClientInterface
INTERFACE  = Java_sml_ClientInterface.i
SWIGOPT    = -Wall -package sml
# -c++ -java

all::	java

java::
	$(MAKE) -f $(TOP)/Makefile CXXSRCS='$(CXXSRCS)' SWIG='$(SWIG)' \
	SWIGOPT='$(SWIGOPT)' TARGET='$(TARGET)' INTERFACE='$(INTERFACE)' java_cpp
	javac *.java

clean::
	$(MAKE) -f $(TOP)/Makefile java_clean

check: all
