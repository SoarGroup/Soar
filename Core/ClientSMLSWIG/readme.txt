Generating the SWIG wrappers in Windows with Visual Studio
==========================================================
These steps were tested with Visual Studio.NET 2005.  Attempt other versions at your own risk.

Java 1.5/5.0 and 1.6/6.0
========================
0) You need the Java JDK installed.

1) You need to have SWIG installed. This release was tested with SWIG-1.3.31, but later versions will probably work.  SWIG can be downloaded from www.swig.org.  Be sure to get the swigwin download.

2) Define the following user environment variables:

   JAVA_INCLUDE (location of jni.h)
   JAVA_BIN (location of javac.exe)
   SWIG (location of SWIG)

Note: If you define these variables and Visual Studio is already started, then you will need to restart it before it will recognize these variables.

3) Build all of the SML stuff (i.e. everything in SML.sln).

4) Rebuild the ClientSMLJava project (due to a bug in VS2005, a rebuild is necessary to force this to build correctly).  The sml.jar file and Java_sml_ClientInterface.dll should appear in SoarLibrary/bin.

5) Interfaces built with Java 1.5 appear to work with Java 1.6 fine. The reverse is not true.

Tcl 8.4.11.1
============
0) Install Tcl.  We used ActiveTcl-8.4.11.1, but any non-threaded 8.4.x install should work (so not ActiveTcl 8.4.11.2 or later).

1) You need to have SWIG installed. This release was tested with SWIG-1.3.31, but later versions will probably work. SWIG can be downloaded from www.swig.org.  Be sure to get the swigwin download.

2) Define the following user environment variables:

   TCL_INCLUDE (location of tcl.h)
   TCL_LIB (location of tcl84.lib)
   TCL_BIN (location of tclsh.exe)
   SWIG (location of SWIG)

Note: If you define these variables and Visual Studio is already started, then you will need to restart it before it will recognize these variables.

3) Build all of the SML stuff (i.e. everything in SML.sln).

4) Rebuild the ClientSMLTcl project (due to a bug in VS2005, a rebuild is necessary to force this to build correctly).  The tcl_sml_clientinterface package should appear in SoarLibrary/bin.

Python 2.4.3
============
0) Install Python.  We used Python 2.4.3, but other 2.4.x versions may work.  Python 2.5 requires special steps (see below)

1) You need to have SWIG installed. This release was tested with SWIG-1.3.31, but later versions will probably work..  SWIG can be downloaded from www.swig.org.  Be sure to get the swigwin download.

2) Define the following user environment variables:

   PY_INCLUDE (location of Python.h)
   PY_LIB (location of python24.lib)
   SWIG (location of SWIG)

Note: If you define these variables and Visual Studio is already started, then you will need to restart it before it will recognize these variables.

3) Build all of the SML stuff (i.e. everything in SML.sln).

4) Rebuild the ClientSMLPython project (due to a bug in VS2005, a rebuild is necessary to force this to build correctly).  The Python_sml_clientinterface.py and _Python_sml_ClientInterface.pyd files should appear in SoarLibrary/bin.

NOTE: This only works for a release build.  To do a debug build requires building a debug version of python, which is beyond the scope of this document.

Python 2.5
==========
0) Install Python.  We used Python 2.5.0, but other 2.5.x versions may work.

1) You need to have SWIG 1.3.31 installed (later versions will probably work).

2) Define the following user environment variables:

   PY_INCLUDE (location of Python.h)
   PY_LIB (location of python25.lib)
   SWIG (location of SWIG)

Note: If you define these variables and Visual Studio is already started, then you will need to restart it before it will recognize these variables.

3) Build all of the SML stuff (i.e. everything in SML.sln).

4) In the ClientSMLPython project linker settings, change the library name from python24.lib to python25.lib.

5) Rebuild the ClientSMLPython project (due to a bug in VS2005, a rebuild is necessary to force this to build correctly).  The Python_sml_clientinterface.py and _Python_sml_ClientInterface.pyd files should appear in SoarLibrary/bin.

NOTE: This only works for a release build.  To do a debug build requires building a debug version of python, which is beyond the scope of this document.
