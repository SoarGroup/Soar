#!/bin/sh
SOARLIB="../soar-library"

if [[ `uname -s` == "Darwin" ]]
then
  if ! javac -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath src src/edu/umich/JavaBaseEnvironment/*.java; then
    echo "Build failed."
    exit 1;
  fi
  jar cf ${SOARLIB}/JavaBaseEnvironment.jar -C src .
else
  if ! javac -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath src src/edu/umich/JavaBaseEnvironment/*.java; then
    echo "Build failed."
    exit 1;
  fi
  jar cf ${SOARLIB}/JavaBaseEnvironment.jar -C src .
fi

