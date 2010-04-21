#!/bin/bash
export SOAR_HOME="$(dirname "$0")"
export DYLD_LIBRARY_PATH="$SOAR_HOME/lib"
cd $SOAR_HOME
java -XstartOnFirstThread -jar "$SOAR_HOME/share/java/soar-soar2d-9.3.0.jar" soar2d/config/eaters.cnf &

