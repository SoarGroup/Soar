=====================
TclSoarLib
Author: Mazin Assanie
5/30/17
=====================

- Seamlessly turns Soar prompt into a Tcl prompt with a single command.
- Productions can make Tcl calls by writing “(exec tcl  | <Tcl code> |)” clauses on the RHS of rules.  Soar symbols and variables can be include in RHS item.
- Provides Tcl capabilities to both local and remote clients, including the java-based debugger.
- Processes Tcl commands from both the Soar command line and any files sourced.
- Each agent has its own Tcl interpreter.
- Tested on OSX Yosemite, Windows 7, and Ubuntu 12.04.
- Very simple installation process.
  - The user must simply have ActiveTcl 8.6 installed and the TclSoarlib files
    in the same directory as the latest Soar dll.

===================
Binary Installation
===================

1.  Install ActiveTcl 8.6+ (http://www.activestate.com/activetcl/downloads)
    - We recommend that you use the default install location.

2.  Download the SoarSuite binary for your platform from the Soar wiki
    - http://soar.eecs.umich.edu/articles/downloads/soar-suite
    - TclSoarLib is automatically included in the SoarSuite download.

=====
Usage
=====

To enable:

From any soar prompt enter the command:

% soar tcl on

IMPORTANT NOTE:

If you'd like to have Tcl turned on automatically when Soar launches, add the 
above command to your settings.soar file in the main Soar directory.  If you
plan to source files that contain tcl commands, you must turn on tcl through the
settings file or in a separate command issued prior to your source command.  Due
to some technical limitation, Soar cannot currently source a file that both turns 
on Tcl and uses it immediately.

=========================
Building from Source Code
=========================

In most cases, you will never need to build TclSoarLib yourself.  It is included
in all of the binary builds and is automatically built with the Soar build
scons build script.  But if you do need to...

1.	Make sure ActiveTcl 8.6 is installed.
    - http://www.activestate.com/activetcl/downloads
    - We recommend that you use the default install location.

2.	Download the SoarSuite source code from the web site or clone the git repository
    - http://soar.eecs.umich.edu/articles/downloads/soar-suite
    - https://github.com/SoarGroup/Soar
    - Follow the standard build instructions for your platform until you get to
      the build step

3.	When you get to the compilation step, specify either the “all” or the
    “tclsoarlib” build target.

    For example, on Windows:

    % build all

    or

    % build tclsoarlib

==========
Known Bugs
==========

1.  Turning off tcl mode is currently disabled as it causes a crash on some systems.

=======
Caveats
=======

1.	You cannot turn tcl on and execute tcl code in the same file

Soar cannot deal with such files because it does not load the tcl interpreter
until the source command completes.  If you'd like to have Tcl turned on 
automatically when Soar launches, add a 'cli tcl on' command to your settings.soar 
file in the main Soar directory.  You can also turn it on at the command line
and then source your file.

2.	Only one RHS Tcl call will produce output.

Soar rhs commands “write” (and  even something like “cmd echo”) will always
work.  But only the last Tcl function called can produce output, for example a
“puts” command or a custom Tcl proc that produces output as a side effect.
That said,  all rhs Tcl calls do get executed, so they will do what they are
supposed to do, including perhaps writing output to a file.  The problem is
that the print output just doesn’t get redirected to the right place, despite
being produced.

As a workaround, a user can make sure  both that there is only one Tcl call
which needs to produce output and that it comes after any other Tcl RHS
actions.

Future versions can remedy this issue, but it may require a different
implementation for rhs output.

3.	Does not support Tk code

Tk is a widget toolkit that many Tcl programs use to provide a GUI, for
example the old Soar TSI debugger.  Future versions can remedy this issue.

4.	Tcl code that tries to do low-level Soar functions may or may not work.

Creating and deleting a kernel will certainly not work.  But other things like
creating an agent, other sml calls, other swig calls may work fine.  This
caveat is inherent to the design of Tcl as a plug-in without a main event loop.

5.	Tcl code that requires a Tcl event loop may or may not work

One example is the Tcl “after” command.  Future versions can remedy this issue.

---------------

Questions:  Contact Mazin Assanie (mazina@umich.edu)
