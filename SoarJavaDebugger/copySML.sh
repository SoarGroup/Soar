
# Need to run this to copy the Java files over to our project
# where we can build and work with them.
# Later, we may choose to just copy around a JAR file containing the Java files
export SOARIO=../SoarIO
export SOARLIB=../soar-library

cp -f $SOARIO/ClientSMLSWIG/java/build/*.java sml