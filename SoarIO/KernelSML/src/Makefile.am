AUTOMAKE_OPTIONS = gnu

lib_LTLIBRARIES = libKernelSML.la 

libKernelSML_la_LIBADD = \
../../../SoarKernel/src/libSoarKernel.la\
../../../gSKI/src/libgSKI.la\
../../CommandLineInterface/src/libCommandLineInterface.la\
../../ClientSML/src/libClientSML.la

libKernelSML_la_SOURCES = \
KernelSMLInterface.cpp\
KernelSMLdll.cpp\
sml_AgentListener.cpp\
sml_AgentOutputFlusher.cpp\
sml_AgentSML.cpp\
sml_ConnectionManager.cpp\
sml_KernelSML.cpp\
sml_KernelSMLHandlers.cpp\
sml_OutputListener.cpp\
sml_PrintListener.cpp\
sml_ProductionListener.cpp\
sml_RhsListener.cpp\
sml_RunListener.cpp\
sml_SystemListener.cpp

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
