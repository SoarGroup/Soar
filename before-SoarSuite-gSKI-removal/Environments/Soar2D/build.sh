#!/bin/sh

pushd `echo $0 | sed -n 's/^\(.*\)build.sh/\1/p'` > /dev/null

SOARBIN="../../SoarLibrary/bin"

if [ ! -e tmp ]; then
        mkdir tmp
	mkdir tmp/images
	mkdir tmp/images/book
	mkdir tmp/images/tanksoar
else
        for file in `find tmp -iname "*.class"`
        do
                rm $file
        done
        for file in `find tmp -iname "*.xml"`
        do
                rm $file
        done
	if [ -e tmp/images ]; then
		rm -rf tmp/images
	fi
	mkdir tmp/images
	mkdir tmp/images/book
	mkdir tmp/images/tanksoar
fi

javac -source 1.5 -d tmp -classpath jdom.jar:${SOARBIN}/swt.jar:${SOARBIN}/sml.jar:${SOARBIN}/tosca.jar -sourcepath src src/soar2d/Soar2D.java
RET=$?
if [[ $RET = 0 ]]
  then cp -f src/*.xml tmp
  cp -f src/images/* tmp/images
  cp -f src/images/book/* tmp/images/book
  cp -f src/images/tanksoar/* tmp/images/tanksoar
  jar cfm Soar2D.jar JarManifest -C tmp .
  RET=$?
fi

popd > /dev/null

exit $RET
