# Top level make file for the SoarKernel and its test suite

# This target is responsible for making the libsoarkernel.a binary
# and copying it to bin
libsoar:
	$(MAKE) -e -C src libsoarkernel.a
	cp -u src/libsoarkernel.a lib/

# This target makes the mainKernelTest binary and copies
test: libsoar
	$(MAKE) -e -C app_src/SoarKernelTest mainKernelTest
	cp -u app_src/SoarKernelTest/mainKernelTest bin

# This target makes the unit test binary 
unittest: libsoar cppunit-install
	$(MAKE) -e -C SoarKernelUnitTests
	cp -u SoarKernelUnitTests/unitTest bin

# Makes the unit testing framework
cppunit-install:
	cd cppunit && ./configure --prefix=`pwd`/../cppunit-install
	$(MAKE) -C cppunit install

# Runs the unit test suite
runutests: unittest
	cd bin && ./run-unit-tests.sh

# This target runs the kernel test suite
runtest: test
	cd bin && ./mainKernelTest

# This target runs the kernel test suite through valgrind
vruntest: test
	cd bin && valgrind -v --logfile=vgrind --num-callers=8 --leak-check=yes --show-reachable=yes ./mainKernelTest

# This target should clean most everything up
clean:
	$(MAKE) -e -C app_src/SoarKernelTest clean
	$(MAKE) -e -C src clean
	$(MAKE) -e -C SoarKernelUnitTests clean
	if [ -f cppunit/Makefile ]; then $(MAKE) -C cppunit distclean; fi
	rm -f lib/libsoarkernel.a
	rm -Rf cppunit-install
	cd bin/TestResults/current && rm -f ????_*.soar ????_*.log
	rm -f bin/mainKernelTest
	rm -f bin/unitTest

.PHONY: clean runtest test libsoar unittest vruntest runutests
