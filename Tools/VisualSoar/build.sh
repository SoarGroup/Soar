#!/bin/sh
pushd `echo $0 | sed -n 's/^\(.*\)build.sh/\1/p'` > /dev/null

SOARLIB="../../SoarLibrary/bin"

for file in `find . -name "*.class"`
do
  rm -f $file
done

javac -source 1.4 -classpath .:${SOARLIB}/sml.jar -sourcepath Source Source/edu/umich/visualsoar/VisualSoar.java
RET=$?
if [[ $RET = 0 ]]
  then 
  if [[ `uname -s` == "Darwin" ]]
    then jar cfm ${SOARLIB}/VisualSoar.jar Source/meta-inf/manifest.mf -C Source .
    else jar cfm ${SOARLIB}/VisualSoar.jar Source/META-INF/MANIFEST.MF -C Source .
  fi
  RET=$?

  if [[ `uname -s` == "Darwin" ]]
  then
    echo "on Mac OS X, building application package for VisualSoar..."

    APP_PATH=$SOARLIB/VisualSoar.app/Contents
    mkdir -p $APP_PATH/MacOS
    mkdir -p $APP_PATH/Resources/Java

    cp VisualSoar.plist $APP_PATH/Info.plist
    cp $SOARLIB/icons/vs.icns $APP_PATH/Resources
    cp $SOARLIB/visualsoar.jar $APP_PATH/Resources/Java
    cp /System/Library/Frameworks/JavaVM.framework/Resources/MacOS/JavaApplicationStub $APP_PATH/MacOS
    chmod a+x $APP_PATH/MacOS/JavaApplicationStub
  fi
fi

popd > /dev/null

exit $RET
