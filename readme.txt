=============================
SoarSuite readme.txt Overview
=============================
(1) Windows Build Instructions
(2) Linux Build Instructions
(3) Mac OSX Build Instructions
(4) Building Individual Subcomponents
(5) SCons Script Documentation
(6) How Library Search Paths Work

==========================
Windows Build Instructions
==========================

32-bit and 64-bit Windows platforms are supported for Windows XP, Vista, and 7.
Other versions of 32-bit windows might work but are not officially supported.
--------------------------
Install the Prerequisities
--------------------------
 - Microsoft Visual Studio 2008
   - You must have either Microsoft Visual Studio 2008 or the free Microsoft Visual C++ 2008 Express Edition (http://www.microsoft.com/visualstudio/en-us/products/2008-editions/express).  (Note that the free Express Edition does not include a 64-bit compiler.)
   - Warning: If you are on a 64-bit machine, make sure to explicitly choose to install the the 64-bit compiler!  It is not installed by default!
 - Java SE 6 JDK
   - The latest Java SE 6 JDK is available from http://java.sun.com/javase/downloads/index.jsp.  Make sure to download the 64-bit version, if you are on a 64-bit machine.
 - SWIG
   - SWIG acts as a bridge between the Soar C++ libraries and the various other languages, like Java, Tcl, Python, Perl. Since nearly everyone will need the Soar debugger, which is written in Java, this component is required.
   - You can download it from http://www.swig.org/download.html and extract it anywhere, such as your home folder or `C:\` or `C:\Program Files`
 - Python
   - Soar uses the SCons build system, which requires Python. Python version 2.7.2 is recommended, but anything from 2.5 onward should work. The Python 3.x series WILL NOT WORK.  You can download Python 2.7.2 from http://www.python.org/download/releases/2.7.2/
     - If you are planning on a debug build including Python, you will need Python debug libraries (http://www.eecs.umich.edu/~soar/sitemaker/misc/python). Save the correct one for your architecture in `Python26\libs`.

----------
Build Soar
----------
Start cmd.exe, cd into the SoarSuite directory, and type

  build all

The batch file will ask you for the paths to SWIG and Python before building your script.  You no longer have to specify those paths in environment variables.

For more detailed information about the Scons script, see the SCons script documentation section below. See the FAQ for common solutions to errors.

---------------------------------
Visual Studio Solution Generation
---------------------------------
SCons can generate Visual Studio project and solution files that allow users to more easily modify and debug the kernel source code.  When performing 'build all', these files will also be generated.  You can also manually generate these files by typing

  scons msvs

Note that the generated projects are not stand-alone, they still call SCons under the hood to build targets.

========================
Linux Build Instructions
========================

The only officially supported Linux distribution right now is Ubuntu (http://www.ubuntu.com/), though Soar should work on almost any distribution so long as things are configured correctly.

-------------------------
Install the Prerequisites
-------------------------
 - Install the following required packages:
   - openjdk-6-jdk (you can also use Sun's Java JDK)
   - swig
   - build-essential
   - python-all-dev
   - To get the above items, you can use your favorite package manager, such as `apt-get`.  For example:

sudo apt-get install openjdk-6-jdk swig build-essential python-all-dev

 - Python: Soar uses the SCons build system (scons.org), which requires Python. This should already be installed on Linux.  Just make sure you're running an appropriate version: 2.7.2 is recommended, but anything from 2.5 onward should work. The Python 3.x series WILL NOT WORK.  To check your version, type python --version

----------
Build Soar
----------
Go into the SoarSuite folder and run: 

python scons/scons.py all

All of your resulting binaries will be in the /out folder.  

For more detailed information about the Scons script, see the SCons script documentation section below. See the FAQ for common solutions to errors.

---------------------------------
Set Up Your Environment Variables
---------------------------------
While you don't need any environment variables to build Soar, you may need a few set when running or developing for Soar.  To make these changes permanent, edit the `.bashrc` file in your home diretory and put them at the bottom.

Soar binaries are, by default, installed to a prefix `SoarSuite/out`.  Java-based Soar application may need to be able to load relevant dynamic libraries on the fly, so you need to make sure that it can find the libraries it needs. Java will look `LD_LIBRARY_PATH`, so you can use that to tell it where to look.

# The exact path here will depend on your system settings.
export LD_LIBRARY_PATH=/home/$USER/soar/SoarSuite/out/

Certain functions in the kernel may also use the SOAR_HOME environment variable to find binaries.  For example, SpawnDebugger uses it to find the SoarDebugger.jar.

# The exact path here will depend on your system settings.
export SOAR_HOME=/home/$USER/soar/SoarSuite/out/

For more detailed information about library search paths, see our wiki page on [BuildLibrarySearchPaths how library search paths work].

===========================
Mac OS X Build Instructions
===========================

-------------------------
Install the Prerequisites
-------------------------
 - Java Developers Package:  10.6 is the only one we support. You can find this at http://connect.apple.com/ under Java downloads.  You do not have to pay for a developers account.  There's a link at the bottom that will let you register for free resources.
   - Note: If you get an error with "jni.h", visit the http://discussions.apple.com/thread.jspa?threadID=2630649&tstart=0 to see how to resolve it.
 - XCode: XCode is the Mac OS development environment under which you'll build Soar.  It is free with the latest version of MacOS.  If you don't already have it, you can download it http://developer.apple.com/tools/xcode/.
 - Python: Soar uses the SCons build system (scons.org), which requires Python. This should already be installed on OSX.  Just make sure you're running an appropriate version.  2.7.2 is recommended, but anything from 2.5 onward should work. The Python 3.x series WILL NOT WORK.  To check your version, type python --version
  
----------
Build Soar
----------
Go into the SoarSuite folder and run: 

scons all

All of your resulting binaries will be in the /out folder.  

For more detailed information about the Scons script, see the SCons script documentation section below. See the FAQ for common solutions to errors.

----------------------------
Set Up Environment Variables
----------------------------
While you don't need any environment variables to build Soar, you may need a few set when running or developing for Soar.  To make these changes permanent, edit the `.bashrc` file in your home diretory and put them at the bottom.

Soar binaries are, by default, installed to a prefix `SoarSuite/out`.  Java-based Soar application may need to be able to load relevant dynamic libraries on the fly, so you need to make sure that it can find the libraries it needs. Java will look `DYLD_LIBRARY_PATH`, so you can use that to tell it where to look.

# The exact path here will depend on your system settings.
export DYLD_LIBRARY_PATH=/home/$USER/soar/SoarSuite/out/

Certain functions in the kernel may also use the SOAR_HOME environment variable to find binaries.  For example, SpawnDebugger uses it to find the SoarDebugger.jar.

# The exact path here will depend on your system settings.
export SOAR_HOME=/home/$USER/soar/SoarSuite/out/

For more detailed information about library search paths, see our wiki page on [BuildLibrarySearchPaths how library search paths work].

==============================
Building Individual Subcomponents
==============================

The Soar suite has several individually buildable targets:

- The Soar kernel library. This includes core Soar functionality
  as well as the Soar Markup Language (SML) interface. SML is the standard
  interface for hooking Soar up to external or virtual environments. The
  Java debugger and command line tools also rely on SML.

    Windows: Soar.dll, Soar.lib
    Linux:   libSoar.so
    OSX:     libSoar.dylib
  
- SWIG wrapper libraries. These contain language-specific wrappers
  automatically generated by SWIG to allow languages other than C++
  to interface with SML. The primary languages we use this for are Java
  and Python, although interfaces also exist for Tcl and C#, but have
  been untested for a while.
  
    Common:  Python_sml_ClientInterface.py (Python), sml.jar (Java)
    Windows: _Python_sml_ClientInterface.pyd, Java_sml_ClientInterface.dll
    Linux:   _Python_sml_ClientInterface.so, libJava_sml_ClientInterface.so
    OSX:     _Python_sml_ClientInterface.dylib, libJava_sml_ClientInterface.dylib
	
- Java debugger. This is the primary way for users to run Soar and debug agents.

    Common:  SoarJavaDebugger.jar, soar-debugger.jar
  
- Command line interfaces. The primary one is TestCLI. There's also the
  more minimalistic cli which is more suitable for using in scripts.

    Linux, OSX: TestCLI, cli
    Windows:    Same, with .exe suffixes

- Tests. This includes a program that runs many unit tests and other
  programs that run performance tests. The unit test program also requires
  a number of agent files (*.soar) that will be copied into your install
  directory.
  
    Common:     Many *.soar files under the TestAgents subdirectory
    Linux, OSX: UnitTests, TestSoarPerformance, TestSMLPerformance, TestSMLEvents
    Windows:    Same, with .exe suffixes
	
- SML headers. C++ header files needed by programs that use
  SML. "Building" these just involves copying the headers from various
  points in the source tree into a single directory. Note that Java and
  Python SML programs DO NOT need these headers, as the SWIG wrapper
  classes incorporate the definitions contained in them.
	
A user who only wants to run Soar but not create new environments should
build the kernel library, Java SWIG wrapper, and the Java debugger. Users
who want to write new environments with SML will also need, depending
on their language choice, the SML C++ headers, Java SWIG wrapper, or
Python SWIG wrapper. The test and command line interface programs are
mainly for internal development.

==========================
SCons Script Documentation
==========================

Soar uses single SCons build process for Windows, Linux, and Mac
OSX. SCons tries to be (too) smart and uses the appropriate compilers,
linkers, and other commands for your OS. A version of SCons is distributed
in the Soar source tree, so you don't need to install it separately. To
run the builder, start a shell (cmd.exe in Windows), cd into the SoarSuite
directory and type

  python scons/scons.py <options>
  
or if you want to use the SCons version you've installed separately,

  scons <options>

"<options>" is a place holder for options we'll pass into the builder,
and should not be typed literally.

For Windows users, we have provided a batch file called build.bat. This
file will search for the locations of the python, java, and swig
installs, and set the PATH environment variable so that the various
compilers can run. In addition, SCons automatically finds the installed
Visual C++ compiler, so there's no need to use the Visual Studio Command
Prompt. Windows users can simply type:

  build <options>

For the rest of this document we'll be showing example build commands
using the "scons <options>" form. Please substitute the appropriate
command for "scons".

-------------
Build Targets
-------------
The builder can be directed to compile any number of targets,
corresponding to the list given above. To see a list of available
targets, type

  scons list

You should see something like this:

  all
  cli
  debugger
  debugger_api
  headers
  java_sml_misc
  kernel
  sml_java
  sml_python
  tests

The "all" target tells scons to build everything. Any number of targets
can be passed to the builder together. For example:

  scons kernel debugger sml_python headers

If no targets are specified, the default targets that will be built are
kernel, sml_java, sml_python, debugger.

------------------
Output Directories
------------------
By default, the built libraries and executables will be placed into the
"out" subdirectory under SoarSuite, and intermediates such as .o files
will be placed in the "build" subdirectory. You can change these using
the --outdir=<dir> and --build-dir=<dir> options. These directories can
be located or moved anywhere in your file system. However, we recommend
you not change the file structure inside the "out" directory, as the
executables expect the libraries to be in certain places relative to them.

To delete all the files you just built, type

  scons -c all

-----------
Build Flags
-----------

You can modify building behavior by passing in a number of flags in the
form '--<flagname>'. To see the list of available flags, type

  scons --help

and look under the "Local Options" section (the other flags are for
scons itself). One particularly useful flag is "--debug", which turns
off compiler optimizations and inserts debugging symbols for either gdb
(-g) or Visual Studio (/Z7 /DEBUG). Another flag is "--verbose", which
makes scons print out the full commands it uses to build each file. This
is useful if the build doesn't succeed and you want to figure out what
is happening under the hood.

=============================
How Library Search Paths Work
=============================

This section is not about building Soar, but running the executables that
you've built. Because the Soar libraries are not installed to standard
locations such as /usr/lib on Linux or C:\Windows\System32 on Windows,
the operating system doesn't know where to look for them. This is probably
the biggest and most frequently encountered problem when getting Soar up
and running.

The most straightforward way to solve this problem is to tell the OS where
to look. In Linux, this means setting the LD_LIBRARY_PATH environment
variable to include the directory libSoar.so is in. Similarly on Mac OSX,
you should set DYLD_LIBRARY_PATH. On both operating systems, the command
for setting the environment variable should look like:

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/directory/of/your/Soar/library

or

export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/directory/of/your/Soar/library

Note that you can't put spaces around the =, and you have to use export
so that the variable will be inherited by child processes.

In Windows, you should set PATH to include the directory containing Soar.dll.

The rest of this section contains explanations of what's going on under
the hood of every operating system so that you'll have some insight
should you run into issues.

-----
Linux
-----
In Linux, the GNU linker provides an -rpath flag that hard codes
library search paths into executables. We use this flag for all native
executables, such as TestCLI and TestSoarPerformance. These paths are
set assuming that the executables and required libraries (specifically
libSoar.so) reside in the same directory, which is the default build
behavior. Therefore, as long as you don't move the executables or
libraries to different relative directories, you should be able to just
run the executables without doing anything special. If you are having
problems, you should manually set LD_LIBRARY_PATH.

--------
Mac OS X
--------
Dynamic libraries on OS X have the concept of install_name. Basically,
shared libraries have a full path hard-coded into them when they are
built, and when executables link against the library, they record those
paths as the places they'll look for the library at runtime. For more
details, see

https://developer.apple.com/library/mac/documentation/developertools/conceptual/MachOTopics/1-Articles/loading_code.html

We set the install_name of libSoar.dylib to be
"@executable_path/libSoar.dylib", which means that every executable
will look for the Soar library in the same directory as itself. For
the executables that are built with Soar, such as TestCLI and
TestSoarPerformance, you should be able to just run them without
any problems. If you are having problems, you should manually set
DYLD_LIBRARY_PATH.

The more likely case when this will appear is when you build your own
SML programs and link them against libSoar.dylib. Your programs will
also think that the library exists in the same directory as themselves. In
this case please use the above solution. As a more permanent fix,
you can change the library's install_name to the absolute path of the
library using the install_name_tool program. Every program you build from
then on will look for the library using the absolute path, which should
not fail unless you move the library or rebuild it. Please consult the
system man page for that command for further information.

-------
Windows
-------
Windows searches the executable's directory for needed libraries by
default, and since all Soar libraries and executables are in the same
directory, there shouldn't be any library not found errors. If you are
building your own executable environments, either put them in the same
directory as Soar.dll, or add the directory that contains Soar.dll to
your PATH environment variable. For more information about library search
order, see

http://msdn.microsoft.com/en-us/library/windows/desktop/ms682586%28v=vs.85%29.aspx

----
Java
----
The Java virtual machine has two sets of search paths: the class path and
the library path. In general, java classes and jars are searched for
in the class path, and native libraries that are loaded by JNI are searched
for in the library path. Running Java SML clients requires you to set
both correctly.

The SWIG generated SML wrapper classes are contained in sml.jar,
so you need to include its path (not the path to the directory
containing the jar) in the class path. Those classes in turn load the
Java_sml_ClientInterface and Soar native libraries, so the directory
containing these libraries must be in the library path.

You can set the class path either by setting the CLASSPATH environment
variable (remember to export in Linux and OSX), or passing a command
line switch to the java executable:

java -cp /path/to/sml.jar

By default, the JVM will include the LD_LIBRARY_PATH and DYLD_LIBRARY_PATH
environment variables in the library path in Linux and OS X,
respectively. In Windows the current working directory and the PATH
environment variable is included by default. You can manually set the
library path by passing a command line switch to the java executable:

java -Djava.library.path=/directory/of/your/Soar/library

Note that while you can also set java.library.path at runtime using Java
code, you have to use some hacks to make the virtual machine actually
acknowledge the changes. See this page for more information:

http://stackoverflow.com/questions/5419039/is-djava-library-path-equivalent-to-system-setpropertyjava-library-path

------
Python
------
Like with Java, SWIG also generates a Python wrapper, called
Python_sml_ClientInterface.py as well as a native library called
_Python_sml_ClientInterface.so (.pyd in Windows) for the Python
interface. Unlike Java, Python only has one list of directories where it
looks for both files, contained in sys.path. On all operating systems,
this list will include the directories in the environment variable
PYTHONPATH. You can also set sys.path directly in your python script. For
example:

import sys
sys.path.append('/directory/of/your/Soar/library')
import Python_sml_ClientInterface as sml
