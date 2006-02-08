###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX = ../../..

bin_PROGRAMS = QuickLink

QuickLink_SOURCES = Input_Controller.cpp QL_Interface.cpp View_Console.cpp WME_Id.cpp WME_String.cpp Input_Console.cpp Input_File.cpp QL_Controller.cpp Utilities.cpp WME_Float.cpp WME_Int.cpp WME_Type.cpp

QuickLink_LDADD = \
$(BASEDIR_PREFIX)/Core/KernelSML/src/libSoarKernelSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la

INCLUDES = \
-I../include \
-I$(BASEDIR_PREFIX)/Core/ClientSML/include

CXXFLAGS = $(SOAR_CXXFLAGS)
