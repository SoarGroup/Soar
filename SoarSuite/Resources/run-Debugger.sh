#!/bin/sh
platform=$(uname)

cd SoarSuite/SoarLibrary/bin
export DYLD_LIBRARY_PATH="../lib"

if [ $platform = "Darwin" ]
then
	java -XstartOnFirstThread -jar SoarJavaDebugger.jar
else
	java -jar SoarJavaDebugger.jar
fi
