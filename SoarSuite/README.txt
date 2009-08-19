Windows: 

Install Visual Studio 2008

Build SML.sln and run ant, or just click build.bat

Full instructions on wiki at http://soar.googlecode.com

-----
Build notes for non-Windows platforms:

Build using SCons (http://www.scons.org/):
    scons

Note scons switches:
	scons -h

Note that there are some scons shortcuts in the Makefile, open and look at it.

------
Linux:
The following environment variables need to be set:
    JAVA_HOME="/path/to/sun-jdk-root"
    LD_LIBRARY_PATH="/full/path/to/SoarSuite/SoarLibrary/lib"

On Ubuntu, the Sun JDK root is here: /usr/lib/jvm/java-6-sun

------
Mac:
The following environment variables need to be set on OSX:
    JAVA_HOME="/System/Library/Frameworks/JavaVM.framework/Versions/CurrentJDK/Home"
    DYLD_LIBRARY_PATH=/full/path/to/SoarSuite/SoarLibrary/lib
    SCONS_LIB_DIR=/Library/scons-0.98.0

A good place to put these is ~/.bash_profile

