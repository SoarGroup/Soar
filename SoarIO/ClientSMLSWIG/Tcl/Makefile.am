BASEDIR_PREFIX=../../
AUTOMAKE_OPTIONS = gnu
EXTRA_DIST = Tcl_sml_ClientInterface.i
lib_LTLIBRARIES = libTcl_sml_ClientInterface.la
libTcl_sml_ClientInterface_la_SOURCES = Tcl_sml_ClientInterface_wrap.cpp 

libTcl_sml_ClientInterface_la_LIBADD = \
$(BASEDIR_PREFIX)ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)KernelSML/src/libKernelSML.la \
$(BASEDIR_PREFIX)CommandLineInterface/src/libCommandLineInterface.la \
$(BASEDIR_PREFIX)ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la \
$(BASEDIR_PREFIX)../TgDI/src/libTgDI.la \
$(BASEDIR_PREFIX)../gSKI/src/libgSKI.la\
$(BASEDIR_PREFIX)../SoarKernel/src/libSoarKernel.la

INCLUDES   = -I../../ClientSML/include -I../../ElementXML/include -I../../ConnectionSML/include
CXXFLAGS = $(SOAR_CXXFLAGS)

SWIG       = swig@swig_suffix@
SWIGOPT    = -tcl -c++ -Wall

Tcl_sml_ClientInterface_wrap.cpp: Tcl_sml_ClientInterface.i
	$(SWIG) $(SWIGOPT) -o $@ $<

clean:
	rm -f Tcl_sml_ClientInterface_wrap.cpp
