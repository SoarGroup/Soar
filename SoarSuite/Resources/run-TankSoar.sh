#!/bin/sh
platform=$(uname)

cd SoarSuite/Environments/Soar2D
export DYLD_LIBRARY_PATH="../../SoarLibrary/lib"

if [ $platform = "Darwin" ]
then
	java -XstartOnFirstThread -jar Soar2D.jar tanksoar.xml
else
	java -jar Soar2D.jar tanksoar.xml
fi
