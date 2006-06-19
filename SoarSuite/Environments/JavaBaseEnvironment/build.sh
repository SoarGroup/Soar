#!/bin/sh

pushd `echo $0 | sed -n 's/^\(.*\)build.sh/\1/p'` > /dev/null

SOARLIB="../../SoarLibrary/bin"

if [ ! -e bin ]; then
	mkdir bin
else
	for file in `find bin -iname *.class`
	do
		rm $file
	done
fi

javac -source 1.4 -d bin -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath source source/utilities/*.java source/simulation/*.java source/simulation/visuals/*.java
RET=$?
if [[ $RET = 0 ]]
  then jar cf ${SOARLIB}/JavaBaseEnvironment.jar -C bin .
  RET=$?
fi

popd > /dev/null

exit $RET
