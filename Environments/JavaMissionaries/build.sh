#!/bin/sh
SOARLIB="../../SoarLibrary/bin"

if [ ! -e bin ]; then
        mkdir bin
        mkdir bin/mac
else
        for file in `find bin -iname *.class`
        do
                rm $file
        done
        for file in `find bin -iname *.xml`
        do
                rm $file
        done
        if [ -e bin/mac ]; then
                rm -rf bin/mac
        fi
        mkdir bin/mac
fi



if ! javac -source 1.4 -d bin -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath src src/edu/umich/mac/MissionariesAndCannibals.java; then
	echo "Build failed."
	exit 1;
fi

cp -f src/mac/* bin/mac

jar cfm mac.jar JarManifest -C bin .

