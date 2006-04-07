#!/bin/sh
SOARLIB="../../SoarLibrary/bin"

mkdir bin
rm bin/*.class

if ! javac -source 1.4 -d bin -classpath ${SOARLIB}/swt.jar:${SOARLIB}/sml.jar:${SOARLIB}/JavaBaseEnvironment.jar -sourcepath source source/tanksoar/TankSoar.java ; then
	echo "Build failed."
	exit 1;
fi
cp -f source/* bin
mkdir bin/images
cp -f source/images/* bin/images
jar cfm JavaTankSoar.jar JarManifest -C bin .

# Mac stuff deleted, see JavaEaters/build.sh
