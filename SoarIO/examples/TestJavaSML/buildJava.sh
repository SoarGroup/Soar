#!/bin/sh

# *** MAKE SURE CHANGES TO THIS FILE ARE REFLECTED IN THE .BAT FILE

# Need to run this to copy the Java files over to our project
# where we can build and work with them.
# Later, we may choose to just copy around a JAR file containing the Java files
SOARLIB=../../../soar-library
SOARIO=../..

mkdir sml
cp -f $SOARIO/ClientSMLSWIG/Java/build/*.java sml

javac Application.java

jar cvfm TestJavaSML.jar JarManifest *.class sml/*.class

cp -f *.jar $SOARLIB

