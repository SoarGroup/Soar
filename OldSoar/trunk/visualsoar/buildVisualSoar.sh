#!/bin/sh
# Builds the debugger
SOARLIB="../soar-library"

for file in `find . -name "*.class"`
do
  rm -f $file
done

if ! javac -classpath .:${SOARLIB}/sml.jar -sourcepath Source Source\edu\umich\visualsoar\VisualSoar.java; then
  echo "Build failed."
  exit 1;
fi
jar cfm VisualSoar.jar Source\meta-inf\manifest.mf -C Source .
