TestJavaSML
===========

This is a simple project to demonstrate and test the use of Soar through Java.

In order to run the project you need to first build everything in the SML.sln solution (under SoarIO, up 2 levels in the folder tree as I write this) and there must be Java files in ClientSMLSWIG/Java.  These can be built directly from the C++ code or we may have decided to include them in the release.

Given those you can copy the files over with "copyJava.bat" or do something similar manually which will bring the Java files into this example and the DLLs needed to run Soar from Java.

At that point you should be able to compile and run Application.java.
