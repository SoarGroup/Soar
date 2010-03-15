#!/bin/sh
platform=$(uname)
if [ $platform = "Darwin" ]
then
	export DYLD_LIBRARY_PATH="$(dirname $0)/SoarSuite/SoarLibrary/lib"
	java -XstartOnFirstThread -jar $(dirname $0)/SoarSuite/SoarLibrary/bin/SoarJavaDebugger.jar
else
	export LD_LIBRARY_PATH="$(dirname $0)/SoarSuite/SoarLibrary/lib"
	java -jar $(dirname $0)/SoarSuite/SoarLibrary/bin/SoarJavaDebugger.jar
fi

