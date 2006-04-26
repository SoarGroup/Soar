###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX = ../..

bin_PROGRAMS = TOHSML

TOHSML_SOURCES = TowersOfHanoi.cpp smlTowers.cpp smlTowersSoarAgent.cpp\
Towers.h\
smlTowers.h\
smlTowersSoarAgent.h

TOHSML_LDADD = \
$(BASEDIR_PREFIX)/Core/ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)/Core/ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la

INCLUDES = \
-I$(BASEDIR_PREFIX)/Core/ClientSML/include \
-I$(BASEDIR_PREFIX)/Core/ConnectionSML/include

CXXFLAGS = $(SOAR_CXXFLAGS)
