#!/bin/sh
# Builds the debugger
SOARLIB="../soar-library"

if ! javac -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar debugger/Application.java; then
  echo "Build failed."
  exit 1;
fi
jar cfm ${SOARLIB}/SoarJavaDebugger.jar JarManifest .
