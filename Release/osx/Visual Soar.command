#!/bin/bash
export SOAR_HOME="$(dirname $0)"
export DYLD_LIBRARY_PATH="$(dirname $0)/lib"
java -jar $(dirname $0)/share/java/soar-visualsoar-9.3.0.jar

