AUTOMAKE_OPTIONS = gnu

noinst_LTLIBRARIES = libClientSML.la

libClientSML_la_SOURCES = \
sml_ClientAgent.cpp\
sml_ClientErrors.cpp\
sml_ClientFloatElement.cpp\
sml_ClientIdentifier.cpp\
sml_ClientIntElement.cpp\
sml_ClientKernel.cpp\
sml_ClientStringElement.cpp\
sml_ClientWMElement.cpp\
sml_ClientWorkingMemory.cpp\
sml_DeltaList.cpp\
sml_ObjectMap.cpp\
sml_OutputDeltaList.cpp

INCLUDES = \
-I../..\
-I../include\
-I../../ElementXML/include\
-I../../ConnectionSML/include\
-I../../../gSKI/include

CXXFLAGS = $(SOAR_CXXFLAGS)
