#!/bin/bash
export SOAR_HOME="$(pwd)"
export LD_LIBRARY_PATH="$SOAR_HOME"
java -Djava.library.path="$SOAR_HOME" -jar "$SOAR_HOME/SoarJavaDebugger.jar" $1 $2 $3 $4 $5 &
