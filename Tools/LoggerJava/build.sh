#!/bin/sh
# *** MAKE SURE CHANGES TO THIS FILE ARE REFLECTED IN THE .BAT FILE

pushd `echo $0 | sed -n 's/^\(.*\)build.sh/\1/p'` > /dev/null

SOARLIB=../../SoarLibrary/bin

javac -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath . log/MainFrame.java
RET=$?
if [[ $RET = 0 ]]
  then jar cfm ${SOARLIB}/LoggerJava.jar JarManifest .
  RET=$?
fi

popd > /dev/null

exit $RET
