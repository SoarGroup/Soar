BASEDIR_PREFIX=../../

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = Tcl_sml_ClientInterface.i

if SWIGTCL
lib_LTLIBRARIES = libTcl_sml_ClientInterface.la
else
lib_LTLIBRARIES =
endif

libTcl_sml_ClientInterface_la_SOURCES = Tcl_sml_ClientInterface_wrap.cpp 

libTcl_sml_ClientInterface_la_LIBADD = \
$(BASEDIR_PREFIX)KernelSML/src/libSoarKernelSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la

INCLUDES   = -I../../ClientSML/include -I../../ElementXML/include -I../../ConnectionSML/include

SWIG=@SWIG@
CXXFLAGS = $(SOAR_CXXFLAGS) -Wno-unused
LIBDIR=@libdir@

SWIGOPT    = -tcl -Wall -pkgversion 8.6.0 $(INCLUDES) -outdir build

Tcl_sml_ClientInterface_wrap.cpp: Tcl_sml_ClientInterface.i
	$(SWIG) $(SWIGOPT) -o $@ $<

clean:
	rm -f Tcl_sml_ClientInterface_wrap.cpp

install-data-hook:
if SWIGTCL
	$(TCLSH) ../../../soar-library/makeTclSMLPackage.tcl
	rm -rf $(LIBDIR)/Tcl_sml_ClientInterface
	mkdir $(LIBDIR)/Tcl_sml_ClientInterface
	mv .libs/pkgIndex.tcl $(LIBDIR)/Tcl_sml_ClientInterface
	ln -s $(LIBDIR)/libTcl_sml_ClientInterface.so $(LIBDIR)/Tcl_sml_ClientInterface/libTcl_sml_ClientInterface.so
endif
