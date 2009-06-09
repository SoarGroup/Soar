#!/bin/sh
cd SoarSuite/SoarLibrary/bin

platform=$(uname)
if [ $platform = "Darwin" ]
then
	export DYLD_LIBRARY_PATH="../lib"
	java -XstartOnFirstThread -jar SoarJavaDebugger.jar
else
	export LD_LIBRARY_PATH="../lib"
	java -jar SoarJavaDebugger.jar
fi

