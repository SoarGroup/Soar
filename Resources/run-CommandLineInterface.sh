#!/bin/sh
cd SoarSuite/SoarLibrary/bin

platform=$(uname)
if [ $platform = "Darwin" ]
then
	export DYLD_LIBRARY_PATH="../lib"
else
	export LD_LIBRARY_PATH="../lib"
fi
./TestCLI

