#!/bin/sh
SOARLIB="../../SoarLibrary/bin"

if [ ! -e bin ]; then
        mkdir bin
	mkdir bin/images
else
        for file in `find bin -iname *.class`
        do
                rm $file
        done
        for file in `find bin -iname *.xml`
        do
                rm $file
        done
	if [ -e bin/images ]; then
		rm -rf bin/images
	fi
	mkdir bin/images
fi

if ! javac -d bin -classpath ${SOARLIB}/swt.jar:${SOARLIB}/sml.jar:${SOARLIB}/JavaBaseEnvironment.jar -sourcepath source source/tanksoar/TankSoar.java ; then
	echo "Build failed."
	exit 1;
fi

cp -f source/*.xml bin
cp -f source/images/* bin/images

jar cfm JavaTankSoar.jar JarManifest -C bin .

