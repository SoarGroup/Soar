BASEDIR_PREFIX=../../

AUTOMAKE_OPTIONS = gnu

EXTRA_DIST = Tcl_sml_ClientInterface.i

lib_LTLIBRARIES = libTcl_sml_ClientInterface.la

libTcl_sml_ClientInterface_la_SOURCES = Tcl_sml_ClientInterface_wrap.cpp 

libTcl_sml_ClientInterface_la_LIBADD = \
$(BASEDIR_PREFIX)KernelSML/src/libKernelSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la

INCLUDES   = -I../../ClientSML/include -I../../ElementXML/include -I../../ConnectionSML/include

CXXFLAGS = $(SOAR_CXXFLAGS) -Wno-unused

SWIG       = swig@swig_suffix@
SWIGOPT    = -tcl -c++ -Wall -pkgversion 1.0

Tcl_sml_ClientInterface_wrap.cpp: Tcl_sml_ClientInterface.i
	$(SWIG) $(SWIGOPT) -o $@ $<

clean:
	rm -f Tcl_sml_ClientInterface_wrap.cpp
