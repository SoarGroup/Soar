#
# The Soar code for a critter.
#
# Input will come in on the top state (S1) and have the following form:
#
#   S1 ^location *x.y*
#
#   S1 ^north *?*
#   S1 ^south *?*
#   S1 ^east  *?*
#   S1 ^west  *?*
#
# where *x.y* indicates the map coordinates of the current position.
# The *?* will be a single character indicating what is on the map at
# that location.  For example, S1 ^east X would indicate that (relative
# to this agent), there is an object X one step to the east.  For the
# possible map values, see the simulation code (in critter_world.tcl).
#
# Given the above input, the critter decides on a move to make using the
# normal Soar mechanisms of operator proposal and application.  To help
# make this decision, the agent augments the top state with the attribute
# "last-dir" to record what the last direction of movement was.  This is
# used to avoid moving back to the last spot visited.
#
# To move in one of the four directions, the agent issues a RHS call such
# as this:
#
#   (tcl |selectDirection | <d>)
#
# This command, in turn, forwards the move request to the control
# agent to update the simulator.
#
# To reflect the current state of the inputs in the GUI, the agent
# invokes this RHS call:
#
#   (tcl |updateCritterView .$interp_name |<n>| |<s>| |<e>| |<w>)
#
# where the $interp_name will be taken from the current agent and
# embedded in the rule (the rule is defined using "" instead of {} so Tcl
# can do the variable expansion).  This RHS action expands to
# {updateCritterView .foo # . # .} (assuming the agent is called "foo" and
# the squares have the indicated values) and is run in the agent's Tcl
# shell.
#
#-------------------------------------------------------

# Make sure that we've got Soar around
#puts stdout "auto_path= $auto_path" ; flush stdout
puts stdout "critter.tcl: auto_path= $auto_path\n packages = [package names]\n"
package require Soar

# Set up some basic stuff
watch 0
output-strings-destination -push -channel stdout
source $default


sp {critter*create*space*critter
   "Formulate the initial problem space"
  (state <s> ^superstate nil)
  -->
  (<s> ^name move-around ^problem-space <p>)
  (<p> ^name critter)}

#-------------------------------------------------------

sp {critter*create*state*initial-state
   "Formulate the initial state"
  (state <s> ^name move-around ^problem-space <p>)
  (<p> ^name critter)
  -->
  (<s> ^last-dir unknown)}

#-------------------------------------------------------
#
# Form operators based on input information
#
#-------------------------------------------------------
#
# Operator Proposal

sp {critter*propose*operator*move
   "Don't propose moving over anything but empty space"
  (state <s> ^problem-space.name critter ^superstate nil)
  (<s> ^{<< north south east west >> <d>} |.|)
  (<s> ^location <l>)
  -->
  (<s> ^operator <o> + =)
  (<o> ^name move ^direction <d> ^from-location <l>)}

#-------------------------------------------------------
#
# Operator Preferences

sp {critter*compare*operator*move*north*worst
   "Prefer not to move where we have just been"
  (state <s> ^problem-space.name critter
	     ^operator <o> +
	     ^superstate nil)
  (<o> ^name move ^direction north)
  (<s> ^last-dir south)
  -->
  (<s> ^operator <o> <)}

sp {critter*compare*operator*move*south*worst
   "Prefer not to move where we have just been"
  (state <s> ^problem-space.name critter
	     ^operator <o> +
	     ^superstate nil)
  (<o> ^name move ^direction south)
  (<s> ^last-dir north)
  -->
  (<s> ^operator <o> <)}

sp {critter*compare*operator*move*east*worst
   "Prefer not to move where we have just been"
  (state <s> ^problem-space.name critter
 	     ^operator <o> +
	     ^superstate nil)
  (<o> ^name move ^direction east)
  (<s> ^last-dir west)
  -->
  (<s> ^operator <o> <)}

sp {critter*compare*operator*move*west*worst
   "Prefer not to move where we have just been"
  (state <s> ^problem-space.name critter
	     ^operator <o> +
	     ^superstate nil)
  (<o> ^name move ^direction west)
  (<s> ^last-dir east)
  -->
  (<s> ^operator <o> <)}

sp {critter*compare*operator*move*same-direction*better
   "Prefer to continue moving in the same direction"
  (state <s> ^problem-space.name critter
	    ^operator <o1> + ^operator {<o2> <> <o1>} +
	    ^superstate nil)
  (<o1> ^name move ^direction <d>)
  (<s> ^last-dir <d>)
  -->
  (<s> ^operator <o1> > <o2>)}

#-------------------------------------------------------
#
# Operator Application
#
# This tells the simulator that the agent wants to
# move in the indicated direction.  As a side effect,
# the corresponding square on the agent's GUI is
# highlighted.

sp {critter*apply*operator*move*same-direction
  (state <s> ^problem-space.name critter
	     ^operator <o>
	     ^superstate nil)
  (<o> ^name move ^direction <d>)
  (<s> ^last-dir <d>)
  -->
  (<s> ^last-dir <d>)
  (tcl |selectDirection | <d>)}

sp {critter*apply*operator*move*new-direction
  (state <s> ^problem-space.name critter
	     ^operator <o>
	     ^superstate nil)
  (<o> ^name move ^direction <d>)
  (<s> ^last-dir {<prev-d> <> <d>})
  -->
  (<s> ^last-dir <d> + <prev-d> -)
  (tcl |selectDirection | <d>)}

#-------------------------------------------------------
#
# Operator Termination

sp {critter*terminate*operator*move*location-changed
  (state <s> ^problem-space.name critter
	     ^operator <o>
	     ^superstate nil)
  (<o> ^name move ^direction <d> ^from-location <l>)
  (<s> ^location {<> <l>})
  -->
  (<s> ^operator <o> @)}

# If an operator fails, then highlight the corresponding
# square on the agent's GUI in red.

sp "critter*terminate*operator*move*action-failed
  (state <s> ^problem-space.name critter
	     ^operator <o>
	     ^superstate nil)
  (<o> ^name move ^direction <d> ^from-location <l>)
  (<s> ^location <l> ^move-made no)
  -->
  (<s> ^operator <o> @)
  (tcl |highlightCritterViewSquare .$interp_name | <d> | red|)"

#-------------------------------------------------------
#
# GUI update
#
# Update GUI when the location changes.

sp "critter*gui*update
  (state <ts> ^problem-space.name critter ^superstate nil)
  (<ts> ^location <l>)
  (<ts> ^north <n> ^south <s> ^east <e> ^west <w>)
  -->
  (tcl |updateCritterView .$interp_name |<n>| |<s>| |<e>| |<w>)"
