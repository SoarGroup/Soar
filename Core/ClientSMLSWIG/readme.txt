Generating the SWIG wrappers in Windows with Visual Studio
==========================================================
These steps were tested with Visual Studio.NET 2003.  Attempt other versions at your own risk.

Java 1.4.2
==========
0) You need the Java SDK installed. This may work with earlier versions of Java, but we haven't tested it.

1) You need to have SWIG installed. This release was tested with SWIG-1.3.29. Slightly earlier releases may work (but not 1.3.28); we can't guess about future releases. SWIG can be downloaded from www.swig.org.  Be sure to get the swigwin download.

2) Define the following user environment variables:

   JAVA_INCLUDE (location of jni.h)
   JAVA_BIN (location of javac.exe)
   SWIG (location of SWIG)

Note: If you define these variables and Visual Studio is already started, then you will need to restart it before it will recognize these variables.

3) Build all of the SML stuff (i.e. everything in SML.sln).

4) Rebuild the ClientSMLJava project (due to a bug in VS2003, a rebuild is necessary to force this to build correctly).  The sml.jar file and Java_sml_ClientInterface.dll should appear in SoarLibrary/bin.

5) Interfaces built with Java 1.4.2 appear to work with Java 1.5/5.0 fine.  However, we recommend rebuilding for Java 1.5/5.0 since you can get proper enum support in the wrappers that way.

Java 1.5/5.0
============
0) Follow steps 0-3 above.

1) In the file ClientSMLSWIG/Java/Java_sml_ClientInterface.i, uncomment the line:

%include "enums.swg"

This will enable generation of proper enums in the Java code.

Follow steps 4-5 above.

Tcl 8.4.11.1
============
0) Install Tcl.  We used ActiveTcl-8.4.11.1, but any non-threaded 8.4.x install should work (so not ActiveTcl 8.4.11.2 or later).

1) You need to have SWIG installed. This release was tested with SWIG-1.3.29. Slightly earlier releases may work (but not 1.3.28); we can't guess about future releases. SWIG can be downloaded from www.swig.org.  Be sure to get the swigwin download.

2) Define the following user environment variables:

   TCL_INCLUDE (location of tcl.h)
   TCL_LIB (location of tcl84.lib)
   TCL_BIN (location of tclsh.exe)
   SWIG (location of SWIG)

Note: If you define these variables and Visual Studio is already started, then you will need to restart it before it will recognize these variables.

3) Build all of the SML stuff (i.e. everything in SML.sln).

4) Rebuild the ClientSMLTcl project (due to a bug in VS2003, a rebuild is necessary to force this to build correctly).  The tcl_sml_clientinterface package should appear in SoarLibrary/bin.
