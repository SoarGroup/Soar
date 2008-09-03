#!/bin/sh
# *** MAKE SURE CHANGES TO THIS FILE ARE REFLECTED IN THE .BAT FILE

SOARLIB=../../../soar-library

javac -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar Application.java
jar cvfm ${SOARLIB}/TestJavaSML.jar JarManifest .
