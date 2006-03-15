###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX = ../../..

bin_PROGRAMS = QuickLink

QuickLink_SOURCES = Input_Controller.cpp QL_Interface.cpp View_Console.cpp WME_Id.cpp WME_String.cpp Input_Console.cpp Input_File.cpp QL_Controller.cpp Utilities.cpp WME_Float.cpp WME_Int.cpp WME_Type.cpp WME_Shared.cpp\
../include/Input_Console.h\
../include/Input_Controller.h\
../include/Input_File.h\
../include/Input_Type.h\
../include/QL_Interface.h\
../include/Smart_Pointer.h\
../include/Utilities.h\
../include/View_Console.h\
../include/View_Type.h\
../include/WME_Float.h\
../include/WME_Id.h\
../include/WME_Int.h\
../include/WME_Shared.h\
../include/WME_String.h\
../include/WME_Type.h

QuickLink_LDADD = \
$(BASEDIR_PREFIX)/Core/KernelSML/src/libSoarKernelSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la

INCLUDES = \
-I../include \
-I$(BASEDIR_PREFIX)/Core/ClientSML/include

CXXFLAGS = $(SOAR_CXXFLAGS)
