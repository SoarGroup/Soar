BASEDIR_PREFIX = ../../
bin_PROGRAMS = towersSML
towersSML_SOURCES = \
Towers.h \
TowersOfHanoi.cpp \
smlTowers.cpp \
smlTowers.h \
smlTowersSoarAgent.cpp \
smlTowersSoarAgent.h
LDADD = \
$(BASEDIR_PREFIX)ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)KernelSML/src/libKernelSML.la \
$(BASEDIR_PREFIX)CommandLineInterface/src/libCommandLineInterface.la \
$(BASEDIR_PREFIX)ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la \
$(BASEDIR_PREFIX)../gSKI/src/libgSKI.la\
$(BASEDIR_PREFIX)../SoarKernel/src/libSoarKernel.la
#$(BASEDIR_PREFIX)../gSKI/lib/libgSKI.so
INCLUDES = -I$(BASEDIR_PREFIX)ClientSML/include 
AM_CXXFLAGS = $(CUSTOM_CXXFLAGS)
