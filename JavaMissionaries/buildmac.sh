#!/bin/sh
SOARLIB="../soar-library"

if ! javac -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath src src/edu/umich/mac/MissionariesAndCannibals.java; then
  echo "Build failed."
  exit 1;
fi
jar cfm ${SOARLIB}/mac.jar macJarManifest mac -C src .

if ! test -d ${SOARLIB}/mac; then
  mkdir ${SOARLIB}/mac;
fi

cp mac/mac.soar ${SOARLIB}/mac;