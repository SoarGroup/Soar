BASEDIR_PREFIX = ../../

bin_PROGRAMS = towersSML

towersSML_SOURCES = \
TowersOfHanoi.cpp \
smlTowers.cpp \
smlTowersSoarAgent.cpp

towersSML_LDADD = \
$(BASEDIR_PREFIX)KernelSML/src/libKernelSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la 

INCLUDES = -I$(BASEDIR_PREFIX)ClientSML/include 

CXXFLAGS = $(SOAR_CXXFLAGS)
