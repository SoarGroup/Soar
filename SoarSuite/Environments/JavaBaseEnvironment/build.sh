#!/bin/sh
SOARLIB="../../SoarLibrary/bin"

mkdir bin
rm bin/*.class
if ! javac -d bin -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath source source/utilities/*.java source/simulation/*.java source/simulation/visuals/*.java; then
	echo "Build failed."
	exit 1;
fi

jar cf ${SOARLIB}/JavaBaseEnvironment.jar -C bin .
