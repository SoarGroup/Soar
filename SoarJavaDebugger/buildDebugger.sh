#!/bin/sh
# Builds the debugger
SOARLIB="../soar-library"

for file in `find . -name "*.class"`
do
  rm -f $file
done

if ! javac -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar debugger/Application.java; then
  echo "Build failed."
  exit 1;
fi
jar cfm ${SOARLIB}/SoarJavaDebugger.jar JarManifest .
