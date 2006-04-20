#!/bin/sh
SOARLIB="../../SoarLibrary/bin"

if [ ! -e bin ]; then
	mkdir bin
else
	for file in `find bin -iname *.class`
	do
		rm $file
	done
fi

if ! javac -d bin -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath source source/utilities/*.java source/simulation/*.java source/simulation/visuals/*.java; then
	echo "Build failed."
	exit 1;
fi

jar cf ${SOARLIB}/JavaBaseEnvironment.jar -C bin .
