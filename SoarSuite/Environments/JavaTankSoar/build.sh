#!/bin/sh
SOARLIB="../../SoarLibrary/bin"

mkdir bin
rm bin/*.class

if ! javac -d bin -classpath ${SOARLIB}/swt.jar:${SOARLIB}/sml.jar:${SOARLIB}/JavaBaseEnvironment.jar -sourcepath source source/tanksoar/TankSoar.java ; then
	echo "Build failed."
	exit 1;
fi
cp -f source/* bin
mkdir bin/images
cp -f source/images/* bin/images
jar cfm ${SOARLIB}/JavaTankSoar.jar JarManifest -C bin .

# Mac stuff deleted, see JavaEaters/build.sh
