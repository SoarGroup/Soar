#!/bin/sh
pushd `echo $0 | sed -n 's/^\(.*\)build.sh/\1/p'` > /dev/null

# Builds the debugger
SOARLIB="../../SoarLibrary/bin"

for file in `find . -name "*.class"`
do
  rm -f $file
done

javac -source 1.4 -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath . debugger/Application.java
RET=$?
if [[ $RET = 0 ]]
  then
  if [[ `uname -s` == "Darwin" ]]
    then jar cfm ${SOARLIB}/SoarJavaDebugger.jar JarManifest .
    else jar cfm ${SOARLIB}/SoarJavaDebugger.jar JarManifest .
  fi
  RET=$?
  
  if [[ `uname -s` == "Darwin" ]]
    then
    echo "on Mac OS X, building application package for SoarDebugger..."

    APP_PATH=$SOARLIB/SoarDebugger.app/Contents
    mkdir -p $APP_PATH/MacOS
    mkdir -p $APP_PATH/Resources/Java

    cp SoarDebugger.plist $APP_PATH/Info.plist
    cp $SOARLIB/icons/testapp.icns $APP_PATH/Resources
    cp $SOARLIB/SoarJavaDebugger.jar $APP_PATH/Resources/Java
    cp /System/Library/Frameworks/JavaVM.framework/Resources/MacOS/JavaApplicationStub $APP_PATH/MacOS
    chmod a+x $APP_PATH/MacOS/JavaApplicationStub
  fi
fi

popd > /dev/null

exit $RET
