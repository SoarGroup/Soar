###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
noinst_LTLIBRARIES = libClientSML.la

libClientSML_la_SOURCES = \
sml_ClientAgent.cpp\
sml_ClientErrors.cpp\
sml_ClientFloatElement.cpp\
sml_ClientIdentifier.cpp\
sml_ClientIntElement.cpp\
sml_ClientKernel.cpp\
sml_ClientStringElement.cpp\
sml_ClientTraceXML.cpp\
sml_ClientWMElement.cpp\
sml_ClientWorkingMemory.cpp\
sml_ClientXML.cpp\
sml_ClientAnalyzedXML.cpp\
sml_DeltaList.cpp\
sml_ObjectMap.cpp\
sml_OutputDeltaList.cpp\
../include/sml_Client.h\
../include/sml_ClientAgent.h\
../include/sml_ClientAnalyzedXML.h\
../include/sml_ClientDirect.h\
../include/sml_ClientErrors.h\
../include/sml_ClientEvents.h\
../include/sml_ClientFloatElement.h\
../include/sml_ClientIdentifier.h\
../include/sml_ClientIntElement.h\
../include/sml_ClientKernel.h\
../include/sml_ClientStringElement.h\
../include/sml_ClientTraceXML.h\
../include/sml_ClientWMElement.h\
../include/sml_ClientWorkingMemory.h\
../include/sml_ClientXML.h\
../include/sml_DeltaList.h\
../include/sml_ListMap.h\
../include/sml_ObjectMap.h\
../include/sml_OutputDeltaList.h

INCLUDES = \
-I../include\
-I../../ElementXML/include\
-I../../ConnectionSML/include\
-I../../shared/include

CXXFLAGS = $(SOAR_CXXFLAGS)

install-data-am:
	$(INSTALL) -m 644 .libs/libClientSML.a $(libdir)
	
	
