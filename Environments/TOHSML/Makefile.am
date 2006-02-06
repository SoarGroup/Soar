###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX = ../..

bin_PROGRAMS = TOHSML

TOHSML_SOURCES = TowersOfHanoi.cpp smlTowers.cpp smlTowersSoarAgent.cpp

TOHSML_LDADD = \
$(BASEDIR_PREFIX)/Core/KernelSML/src/libSoarKernelSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la 

INCLUDES = -I$(BASEDIR_PREFIX)/Core/ClientSML/include 

CXXFLAGS = $(SOAR_CXXFLAGS)
