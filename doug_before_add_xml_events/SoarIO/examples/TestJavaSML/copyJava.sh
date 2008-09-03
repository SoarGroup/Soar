#!/bin/sh
# Need to run this to copy the Java files over to our project
# where we can build and work with them.
# Later, we may choose to just copy around a JAR file containing the Java files

# Create sml if it doesn't exist
if test ! -d sml
then
  mkdir sml 
fi

cp -f ../../ClientSMLSWIG/Java/*.java sml
