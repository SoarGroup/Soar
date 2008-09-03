#!/bin/sh
# *** MAKE SURE CHANGES TO THIS FILE ARE REFLECTED IN THE .BAT FILE

pushd `echo $0 | sed -n 's/^\(.*\)build.sh/\1/p'` > /dev/null

SOARLIB=../../SoarLibrary/bin

javac -source 1.4 -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath . Application.java
RET=$?
if [[ $RET = 0 ]]
  then jar cfm ${SOARLIB}/TestJavaSML.jar JarManifest .
  RET=$?
fi

popd > /dev/null

exit $RET
