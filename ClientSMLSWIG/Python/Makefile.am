###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX=../../..

EXTRA_DIST = Python_sml_ClientInterface.i

if SWIGPYTHON
lib_LTLIBRARIES = libPython_sml_ClientInterface.la
else
lib_LTLIBRARIES =
endif

libPython_sml_ClientInterface_la_SOURCES = Python_sml_ClientInterface_wrap.cpp 

libPython_sml_ClientInterface_la_LIBADD = \
$(BASEDIR_PREFIX)/Core/ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)/Core/ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la

INCLUDES   = -I../../ClientSML/include -I../../ElementXML/include -I../../ConnectionSML/include

SWIG=@SWIG@
CXXFLAGS = $(SOAR_CXXFLAGS) $(SWIG_PYTHON_CPPFLAGS) -Wno-unused -fno-strict-aliasing
BINDIR=@bindir@
SO=@SO@

SWIGOPT    = -python -Wall $(INCLUDES)

Python_sml_ClientInterface_wrap.cpp: Python_sml_ClientInterface.i
	$(SWIG) $(SWIGOPT) -o $@ $<

clean:
	rm -f Python_sml_ClientInterface_wrap.*
	rm -f Python_sml_ClientInterface.py

install-data-hook:
if SWIGPYTHON
	mv Python_sml_ClientInterface.py $(BINDIR)
endif
