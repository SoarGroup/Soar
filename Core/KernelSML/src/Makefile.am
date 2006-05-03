###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
lib_LTLIBRARIES = libSoarKernelSML.la 

libSoarKernelSML_la_LIBADD = \
../../SoarKernel/src/libSoarKernel.la\
../../gSKI/src/libgSKI.la\
../../CLI/src/libCommandLineInterface.la\
../../ConnectionSML/src/libConnectionSML.la\
../../ElementXML/src/libElementXML.la

libSoarKernelSML_la_SOURCES = \
KernelSMLInterface.cpp\
KernelSMLdll.cpp\
sml_AgentListener.cpp\
sml_AgentOutputFlusher.cpp\
sml_AgentSML.cpp\
sml_ConnectionManager.cpp\
sml_ListenerThread.cpp\
sml_ReceiverThread.cpp\
sml_KernelSML.cpp\
sml_KernelSMLHandlers.cpp\
sml_OutputListener.cpp\
sml_PrintListener.cpp\
sml_ProductionListener.cpp\
sml_RhsListener.cpp\
sml_RunListener.cpp\
sml_RunScheduler.cpp\
sml_SystemListener.cpp\
sml_UntypedListener.cpp\
sml_UpdateListener.cpp\
sml_XMLListener.cpp\
../include/KernelSMLdll.h\
../include/sml_AgentListener.h\
../include/sml_AgentOutputFlusher.h\
../include/sml_AgentSML.h\
../include/sml_ConnectionManager.h\
../include/sml_EventManager.h\
../include/sml_KernelSML.h\
../include/sml_ListenerThread.h\
../include/sml_OldRunScheduler.h\
../include/sml_OutputListener.h\
../include/sml_PrintListener.h\
../include/sml_ProductionListener.h\
../include/sml_ReceiverThread.h\
../include/sml_RhsListener.h\
../include/sml_RunListener.h\
../include/sml_RunScheduler.h\
../include/sml_SystemListener.h\
../include/sml_UntypedListener.h\
../include/sml_UpdateListener.h\
../include/sml_XMLListener.h

INCLUDES = \
-I../include \
-I../../ElementXML/include \
-I../../ConnectionSML/include \
-I../../gSKI/include \
-I../../CLI/include \
-I../../shared/include

CXXFLAGS = $(SOAR_CXXFLAGS)
