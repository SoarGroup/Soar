#!/bin/sh
cd SoarSuite/Environments/Soar2D
export DYLD_LIBRARY_PATH="../../SoarLibrary/lib"
java -XstartOnFirstThread -jar Soar2D.jar eaters.xml

