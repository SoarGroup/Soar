#!/bin/sh
SOARLIB="../soar-library"

if ! javac -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath src src/edu/umich/toh/TowersOfHanoi.java; then
  echo "Build failed."
  exit 1;
fi
jar cfm ${SOARLIB}/toh.jar tohJarManifest -C src .

if [ `uname -s`="Darwin" ]
then
    echo "on Mac OS X, building application package for JavaTOH..."

    APP_PATH="$SOARLIB/Towers Of Hanoi.app/Contents"
    mkdir -p "$APP_PATH/MacOS"
    mkdir -p "$APP_PATH/Resources/Java"

    cp TowersOfHanoi.plist "$APP_PATH/Info.plist"
    cp "$SOARLIB/toh.jar" "$APP_PATH/Resources/Java"
    cp "$SOARLIB/java_swt" "$APP_PATH/MacOS"
    chmod a+x "$APP_PATH/MacOS/java_swt"
fi

