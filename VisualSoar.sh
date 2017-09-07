#!/bin/bash

SOAR_HOME="/home/bazald/repo/thesis/Soar/out_cd"

pushd "$SOAR_HOME"
LD_LIBRARY_PATH="$SOAR_HOME" \
java \
	-Djava.library.path="$SOAR_HOME" \
	-jar "$SOAR_HOME/VisualSoar.jar" \
	$@
popd
