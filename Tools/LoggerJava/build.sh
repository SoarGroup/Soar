#!/bin/sh
# *** MAKE SURE CHANGES TO THIS FILE ARE REFLECTED IN THE .BAT FILE

SOARLIB=../../SoarLibrary/bin

javac -classpath .:${SOARLIB}/swt.jar:${SOARLIB}/sml.jar -sourcepath . log/MainFrame.java
jar cfm ${SOARLIB}/LoggerJava.jar JarManifest .
