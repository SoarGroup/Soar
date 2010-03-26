Soar 9.3.0 Release Notes, March, 2010
=====================================

This release of Soar includes modules for reinforcement learning
(introduced in 9.0), episodic memory (introduced in 9.1), and semantic
memory (introduced in 9.2), along with many new features and stability
updates. All learning mechanisms are now disabled by default.

Soar can be downloaded from the Soar home page at:

	http://sitemaker.umich.edu/soar

Soar releases include source code, demo programs, and a number of
applications that serve as examples of how to interface Soar to an external
environment.  There is support for integrating Soar with C++, Java, and
Python applications.  Many tools that aid in development of Soar programs
are also included.

[Reinforcement Learning (RL)]

RL is the architectural integration of reinforcement learning with Soar.
The RL mechanism will automatically tune the values of numeric-indifferent
preference rules based on reward generated while a Soar agent executes.
These values represent the expected discounted sum of future rewards the
agent will receive if it selects that operator for states matched by the
condition of the rule. See the RL manual and tutorial in Documentation.

To see RL in action right away, try the Water Jug RL demo (it can be loaded
from the Demos->Water Jug menu in the Java Debugger). To see the effects of
RL, run it to completion, then init-soar and run it again.  Repeat 4-5
times to see it reach optimal behavior. (Note: the agent may occasionally
perform non-optimal behavior even after it has converged because of its
exploration policy. See the RL manual and tutorial for details).

[Episodic Memory (EpMem)]

EpMem is a task-independent, architectural integration of an artificial
episodic memory with Soar. The EpMem mechanism will automatically record
episodes as a Soar agent executes. These episodes can later be queried and
retrieved in order to improve performance on future tasks. See the EpMem
manual for details.

[Semantic Memory (SMem)]

SMem is a task-independent, architectural integration of an artificial
semantic memory with Soar. The SMem mechanism facilitates deliberate
recording and querying of semantic chunks as a Soar agent executes.

[Help and Contact information]

Please do not hesitate to contact the Soar group by sending mail to:

	For general Soar-related discussion and announcements:
		soar-group@lists.sourceforge.net
	
	For more technical developer discussion:
		soar-sml-list@lists.sourceforge.net

Please do not hesitate to file bugs on our issue tracker (search for
duplicates first):

	http://code.google.com/p/soar/issues/list	

[Important Changes]

There have been performance, correctness, and stability improvements across
the board, especially with the learning mechanisms, which are all disabled
by default.

Long-term identifiers are now integrated in all symbolic memory systems.
More information on long-term identifiers can be found in section 4.2 of
the Soar-SMem manual in the documentation folder.

The allocate command has been added to Soar to allow agents to set aside
memory before starting a run so that the cost of allocation is not incurred
during the run.

The probability of selection for proposed operators is now included in the
preferences command output, helpful for debugging agents using indifferent
selection or RL, or agents that have complex operator preference semantics.

SML tracks changes on the Soar output-link in order for a number of useful
functions to perform correctly. This change tracking comes at a cost and
may now be disabled for a significant increase in performance -- even for
agents that do not use the output-link. See the new output link guide for
more information ("Examine in Detail" is the only option when disabling
change tracking):

	http://code.google.com/p/soar/wiki/SMLOutputLinkGuide

SML often opens ports when a Kernel is created. This behavior has been
extended so that it can bind to any available port, and to use local
sockets or local pipes with names based on the process id, fixing a lot of
issues people were having. Complete documentation is included in the
ClientKernel header file.

A more comprehensive list of changes is below.

Prerequisites
=============

[Platforms]

Officially supported platforms include 32- and 64-bit versions of these
systems:

  * Windows XP, Vista, 7
  * Ubuntu Linux 9.10
  * Mac OS X 10.5, 10.6

Other platforms may work but have not specifically been tested, you can try
binaries or attempt to compile from source:

	http://code.google.com/p/soar/wiki/Build

Due to path length limits on some versions of Windows, you may need to
extract Soar to C:\ -- watch for errors during the extraction process.
	
[Java]

Java is required to use the Java applications distributed with Soar,
including the debugger. Java must be available in the PATH. Some operating
systems already have Java installed, but be aware that we only develop and
test our applications using the Sun Java runtime environment 6 and other
JVMs (such as GCJ) may not work:

	http://developers.sun.com/downloads/top.jsp

IMPORTANT NOTE ABOUT 64-BIT BINARIES: 32-bit Java virtual machines cannot
load 64-bit shared libraries. Please download appropriate binaries for your
installed virtual machine. Attempting to open tools such as the Soar Java
Debugger with the wrong JVM can cause the java process to hang on Windows
(use task manager to terminate it).

OS X USERS CAN CHECK/SELECT WHAT JVM THEY ARE USING WITH A UTILITY: Use
spotlight or look for the utility application "Java Preferences". More
information:

	http://developer.apple.com/java/faq/

[Python]

The included Python libraries support Python 2.6. Your installed Python
architecture (32- or 64-bit) must be the same as the binaries you download.

Changes for 9.3.0
=================

We have moved some of our development hosting over to Google Code,
including the wiki and issue tracker. Please check it out at

	http://soar.googlecode.com

Much more documentation and example code is on the Google Code wiki.

A new "smem --init" command has been introduced for reinitialization of all
symbolic memory systems.

RL: Greatly improved template performance.

RL: Added ability to disable hierarchical discounting.

Working Memory Activation (WMA): Rewritten from scratch for performance (no
more ring) and correctness (including no more need for top-level ref
counts), many fewer parameters.

WMA: Printing working memory with the "internal" flag shows activation
value.

EpMem: Added ability to output visualization of episodes in Graphviz.

Epmem: Removal of lots of legacy code (including "tree" mode).

Epmem: Status WME (success/failure) refers to the initiating command.

SMem: Added ability to output visualization of semantic store in Graphviz.

SMem: Storage occurs at the end of every phase.

SMem: Greatly improved retrieval performance.

SMem: Status WME (success/failure) refers to the initiating command.

The build procedure has changed dramatically and is detailed on the Google
Code wiki.

Issue 31: Kernel timers for performance tuning have been updated to use
code from stlsoft.org, addressing many timer issues.

Issue 16: rand command and rand rhs functions implemented. This was
erroneously included in the previous release's change log.

Issue 60, 42: Invalid select command crash, other select command fixes.

Issue 61: Non-single compiliation units work now.

Issue 57: Many changes related to output-link change tracking including a
new method to disable output-link change tracking for a significant
performance increase if not using specific SML methods. See Wiki:
SMLOutputLinkGuide and method documentation for ClientAgent for much more
information.

Issue 62: KernelSML is no longer a singleton per process. Multiple Kernels
can now be created (and deleted) which have distinct sets of agents.

Issue 59: Added an RL parameter to turn off discounting during gaps.

Issue 53: Capture input string quoting problem fixed.

Issue 27: Listener port semantics extended to allow random listener ports
and process-specific local sockets/pipes.

Issue 40: Java Debugger layout file issues.

Issue 51: Run command extended with a new option to interrupt when a goal
retracts.

Issue 39: Memory leaks fixed.

Issue 36: Increased callback performance in SML (affects agents not using
any callbacks).

Issue 7: Memory pool preallocation command added.

Issue 18: preferences command extended to include probability that current
operators may be selected.

