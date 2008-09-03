###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
lib_LTLIBRARIES = libElementXML.la

libElementXML_la_SOURCES = \
ElementXMLInterface.cpp\
ElementXMLdll.cpp\
sml_ElementXMLImpl.cpp\
sml_ParseXML.cpp\
sml_ParseXMLFile.cpp\
sml_ParseXMLString.cpp\
../include/ElementXMLHandle.h\
../include/ElementXMLInterface.h\
../include/ElementXMLdll.h\
../include/sml_ElementXMLImpl.h\
../include/sml_ParseXML.h\
../include/sml_ParseXMLFile.h\
../include/sml_ParseXMLString.h


INCLUDES = -I../include -I../../shared/include

CXXFLAGS = $(SOAR_CXXFLAGS)
