#!/bin/bash

# To run the memory tests you must add -pg flag to the CXXFLAGS line in ./SoarKernelTest/Makefile and 
# maybe also in 2 other makefiles: ./src/Makefile and ./Makefile
# Additionally I added this profile target to the main makefile
#
#   # This target compiles the test for profiling
#   profile: libsoar
#	   $(MAKE) -e -C app_src/SoarKernelTest mainKernelTest -lc_p
# 	   cp app_src/SoarKernelTest/mainKernelTest bin
#	   cd bin && ./mainKernelTest
#

let LAST=$1+$2
export TESTNUM=$1

echo "$TESTNUM $LAST"

while [ $TESTNUM != $LAST ]
do
  echo $TESTNUM

  #malloc test
  cp include/mem.malloc.h include/mem.h
  make clean
  make profile
  cp bin/gmon.out "results/malloc$TESTNUM.out"
  gprof bin/mainKernelTest "results/malloc$TESTNUM.out" > "results/malloc$TESTNUM.txt"

  #pool test
  cp include/mem.pool.h include/mem.h
  make clean
  make profile
  cp bin/gmon.out "results/pool$TESTNUM.out"
  gprof bin/mainKernelTest "results/pool$TESTNUM.out" > "results/pool$TESTNUM.txt"

  let TESTNUM+=1
done





