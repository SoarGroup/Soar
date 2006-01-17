#
# CRITTER SIMULATOR
#
# This demo illustrates how to create multiple agents and have
# them interact in a simulated world.  This demo is intentionally
# implemented in a simple manner to clearly illustrate the
# concepts of agent interactions with a simulator.
#
#-----------------------------------------------------------------------

# Global decls

global soar_library auto_path
global critter_world_dir
global input_root wall_char space_char map_file stop_cw

puts stdout "critter_sim: auto_path= $auto_path\n packages = [package names]\n"
flush stdout
package require Tk 4.2
wm withdraw .


#
# Procedure to create the simulator GUI.  This widget contains a
# map and a few buttons to control the simulator.
#

proc makeCritterWorld {{w .cw}} {
  global map_file

  catch {destroy $w}

  toplevel $w
  wm title $w "Critter World"
  wm iconname $w "Critter World"

  frame $w.frame -borderwidth 4 -relief ridge -bg Bisque3

  makeMap $w.frame.map
  loadMap $w.frame.map $map_file

  button $w.run  -text Run -command "set stop_cw 0; simStep 1000"
  button $w.stop -text Stop -command "set stop_cw 1"
  button $w.step -text Step -command "set stop_cw 0; simStep 1"

# Note that we need to tell some other agent (the original agent where 
# this demo was started) to kill the current interpreter.  We use the
# after command so the current agent will not appear in the evaluation
# stack when its destroyed.

  button $w.ok   -text Dismiss -command "destroyCritters; destroy $w"

  pack $w.frame.map
  pack $w.frame -side top -padx 2m -pady 2m
  pack $w.run $w.stop $w.step -side left -padx 1m -pady 2m
  pack $w.ok -side right -padx 1m -pady 2m
}

#-----------------------------------------------------------------------
#
# These procedures define the map loading operation.  When a map is
# loaded, any existing map with the same widget name is destroyed.
# A single text widget is created to contain the map and the map is
# loaded into that widget.  The map is found in the given text file
# and is copied into the widget verbatim.  The map can have any width
# and height, but each line (row) of the map should be the same length.
#

proc makeMap {w} {
  catch {destroy $w}
  text $w
}

#
# This procedure reads a map file.  The length of each line of the
# file is checked to find the max width line.  This is used to size
# the window.  The number of lines in the file is used to set the
# height of the text widget.  We save the width and height in globals
# since we'll need those values later.
#

proc loadMap {w file} {
  global mapdata

  set max_width  0
  set max_height 0

  set fileId [open $file r]

  while {[gets $fileId line] >= 0} {
    set lineLength [string length $line]
    if {$lineLength > $max_width} {
      set max_width $lineLength
    }
    incr max_height
    $w insert insert "$line\n"
  }

  close $fileId

  $w configure -width $max_width -height $max_height

# An odd quirk.  Text widgets index lines from 1 but characters in
# the line starting from 0.  Go figure.

  set mapdata(width)  [expr $max_width - 1]
  set mapdata(height) $max_height
}

#-----------------------------------------------------------------------
#
# Critter creation and destruction...
#
# createCritters: This procedure creates a number of agents and
#                 initializes them properly.  This initialization
#                 includes:
#                   - creating the agent
#                   - hiding its default top-level window
#                   - setting agent debugging output down to minimim
#                   - loading default and critter productions
#                   - loading critter GUI
#                   - running critter 2 dc's to ensure top-state S1
#                     exists (ok, that's a bit of a hack)
#
# If you want to run more agents, just add to the sim_agents list
# below.
#
# In addition to agent initialization, globals are updated for each
# agent to 
#   - initialize the character used on the map display
#   - initialize the movement request table
#   - clear the list of input wmes that have been added
#

proc createCritters {w} {
	global sim_agents agent_char default request auto_path soar_library
	global wmes

	set sim_agents [list alpha beta gamma]

	foreach agent $sim_agents {
		interp create $agent

		$agent alias sim sim
	        $agent eval [list set auto_path "$auto_path"]
                load {} Tk $agent
		$agent eval "set interp_name $agent"
		$agent eval "source critter.tcl"
		$agent eval "source critter_gui.tcl"

		$agent eval {d 2}

		set agent_char($agent) [string toupper [string index $agent 0]]
		set request($agent) center
		set wmes($agent) {}

		randomlyPlaceAgent $w $agent
		puts "created agent $agent"
	}
}

# This procedure destroys all critter agents and their windows.

proc destroyCritters {} {
  global sim_agents

  foreach agent $sim_agents {
      interp delete $agent
  }
  
}

#-----------------------------------------------------------------------
#
# simStep steps the simulator forward one step while calling all the
# agents to make a movement decision.  If the stop_cw flag is set
# (most likely this happens from a GUI action) then the stepping
# loop halts.

proc simStep {{n 1}} {
  global sim_agents stop_cw

  for {set i 0} {$i < $n} {incr i} {
    foreach agent $sim_agents {
      $agent eval {d 1}
      updateWorld $agent .cw.frame.map
    }
  if {$stop_cw == 1} {return}
  }
}

#
# updateWorld: This procedure is used to process the agent requests
#              and move them in the world.  If the request is invalid
#              it is ignored.  This may happen if the agent tries to
#              move off the map, into another agent, or into a wall.
#              The variable move_made is used to record whether the
#              move suceeded or not.

proc updateWorld {agent w} {
  set move_made [processAgentRequest $agent $w]
  updateAgentInputs $agent $w $move_made
  update
}

#
# processAgentRequest: This procedure moves the agent in the requested
#                      direction.  If the request is invalid, the 
#                      agent is not moved and a failure is recorded.

proc processAgentRequest {agent w} {
  global request pos_x pos_y mapdata agent_char space_char

# Set x and y to current position and update with desired
# move direction.

  set x $pos_x($agent)
  set y $pos_y($agent)

  if       {[string compare $request($agent) north] == 0} {
    set y [expr $pos_y($agent) - 1]
  } elseif {[string compare $request($agent) south] == 0} {
    set y [expr $pos_y($agent) + 1]
  } elseif {[string compare $request($agent) east ] == 0} {
    set x [expr $pos_x($agent) + 1]
  } elseif {[string compare $request($agent) west ] == 0} {
    set x [expr $pos_x($agent) - 1]
  }

# If the new position is off the map, then abort

    if {($x < 0) || ($x > $mapdata(width))} {
      return fail
    }

    if {($y < 1) || ($y > $mapdata(height))} {
      return fail
    }

# If the current position contains an object that can't
# be moved over, then ignore the move as well.

  set pos_char [$w get $y.$x]
  if {!(   ($pos_char == $space_char) \
        || ($pos_char == $agent_char($agent)))} {
     return fail
  }

# Update the map.  Overwrite the agent position with
# a space character and insert the agent's char at
# the new position.

  $w delete $pos_y($agent).$pos_x($agent)
  $w insert $pos_y($agent).$pos_x($agent) $space_char
  $w delete $y.$x
  $w insert $y.$x $agent_char($agent)

# Set the updated position values

  set pos_x($agent) $x
  set pos_y($agent) $y

  return succeed
}

#
# updateAgentInputs: This procedure updates the state of the
#                    agent's input link with the local area
#                    it can see in the map.
#
# The agent's input is updated by sending it commands to
# do add-wme's with the given information.  The add-wme
# command returns the timetag of the wme in the first
# field, so we scan that off and remember it.  When we
# need to make a change to the inputs, we remove the old
# wmes and add new ones.
#

proc updateAgentInputs {agent w move_made} {
  global wmes input_root request pos_x pos_y

# If the move failed, let the agent know.

  if {$move_made == "fail"} {
   scan [$agent eval "add-wme $input_root move-made no"] \
        "%d" wme_added
   lappend wmes($agent) $wme_added
   set request($agent) pass
  }

# Pass is used to indicate that the agent will not be
# moved.  Hence, no need to update the input wmes for
# the agent.

  if {$request($agent) == "pass"} {return}

# Delete any existing wmes

  if {$wmes($agent) != ""} {
    foreach wme $wmes($agent) {
      $agent eval "remove-wme $wme"
    }
    set wmes($agent) {}
  }
  
 # add new wmes and record their ids for later removal

 foreach dir {north south east west} {
   set content [getMapContentRelative $agent $w $dir]
   scan [$agent eval "add-wme $input_root $dir |$content|"] \
        "%d" wme_added
   lappend wmes($agent) $wme_added
 }

  scan [$agent eval \
        "add-wme $input_root location |$pos_x($agent).$pos_y($agent)|"] \
       "%d" wme_added
  lappend wmes($agent) $wme_added

  set request($agent) pass
}

# This procedure gets the contents of the map one step away from
# the agent in the given direction.

proc getMapContentRelative {agent w dir} {
  global mapdata pos_x pos_y wall_char

  set x $pos_x($agent)
  set y $pos_y($agent)

  if       {[string compare $dir north] == 0} {
    set y [expr $pos_y($agent) - 1]
  } elseif {[string compare $dir south] == 0} {
    set y [expr $pos_y($agent) + 1]
  } elseif {[string compare $dir east ] == 0} {
    set x [expr $pos_x($agent) + 1]
  } elseif {[string compare $dir west ] == 0} {
    set x [expr $pos_x($agent) - 1]
  }

# If the new position is off the map, then return a wall

    if {($x < 0) || ($x > $mapdata(width))} {
      return $wall_char
    }

    if {($y < 1) || ($y > $mapdata(height))} {
      return $wall_char
    }

# Otherwise, get the contents of the map at (x,y)

  return [$w get $y.$x]
}

#-----------------------------------------------------------------------
#
# Include the code to generate random numbers.  This is used to
# pick starting positions for agents.
#

source random.tcl

#-----------------------------------------------------------------------

#
# This procedure randomly selects a position on the map which 
# is open, so that the agent can be placed at the position.
#

proc randomlyPlaceAgent {w agent} {
  global mapdata agent_char space_char pos_x pos_y wall_char

  set pointFound $wall_char
 
  while {$pointFound != $space_char} {
    set x [randomRange $mapdata(width)]
    set y [expr [randomRange [expr $mapdata(height) - 1]] + 1]

    set pointFound [$w get $y.$x]
  }

# Remember the location for later use

  set pos_x($agent) $x
  set pos_y($agent) $y

# Update the map

  $w delete $y.$x
  $w insert $y.$x $agent_char($agent)
  
  return $y.$x
}

#-----------------------------------------------------------------------
#
# Sim: This procedure is called by the agents to submit their move 
#      requests.  Their requests are recorded only, not processed.
#

proc sim {agent move} {
  global request
  set request($agent) $move
}

#-----------------------------------------------------------------------

#
# Startup initializations ...
#

set input_root S1     ;# Where to put input WMEs in the agents

set wall_char  "#"    ;# Map character for a Wall
set space_char "."    ;# Map character for an empty space

set map_file "map.txt"       ;# Map file

set stop_cw 0         ;# Flag used to control stepping process

randomInit [pid]      ;# Initialize the random number generator

#-----------------------------------------------------------------------
#
# Finally, create the GUI and the critters.

makeCritterWorld .cw
createCritters .cw.frame.map
