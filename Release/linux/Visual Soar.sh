#!/bin/bash
export SOAR_HOME="$(dirname "$0")"
export LD_LIBRARY_PATH="$SOAR_HOME/lib"
cd $SOAR_HOME
java -jar "$SOAR_HOME/share/java/soar-visualsoar-9.3.1.jar"

