TestJavaSML
===========

This is a simple project to demonstrate and test the use of Soar through Java.

In order to run the project you need to first build everything in the SML.sln solution (under SoarIO, up 2 levels in the folder tree as I write this) and there must be Java files in ClientSMLSWIG/Java, namely, the java files that result from building ClientSMLSWIG.sln (just the Java project, not the Tcl project that's also in the solution).  These can be built directly from the C++ code or we may have decided to include them in the release.

Given those you can build this example by running buildJava.bat. The resulting TestJavaSML.jar file will be copied to the soar-library location (i.e. where the supporting libraries are located) and can be run from there.