###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX = ../../..

bin_PROGRAMS = SoarTextIO

SoarTextIO_SOURCES = SoarTextIO.cpp \
../include/SoarTextIO.h

SoarTextIO_LDADD = \
$(BASEDIR_PREFIX)/Core/ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)/Core/ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la

INCLUDES = \
-I../include \
-I$(BASEDIR_PREFIX)/Core/ClientSML/include \
-I$(BASEDIR_PREFIX)/Core/ConnectionSML/include

CXXFLAGS = $(SOAR_CXXFLAGS) -lncurses -lpthread
