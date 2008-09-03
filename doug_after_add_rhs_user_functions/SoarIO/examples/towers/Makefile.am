BASEDIR_PREFIX = ../../
bin_PROGRAMS = towers
towers_SOURCES = \
gSKITowers.cpp \
gSKITowersSoarAgent.cpp \
Towers.h \
TowersOfHanoi.cpp \
gSKITowers.h \
gSKITowersSoarAgent.h
LDADD = \
$(BASEDIR_PREFIX)ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)KernelSML/src/libKernelSML.la \
$(BASEDIR_PREFIX)CommandLineInterface/src/libCommandLineInterface.la \
$(BASEDIR_PREFIX)ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la \
$(BASEDIR_PREFIX)../TgDI/src/libTgDI.la \
$(BASEDIR_PREFIX)../gSKI/src/libgSKI.la\
$(BASEDIR_PREFIX)../SoarKernel/src/libSoarKernel.la
INCLUDES = \
-I$(BASEDIR_PREFIX)../TgDI/include \
-I$(BASEDIR_PREFIX)ConnectionSML/include \
-I$(BASEDIR_PREFIX)CommandLineInterface/include \
-I$(BASEDIR_PREFIX)../gSKI/src \
-I$(BASEDIR_PREFIX)../gSKI/include
CXXFLAGS = $(SOAR_CXXFLAGS)
