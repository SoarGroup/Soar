Soar2Soar

Author: Nate Derbinsky, nlderbin@umich.edu
Date  : 2009


Abstract
--------
Soar2Soar facilitates fast Soar environment
prototyping by allowing a Soar agent to serve
as the environment to one or more other agents.

//////////////////       //////////////////      //////////////////
//       // in  // <---  //              // <--- // out //       //
//  env  /////////       //   Soar2Soar  //      /////////   a1  //
//       // out // --->  //              // ---> // in  //       //
//////////////////       //////////////////      //////////////////
                             ^        |
                             |        |
                             |        v
                         //////////////////
                         // out  /// in  //
                         //////////////////
                         //              //
                         //      a2      //
                         //              //
                         //////////////////
                         

The ASCII art above depicts the two-agent case.  
Soar2Soar is a C++ SML application whose initial 
task is to create SML agents (here, env, a1, a2).  
It then creates structures on the input- and output-
links of the "env" agent that are synchronized
in the following fashion: all changes to the
output-link of client agents (here, a1 and a2) 
are synchronized to the input-link of env, and 
conversely output-link changes of env are appropriately
reflected on the input-link of clients.  As described 
below, Soar2Soar scales to arbitrary number of client 
agents (a1, a2, ... an).


Quick Usage Guide
-----------------
All of these steps are discussed in more detail
below:

1. build soar2soar on your platform
2. from a terminal, run: soar2soar <n>
   (where <n> is the number of client agents
   Soar2Soar is to spawn)
3. source agent files and issue arbitrary
   Soar commands for agents, either from
   the Soar2Soar prompt or remote SML tools


Building Soar2Soar
------------------
Soar2Soar is a C++ SML application and thus 
requires access to appropriate SML headers
and to link (statically or dynamically) to 
Soar.  Basic platform-specific instructions
are below.

*nix-
Included with Soar2Soar is an SCons script.

Windows-
The easiest way to build for Windows is to replace
the source code of TestCLI.cpp in the Soar project
with that of soar2soar.cpp (this gets you all the
paths/libraries for free).


Running Soar2Soar
-----------------
From a terminal: soar2soar <n>

The parameter <n> is a required integer and
tells Soar2Soar how many client agents (in addition
to the environment) to spawn.

Once running, you can remotely connect to any of
the agents using a remote SML connection, such as
with the SoarJavaDebugger.


The Soar2Soar Console
---------------------
Once Soar is executed, you are placed at the console:

root>

At any time, issue the "quit" command to exit
Soar2Soar.

The primary purpose of the console is to provide you
access to your agents.  There are two classes of agents:
the environment agent, named "env," and client agents,
named "a1," "a2," ... "an" (where n is the command-line
parameter provided when soar2soar is executed).

To access an agent, simply type its name from the root
console:

root> env
env>

The console will change to reflect the agent name.  From here
you can type arbitrary Soar commands and they will be sent to
the agent.  You will see direct results of this command
reflected on the command-line:

root> env
env> learn
Learning is disabled.

NOTE: Soar2Soar does not register for print callbacks, so 
output of many run-time events, for instance, will not
be reflected.

To get back to the root console, simply type "root":

env> root
root>

Now you can access any other agents.


Example Soar2Soar Session
-------------------------
The following is an example session with a set of included 
blocks-world agents.  On *nix these are copied to the SoarSuite
Demos directory.

Here, two client agents will work on independent (though identical) 
blocks-world problem instances:

./soar2soar 2
root> env
env> source environments/blocks-world.soar
***************
Total: 15 productions sourced.
Source finished.

env> root
root> a1
a1> source agents/blocks-world.soar
**************
Total: 14 productions sourced.
Source finished.

a1> root
root> a2
a2> source agents/blocks-world.soar
**************
Total: 14 productions sourced.
Source finished.

a2> run

An agent halted during the run.

a2> p --depth 10 s1
(S1 ^epmem E1 ^io I1 ^name blocks-world ^operator O1202 ^operator O1201 +
       ^operator O1202 + ^operator O1199 + ^operator O1200 + ^reward-link R1
       ^smem S2 ^soar2soar ready ^superstate nil ^top-state S1 ^type state)
  (E1 ^command C1 ^result R2)
  (I1 ^input-link I2 ^output-link I3)
    (I2 ^block table ^block a ^block b ^block c ^block d ^completion-time 4
           ^desired D3 ^desired D1 ^desired D2 ^desired D4 ^goal achieved
           ^on-top O6 ^on-top O7 ^on-top O8 ^on-top O9)
      (D3 ^block1 b ^block2 a)
      (D1 ^block1 d ^block2 table)
      (D2 ^block1 c ^block2 table)
      (D4 ^block1 a ^block2 d)
      (O6 ^block1 d ^block2 table)
      (O7 ^block1 c ^block2 table)
      (O8 ^block1 b ^block2 a)
      (O9 ^block1 a ^block2 d)
  (O1202 ^name success)
  (O1201 ^block1 b ^block2 c ^name move-block)
  (O1199 ^block1 c ^block2 b ^name move-block)
  (O1200 ^block1 b ^block2 table ^name move-block)
  (S2 ^command C2 ^result R3)


a2> root
root> a1
a1> p --depth 10 s1
(S1 ^epmem E1 ^io I1 ^name blocks-world ^operator O604 ^operator O603 +
       ^operator O604 + ^operator O601 + ^operator O602 + ^reward-link R1
       ^smem S2 ^soar2soar ready ^superstate nil ^top-state S1 ^type state)
  (E1 ^command C1 ^result R2)
  (I1 ^input-link I2 ^output-link I3)
    (I2 ^block table ^block a ^block b ^block c ^block d ^completion-time 4
           ^desired D3 ^desired D1 ^desired D2 ^desired D4 ^goal achieved
           ^on-top O6 ^on-top O7 ^on-top O8 ^on-top O9)
      (D3 ^block1 b ^block2 a)
      (D1 ^block1 d ^block2 table)
      (D2 ^block1 c ^block2 table)
      (D4 ^block1 a ^block2 d)
      (O6 ^block1 d ^block2 table)
      (O7 ^block1 c ^block2 table)
      (O8 ^block1 b ^block2 a)
      (O9 ^block1 a ^block2 d)
  (O604 ^name success)
  (O603 ^block1 b ^block2 c ^name move-block)
  (O601 ^block1 c ^block2 b ^name move-block)
  (O602 ^block1 b ^block2 table ^name move-block)
  (S2 ^command C2 ^result R3)


a1> root
root> env
env> p --depth 10 s1
(S1 ^epmem E1 ^io I1 ^name blocks-world
       ^reward-link R1 ^smem S2 ^soar2soar ready ^superstate nil ^top-state S1
       ^type state)
  (E1 ^command C1 ^result R2)
  (I1 ^input-link I2 ^output-link I3)
    (I2 ^agents A1 ^console C5)
      (A1 ^agent A5 ^agent A3)
        (A5 ^commands C7 ^id 2 ^name a2)
        (A3 ^commands C6 ^id 1 ^name a1)
      (C5 ^time 4)
    (I3 ^agents A2)
      (A2 ^agent A6 ^agent A4)
        (A6 ^feedback F2 ^id 2 ^input I5 ^name a2)
          (I5 ^block table ^block a ^block b ^block c ^block d
                 ^completion-time 4 ^desired D2 ^desired D4 ^desired D3
                 ^desired D1 ^goal achieved ^on-top O9 ^on-top O8 ^on-top O7
                 ^on-top O6)
            (D2 ^block1 b ^block2 a)
            (D4 ^block1 d ^block2 table)
            (D3 ^block1 c ^block2 table)
            (D1 ^block1 a ^block2 d)
            (O9 ^block1 d ^block2 table)
            (O8 ^block1 c ^block2 table)
            (O7 ^block1 b ^block2 a)
            (O6 ^block1 a ^block2 d)
        (A4 ^feedback F1 ^id 1 ^input I4 ^name a1)
          (I4 ^block table ^block a ^block b ^block c ^block d
                 ^completion-time 4 ^desired D6 ^desired D8 ^desired D7
                 ^desired D5 ^goal achieved ^on-top O13 ^on-top O12
                 ^on-top O11 ^on-top O10)
            (D6 ^block1 b ^block2 a)
            (D8 ^block1 d ^block2 table)
            (D7 ^block1 c ^block2 table)
            (D5 ^block1 a ^block2 d)
            (O13 ^block1 d ^block2 table)
            (O12 ^block1 c ^block2 table)
            (O11 ^block1 b ^block2 a)
            (O10 ^block1 a ^block2 d)
  (S2 ^command C2 ^result R3)


env> quit


Agent Initialization
--------------------
As a part of spawning agents, Soar2Soar provides each agent some
productions it needs for initialization purposes.  These rules
are prefaced with "soar2soar*" - they must NOT be modified or
excised if Soar2Soar is to function correctly (this goes for
the environment and all client agents).

The only side-effect of Soar2Soar initialization productions is an
o-supported WME on top-state:

S1 ^soar2soar ready

So your agents (environment and client) should condition upon this
WME for first production matches.  Otherwise, Soar2Soar is transparent
during agent execution.


Environment Agent
-----------------
Described here are the structures available to the environment agent.

Input-
The input-link of the environment agent contains the output of all
client agents, as well as features provided from Soar2Soar.  The
input-link is structured as follows:

input-link
  agents
    agent
      id
      name
      commands
  console
    time

For each agent, there will be a multi-valued attribute ("agent") rooted
at the "agents" identifier.  Each agent has an "id" (unique integer, 1..n),
a name ("a1" .. "an"), and a "commands" identifier (synchronized with this
client agent's output-link).

The "console" identifier is intended to allow Soar2Soar to communicate arbitrary
structures to the environment.  Currently the only usage is the "time" WME which
is an integer which will be updated each cycle to represent the number of seconds
since the execution of Soar2Soar.

Output-
The output-link of the environment agent is primarily used to construct the 
input-links of client agents, as well as to provide feedback to agent outputs:

output-link
  agents
    agent
      id
      name
      input
      feedback

Each client agent has an agent structure on the output-link analogous to the one
on the input-link.  The "id" and "name" WMEs are used to associate client agent
commands (on environmental input) to resulting inputs (on environmental output).

The "input" identifier is where the environment provides arbitrary structures that
are directly synchronized to the respective client agent's input-link.

The "feedback" identifier provides the environment the ability to augment the client
agent's output-link, primarily to support "command ^status complete" capability.
Soar2Soar only supports a single form of feedback: add-wme.  The form of an add-wme
command is as follows:

feedback
  add-wme
    id
    attr
    value

The "id" WME must be an identifier currently on the respective agent's output-link (as
reflected by the "command" identifier on the environment agent's input-link).  The "attr"
and "value" WMEs can be arbitrary symbolic constants.  This command can be thought of as
a call to the client agent's "add-wme" command (with an appropriately mapped identifier).
Once processed, Soar2Soar will provide a "status" augmentation to the add-wme identifier
with either "complete" or "failure" value, depending upon success of the operation.  For
example, from the blocks-world environment agent above:

sp {apply*move-block*status
   (state <s> ^operator <op>
              ^io.output-link.agents.agent <agent>)
   (<op> ^name move-block
         ^move-block <mb>
         ^agent.id <id>)
   (<agent> ^id <id>
            ^feedback <f>)
         
-->
   (<f> ^add-wme <wme>)
   (<wme> ^id <mb>
          ^attr status
          ^value processed)
}

This rule responds to a move-block command from a client agent by adding "^status processed"
augmentation.  The environment also has clean-up rules for the add-wme feedback:

sp {blocks-world*propose*clean
   (state <s> ^name blocks-world
              ^io.output-link.agents.agent.feedback.add-wme.status)
-->
   (<s> ^operator <op> + >)
   (<op> ^name clean)
}

sp {apply*clean
   (state <s> ^operator <op>
              ^io.output-link.agents.agent.feedback <f>)
   (<op> ^name clean)
   (<f> ^add-wme <wme>)
   (<wme> ^status)
-->
   (<f> ^add-wme <wme> -)
}

NOTE: Soar2Soar will halt the environment agent automatically via SML once
all client agents have halted.  Halting the environment should NOT be done
in rules (via the "halt" RHS function).


Client Agent
------------
Soar2Soar agents, aside from initialization (described above) have no special structures and
simply respond to standard input-link changes and act in the world via output-link changes.

NOTE: Soar2Soar will halt the environment agent automatically via SML once all 
client agents have halted.

