#!/bin/sh
SOARLIB="../soar-library"

if ! javac -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath src src/edu/umich/toh/TowersOfHanoi.java; then
  echo "Build failed."
  exit 1;
fi
jar cfm ${SOARLIB}/toh.jar tohJarManifest -C src .
