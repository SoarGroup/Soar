#!/bin/sh
cd SoarSuite/SoarLibrary/bin

if [ $platform = "Darwin" ]
then
	export DYLD_LIBRARY_PATH="../lib"
else
	export LD_LIBRARY_PATH="../lib"
fi
java -jar VisualSoar.jar

