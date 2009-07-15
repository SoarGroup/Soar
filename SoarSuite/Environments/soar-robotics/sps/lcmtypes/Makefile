LCM = lcm-gen

LCMTYPES_LCM = $(shell ls *.lcm)
LCMTYPES_JAVA = $(LCMTYPES_LCM:%.lcm=%.java)

SUFFIXES += .java .lcm

all:    $(LCMTYPES_JAVA)


%.java: %.lcm
	$(LCM) -j  --jpath=../src $<

clean:
	rm -f ../src/org/msoar/sps/lcmtypes/*.java
