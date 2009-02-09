AM_CPPFLAGS = -Wall -I$(top_srcdir) 

noinst_LTLIBRARIES =
if INCLUDE_NOMAD
noinst_LTLIBRARIES += libnomad.la
endif

# CHOOSE A NOMADICS INTERFACE
#
# for connection via Nserver simulator/visualization (good for debugging)
NOMADICS_INTERFACE = Nclient.c

# for direct connection to a robot
#NOMADICS_INTERFACE = direct.c

libnomad_la_SOURCES = \
  Nclient.h \
  ${NOMADICS_INTERFACE} \
  nomad.cc \
  nomad_position.cc \
  nomad_sonar.cc

