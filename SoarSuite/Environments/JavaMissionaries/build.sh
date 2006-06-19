#!/bin/sh
pushd `echo $0 | sed -n 's/^\(.*\)build.sh/\1/p'` > /dev/null

SOARLIB="../../SoarLibrary/bin"

if [ ! -e bin ]; then
        mkdir bin
        mkdir bin/mac
else
        for file in `find bin -iname *.class`
        do
                rm $file
        done
        for file in `find bin -iname *.xml`
        do
                rm $file
        done
        if [ -e bin/mac ]; then
                rm -rf bin/mac
        fi
        mkdir bin/mac
fi



javac -source 1.4 -d bin -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath src src/edu/umich/mac/MissionariesAndCannibals.java
RET=$?
if [[ $RET = 0 ]]
  then cp -f src/mac/* bin/mac
  jar cfm mac.jar JarManifest -C bin .
  RET=$?
fi

popd > /dev/null

exit $RET
