#!/bin/bash
export SOAR_HOME="$(dirname $0)"
export DYLD_LIBRARY_PATH="$(dirname $0)/lib"
java -XstartOnFirstThread -jar $(dirname $0)/share/java/soar-missionaries-9.3.0.jar

