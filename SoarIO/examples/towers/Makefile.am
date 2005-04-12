AUTOMAKE_OPTIONS = foreign

BASEDIR_PREFIX = ../../

bin_PROGRAMS = towers

towers_SOURCES = \
gSKITowers.cpp \
gSKITowersSoarAgent.cpp \
TowersOfHanoi.cpp 

towers_LDADD = \
$(BASEDIR_PREFIX)KernelSML/src/libKernelSML.la \
$(BASEDIR_PREFIX)ElementXML/src/libElementXML.la

INCLUDES = \
-I$(BASEDIR_PREFIX)ConnectionSML/include \
-I$(BASEDIR_PREFIX)CommandLineInterface/include \
-I$(BASEDIR_PREFIX)../gSKI/src \
-I$(BASEDIR_PREFIX)../gSKI/include

CXXFLAGS = $(SOAR_CXXFLAGS)
