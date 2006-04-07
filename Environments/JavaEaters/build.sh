#!/bin/sh
SOARLIB="../../SoarLibrary/bin"

mkdir bin
rm bin/*.class

if ! javac -source 1.4 -d bin -classpath ${SOARLIB}/swt.jar:${SOARLIB}/sml.jar:${SOARLIB}/JavaBaseEnvironment.jar -sourcepath source source/eaters/Eaters.java ; then
	echo "Build failed."
	exit 1;
fi
cp source/* bin
jar cfm JavaEaters.jar JarManifest -C bin .

# This next block is out of date.
if [[ `uname -s` == "Darwin" ]]
then
    echo "on Mac OS X, building application package for JavaEaters..."

    APP_PATH=$SOARLIB/Eaters.app/Contents
    mkdir -p $APP_PATH/MacOS
    mkdir -p $APP_PATH/Resources/Java

    cp Eaters.plist $APP_PATH/Info.plist
    cp $SOARLIB/icons/soar.icns $APP_PATH/Resources
    cp $SOARLIB/eaters.jar $APP_PATH/Resources/Java
    cp /System/Library/Frameworks/JavaVM.framework/Resources/MacOS/JavaApplicationStub $APP_PATH/MacOS
    chmod a+x $APP_PATH/MacOS/JavaApplicationStub
fi

