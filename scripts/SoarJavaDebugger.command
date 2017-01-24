#!/bin/bash
export SOAR_HOME="$(dirname "$0")"
export DYLD_LIBRARY_PATH="$SOAR_HOME"
cd $(dirname "$0")
java -XstartOnFirstThread -Djava.library.path="$SOAR_HOME" -jar "$SOAR_HOME/SoarJavaDebugger.jar" $1 $2 $3 $4 $5 &

