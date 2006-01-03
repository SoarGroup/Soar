.PHONY: all clean

all:
	cd UrbanCombatInterface/ && make
	cd UrbanCombatSML/ && make
	g++ UrbanCombatSML/UCTSML.o UrbanCombatInterface/UrbanCombatInterface.o UrbanCombatInterface/UrbanCombatEffector.o UrbanCombatInterface/UrbanCombatPercepts.o -lSoarKernelSML -lElementXML -o UCTSML

clean:
	rm -f *.o *.out UCTSML
	cd UrbanCombatInterface/ && make clean
	cd UrbanCombatSML/ && make clean
