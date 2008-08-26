#!/bin/sh
cd SoarSuite/SoarLibrary/bin
export DYLD_LIBRARY_PATH="../lib"
java -XstartOnFirstThread -jar SoarJavaDebugger.jar
