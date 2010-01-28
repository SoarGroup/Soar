#!/bin/bash

SOAR_PATH=$DYLD_LIBRARY_PATH/../..

java -classpath $DYLD_LIBRARY_PATH/sml.jar:. Poc $SOAR_PATH $1
