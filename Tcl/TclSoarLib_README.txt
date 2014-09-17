=====================
TclSoarLib
Author: Mazin Assanie
5/27/14
=====================

- Seamlessly turns Soar prompt into a Tcl prompt with a single command.
  -	Uses a combination of C++ and Tcl code.
  -	Processes commands in embedded Tcl interpreters, which calls Soar commands
    as needed.
  -	Productions can make Tcl calls by writing “(exec tcl  | <Tcl code> |)”
    clauses
  on the RHS of rules.  Soar symbols and variables can be include in RHS item.
- Very simple installation process.
  - The user must simply have ActiveTcl 8.6 installed and the TclSoarlib files
    in the same directory as the latest Soar dll.
- Provides Tcl capabilities to both local and remote clients, including the
  java-based C-Soar debugger.
-	Processes Tcl commands from both the Soar command line and any files sourced.
- Each agent has its own Tcl interpreter.
-	Tested on OSX Maverick, Windows 7, and Ubuntu 12.04.

=====
Usage
=====

To enable:

From any soar prompt enter the command:

% cli Tcl on

To disable:

% cli Tcl off

IMPORTANT NOTE:

If you'd like to have Tcl turned on automatically when Soar launches, add the 
above command to your settings.soar file in the main Soar directory.  If you
plan to source files that contain tcl commands, you must turn on tcl through the
settings file or in a separate command issued prior to your source command.  Due
to some technical limitation, Soar cannot currently source a file that both turns 
on Tcl and uses it immediately.

===================
Binary Installation
===================

1.	Install ActiveTcl 8.6+ (http://www.activestate.com/activetcl/downloads)
    - We recommend that you use the default install location.

2.	Download the SoarSuite binary for your platform from the Soar wiki
    - https://code.google.com/p/soar/wiki/Downloads
    - TclSoarLib is automatically included in the SoarSuite download.

=========================
Building from Source Code
=========================

1.	Make sure ActiveTcl 8.6 is installed.
    - http://www.activestate.com/activetcl/downloads
    - We recommend that you use the default install location.

2.	Download the SoarSuite source code from the wiki or check out the
    SVN repository
    - https://code.google.com/p/soar/wiki/Downloads
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

1.  Turning off tcl mode causes a crash on some systems.  That option is
    currently disabled as a result.

=======
Caveats
=======

1.	You cannot turn tcl on and execute tcl code in the same file

Soar cannot deal with such files because it does not load the tcl interpreter
until the source command completes.  If you'd like to have Tcl turned on 
automatically when Soar launches, add a 'cli tcl on' command to your settings.soar 
file in the main Soar directory.  You can also turn it on at the command line
and then source your file.

2.	Currently uses two aliasing mechanism

Because the module wraps known soar commands, aliases are problematic.  For now
TclSoarLib follows previous implementations and switches to its own alias
system.  When Tcl mode is turned on, aliases defined in alias.Tcl are used.
Soar aliases previously defined will no longer be recognized until Tcl mode
is turned off.  Future versions can remedy this issue.

3.	Only one RHS Tcl call will produce output.

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

4.	Does not support Tk code

Tk is a widget toolkit that many Tcl programs use to provide a GUI, for
example the old Soar TSI debugger.  Future versions can remedy this issue.

5.	Tcl code that tries to do low-level Soar functions may or may not work.

Creating and deleting a kernel will certainly not work.  But other things like
creating an agent, other sml calls, other swig calls may work fine.  This
caveat is inherent to the design of Tcl as a plug-in without a main event loop.

6.	Tcl code that requires a Tcl event loop may or may not work

One example is the Tcl “after” command.  Future versions can remedy this issue.

---------------

Questions:  Contact Mazin Assanie (mazina@umich.edu)
