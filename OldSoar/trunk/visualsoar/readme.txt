IMPORTANT NOTE:
===============
The current clean scripts are not complete, so
cleaning (i.e. removing object files) will need to be done
manually after the build.

You may wish to make a copy of the source before building
if you wish to keep a "clean" copy for any reason.



Building on Windows
===================

1) To build Soar, VisualSoar and STI on Windows
   edit the file windows-env.bat and set the paths
   appropriately for your system.

2) Run make-all-windows <version> where <version> == 020 for example.
   (This needs to be done from the command prompt in the Soar folder).

3) Results of the build should end up in Releases folder.
   The files Soarbuild.txt, STIBuild.txt and VSBuild.txt in that
   folder contain the output from the different stages of the build.

Alternatively, to build just the STI components, the workspace
"STI/STI.dsw" can be opened in Visual Studio 6 (or 7) and the
STI components can be built from there.  Results go into
"STI/Debug" or "STI/Release" as appropriate.


Building on Linux
=================

1) To build soar, VisualSoar and STI on Linux
   make sure all of the tools are on your path.
   (See windows-env.bat for list of tools if needed).

2) You may need to convert the soar C source files
   from Windows CRLF to Linux CR's in order for them
   to build correctly.  The Tools folder contains
   "dos-unix" and "unix-dos" utilities that will do this
   conversion (e.g. dos-unix *.c to convert all files in
   a folder to unix end of line markers).

3) Run ./make-all-linux.csh to build everything
   (a version number is not yet supported on this script as it
    is in Windows).
   The results will end up in the Releases folder as
   for Windows.

Alternatively, to build just the STI components, the
makefile "STI/Makefile" can be executed to compile
all of the STI components.
