LCM = lcm-gen

LCMTYPES_LCM = $(shell ls *.lcm)
LCMTYPES_JAVA = $(LCMTYPES_LCM:%.lcm=%.java)
LCMTYPES_CLAS = $(LCMTYPES_LCM:%.lcm=%.class)

SUFFIXES += .java .lcm

all:	$(LCMTYPES_JAVA) 


%.java: %.lcm
	$(LCM) -j --jpath=../src $<
