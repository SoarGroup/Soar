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
	for file in `find bin -iname *.xml`
	do
		rm $file
	done
fi

javac -source 1.4 -d bin -classpath ${SOARLIB}/swt.jar:${SOARLIB}/sml.jar:${SOARLIB}/JavaBaseEnvironment.jar -sourcepath source source/eaters/Eaters.java
RET=$?
if [[ $RET = 0 ]]
  then cp -f source/*.xml bin
  jar cfm JavaEaters.jar JarManifest -C bin .
  RET=$?

  # This next block is out of date.
  if [[ `uname -s` == "Darwin" ]]
  then
    echo "on Mac OS X, building application package for JavaEaters..."

    APP_PATH=$SOARLIB/Eaters.app/Contents
    mkdir -p $APP_PATH/MacOS
    mkdir -p $APP_PATH/Resources/Java

    cp Eaters.plist $APP_PATH/Info.plist
    cp $SOARLIB/icons/soar.icns $APP_PATH/Resources
    cp $SOARLIB/eaters.jar $APP_PATH/Resources/Java
    cp /System/Library/Frameworks/JavaVM.framework/Resources/MacOS/JavaApplicationStub $APP_PATH/MacOS
    chmod a+x $APP_PATH/MacOS/JavaApplicationStub
  fi
fi

popd > /dev/null

exit $RET
