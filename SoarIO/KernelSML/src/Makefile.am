AUTOMAKE_OPTIONS = gnu

lib_LTLIBRARIES = libKernelSML.la 

libKernelSML_la_SOURCES = \
KernelSMLInterface.cpp\
KernelSMLdll.cpp\
sml_AgentListener.cpp\
sml_AgentSML.cpp\
sml_ConnectionManager.cpp\
sml_EventManager.cpp\
sml_KernelListener.cpp\
sml_KernelSML.cpp\
sml_KernelSMLHandlers.cpp\
sml_OutputListener.cpp

INCLUDES = \
-I../.. \
-I../include \
-I../../ClientSML/include \
-I../../ElementXML/include \
-I../../ConnectionSML/include \
-I../../../gSKI/include \
-I../../../TgDI/include \
-I../../CommandLineInterface/include

CXXFLAGS = $(SOAR_CXXFLAGS)
