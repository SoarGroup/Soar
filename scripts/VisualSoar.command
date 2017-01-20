#!/bin/bash
export SOAR_HOME="$(dirname "$0")"
export DYLD_LIBRARY_PATH="$SOAR_HOME"
cd $(dirname "$0")
java -Djava.library.path="$SOAR_HOME" -jar "$SOAR_HOME/VisualSoar.jar" &
