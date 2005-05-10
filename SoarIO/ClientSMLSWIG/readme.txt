Generating the SWIG wrappers in Windows with Visual Studio
==========================================================
These steps were tested with Visual Studio.NET 2003.  Attempt other versions at your own risk.

Java 1.4.2
==========
0) You need the Java SDK installed. This may work with earlier versions of Java, but we haven't tested it.

1) You need to have SWIG installed. This release was tested with SWIG-1.3.24. Slightly earlier releases may work; we can't guess about future releases. SWIG can be downloaded from www.swig.org.  Be sure to get the swigwin download.

2) Define the following user environment variables:

   JAVA_INCLUDE (location of jni.h)
   JAVA_BIN (location of javac.exe)
   SWIG (location of SWIG)

Note: If you define these variables and Visual Studio is already started, then you will need to restart it before it will recognize these variables.

3) Build all of the SML stuff (i.e. everything in SML.sln or SML-dll.sln).

4) Make sure the build mode (debug or release) for the SWIG project matches the build mode of the SML stuff.

5) Build the ClientSMLJava project.  The java and class files should appear in ClientSMLSWIG/Java/build, and Java_sml_ClientInterface.dll should appear in soar-library and ClientSMLSWIG/Java/build.

6) Interfaces built with Java 1.4.2 appear to work with Java 1.5/5.0 fine.  However, we recommend rebuilding for Java 1.5/5.0 since you can get proper enum support in the wrappers that way.

Java 1.5/5.0
============
0) Follow steps 0-4 above.

1) In the file ClientSMLSWIG/Java/Java_sml_ClientInterface.i, uncomment the line:

%include "enums.swg"

This will enable generation of proper enums in the Java code.

Follow steps 5-6 above.

Tcl 8.4.9
=========
0) Install Tcl.  We used ActiveTcl-8.4.9, but any 8.4.x install should work.

1) You need to have SWIG installed. This release was tested with SWIG-1.3.24. Slightly earlier releases may work; we can't guess about future releases. SWIG can be downloaded from www.swig.org.  Be sure to get the swigwin download.

2) Define the following user environment variables:

   TCL_INCLUDE (location of tcl.h)
   TCL_LIB (location of tcl84.lib)
   TCL_BIN (location of tclsh.exe)

Note: If you define these variables and Visual Studio is already started, then you will need to restart it before it will recognize these variables.

3) Build all of the SML stuff (i.e. everything in SML.sln or SML-dll.sln).

4) Make sure the build mode (debug or release) for the SWIG project matches the build mode of the SML stuff.

5) Build the ClientSMLTcl project.  The tcl_sml_clientinterface package should appear in soar-library.
