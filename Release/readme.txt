Soar 9.3.1 Release Notes, March, 2011
=====================================

This release of Soar continues the 9.3 line which includes modules for
reinforcement learning (RL), episodic memory (EpMem), and semantic
memory (SMem), as well as everything from previous versions of Soar. All
learning mechanisms are disabled by default. This release is primarily a
bug fix release.

Soar can be downloaded from the Soar home page at:

        http://sitemaker.umich.edu/soar

Soar releases include source code, demo programs, and a number of
applications that serve as examples of how to interface Soar to an
external environment.  There is support for integrating Soar with C++,
Java, and Python applications.  Many tools that aid in development of
Soar programs are also included.

[Help and Contact information]

Please do not hesitate to contact the Soar group by sending mail to:

        For general Soar-related discussion and announcements:
                soar-group@lists.sourceforge.net
        
        For more technical developer discussion:
                soar-sml-list@lists.sourceforge.net

Please do not hesitate to file bugs on our issue tracker (search for
duplicates first):

        http://code.google.com/p/soar/issues/list       

[Important Changes for 9.3.1]

Inequality comparisons on string constants and identifiers are now
evaluated lexically rather than always returning false. Watch out--we
found that this changed the behavior on some agents that relied on the
old, broken behavior.

Improvements in stability and performance for the learning modules that
rely on databases (EpMem and SMem). Sqlite, the backing database, has
been upgraded to 3.7.4. Database schemas have been changed breaking
backwards compatibility.

The Soar kernel and SML interface now uses 64-bit integers for things
like WME values, timetags, stats, counters, and rete data structures.
Motivation for these changes is current research involving long-running
agents that were rolling over 32-bit integers in some instances. The
client interface is mostly backwards compatible but some errors or
warnings (because of loss of precision) may be encountered.

The command line interface has been updated to behave much more like it
behaved before 8.6.0. The parsing is simplified and follows most of the
rules of the Tcl language and its behavior is now much easier to
validate (and many tests have been added). This should be mostly
transparent to the end user. 

A more comprehensive list of changes is below.

Prerequisites
=============

[Platforms]

Officially supported platforms include 32- and 64-bit versions of these
systems:

  * Windows XP, Vista, 7
  * Ubuntu Linux 9.10
  * Mac OS X 10.6

Other platforms may work but have not specifically been tested, you can
try binaries or attempt to compile from source:

        http://code.google.com/p/soar/wiki/Build

Due to path length limits on some versions of Windows, you may need to
extract Soar to C:\ -- watch for errors during the extraction process.
        
[Java]

Java is required to use the Java applications distributed with Soar,
including the debugger. Java must be available in the path. Some
operating systems already have Java installed, but be aware that we only
develop and test our applications using the Sun Java runtime environment
6 and other JVMs (such as GCJ) may not work:

        http://www.oracle.com/technetwork/indexes/downloads/index.html

IMPORTANT NOTE ABOUT 64-BIT BINARIES: 32-bit Java virtual machines
cannot load 64-bit shared libraries. Please download appropriate
binaries for your installed virtual machine. Attempting to open tools
such as the Soar Java Debugger with the wrong JVM can cause the java
process to hang on Windows (use task manager to terminate it).

OS X USERS CAN CHECK/SELECT WHAT JVM THEY ARE USING WITH A UTILITY: Use
spotlight or look for the utility application "Java Preferences". More
information:

        http://developer.apple.com/java/faq/

[Python]

The included Python libraries support Python versions 2.6 and 2.7.
Your installed Python architecture (32- or 64-bit) must be the same as
the binaries you download.

DETAILED CHANGELIST
===================

Bugfixes
--------

Issue 65: chunks being built despite justifications in backtrace when
creating result on super-superstate.

Issue 70: Weird printing for strings that look like identifiers

Issue 75: Incorrect id printed in GDS removal messages

Issue 77: Lots of duplicate code in sml_KernelHelpers.cpp

Issue 79: Print command now works as documented.

GDS trace output now reports state number instead of internal level.

Connecting to a remote kernel via process number overflows a short.

Major performance fix in semantic memory when validating long-term
identifiers on rules with lots of actions.

vsnprintf_with_symbols was not null-terminating strings converted from
ids causing overruns.

Client SML identifier symbol bug fixed that was causing crashes
(r12087).

Crash caused by disabling output link change tracking.

Memory leak caused during chunking (r12151).

GDS issue where multiple references to removed states weren't all
getting cleared.

Segfault when storing a wme in epmem whose value was an identifier and
id a long-term identifier that had never been stored in epmem.

Segfault that would occur when a wme is added and removed in the same
phase

Major bugfix in interaction between epmem and smem.  (r12411)

WMA bug wherein preferences in an i-supported wme's cached o-set were
getting deallocated.

Code cleanup and maintenance fixes throughout the code.


Episodic & Semantic Memory Changes
----------------------------------

Semantic and episodic memory retrievals can now produce chunks as
opposed to only justifications.

Various experimental activation behaviors added to semantic memory.
Activation is now represented as a real number.  As a result, the
database schema was changed, breaking backwards compatibility.  A
frequency-based activation mode is introduced.

An experimental merge parameter added to semantic and episodic
memories allowing modification of long-term identifiers in working
memory.

An experimental parameter added to episodic memory that controls how
cue wmes are ordered during graph matching.


Command Line Interface Changes
------------------------------

-g/--gds flag added to the watch command for watching only GDS
messages.

Printing productions with print command now displays name of file
production was sourced from.

It is now possible to disable per-cycle stat tracking.

-d flag added to stat which makes it print only the current decision
count.

The TestCLI program has been rewritten, is much cleaner and a good
example of a lightweight debugger.

Added check to prevent crash when disconnecting Java debugger from
remote Soar when not connected.

Added new max-dc-time command to interrupt kernel execution after a
long decision cycle.


New SML Applications
--------------------

RLCLI: a simple debugger for RL experiments.

SoarQNA: facilitates agent access to external knowledge stores via the
IO system.

Liar's Dice probability calculator.


Miscellaneous Changes
---------------------

Visual Studio 2010 migration started, not officially supported yet.

New features and fixes in SMLJava library.

Experimental JMX API interface added for debugger for Soar IDE
integration.  Not officially supported.

Support for swig 2 added.

Build procedure cleanup, some stuff wasn't getting built or cleaned
correctly.  Some issues still exist but can be worked around by make
clean && make.

Jars all target Java 1.5 for better compatibility.

Output-link change tracking is now disabled by default until the
output-link identifier is requested unless the user explicitly enables
it.

GDS stats added to stats xml output.

Lots of changes to stats reporting especially with respect to timers
and time per decision cycle.  Precision increased in many places.
Configurable at runtime using timers command.

Experimental support to discard learned chunks that are duplicates of
existing RL rules modulo numeric preference value.
