BASEDIR_PREFIX=../../

AUTOMAKE_OPTIONS = gnu

EXTRA_DIST = Tcl_sml_ClientInterface.i

if SWIGTCL
lib_LTLIBRARIES = libTcl_sml_ClientInterface.la
else
lib_LTLIBRARIES =
endif

libTcl_sml_ClientInterface_la_SOURCES = Tcl_sml_ClientInterface_wrap.cpp 

libTcl_sml_ClientInterface_la_LIBADD = \
$(BASEDIR_PREFIX)KernelSML/src/libKernelSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la

INCLUDES   = -I../../ClientSML/include -I../../ElementXML/include -I../../ConnectionSML/include

CXXFLAGS = $(SOAR_CXXFLAGS) -Wno-unused

SWIGOPT    = -tcl -Wall -pkgversion 1.0 $(INCLUDES)

Tcl_sml_ClientInterface_wrap.cpp: Tcl_sml_ClientInterface.i
	$(SWIG) $(SWIGOPT) -o $@ $<

clean:
	rm -f Tcl_sml_ClientInterface_wrap.cpp

if SWIGTCL
install-data-hook:
	$(TCLSH) ../../bin/makeTclSMLPackage.tcl
	mv .libs/pkgIndex.tcl @libdir@
endif
