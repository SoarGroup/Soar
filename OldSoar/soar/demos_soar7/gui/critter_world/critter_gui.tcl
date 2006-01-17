#
# This file defines the GUI for a single critter agent.
#
#
# makeCritterView creates the top-level GUI for the critter.
#
# The created widget is named the same as the agent, but
# with a preceeding dot (".").  The widget contains a 
# grid like a tic-tac-toe board showing the agent square in
# the middile and the eight surrounding squares.  Since the 
# map is considered four-connected rather than eight-connected, 
# only the north south, east, and west squares are shown.  The 
# other squares are hidden in black.  The normal button <Enter> 
# binding is disabled so that user is not led to believe that 
# the button is active.

# Global decls

global soar_library auto_path
global critter_world_dir
global input_root wall_char space_char map_file stop_cw

puts stdout "critter_gui: auto_path= $auto_path\n packages = [package names]" ; flush stdout
package require Tk 4.2
wm withdraw .


proc makeCritterView {agent} {
    set w .$agent
    catch {destroy $w}

    toplevel $w
    wm title $w "Agent $agent view"
    wm iconname $w "$agent view"

    frame $w.view -width 200 -height 200 -relief ridge

    for {set i 0} {$i < 9} {incr i} {
	set xpos [expr ($i%3)*.33]
	set ypos [expr ($i/3)*.33]
	button $w.view.$i 
	place $w.view.$i -relx $xpos -rely $ypos \
		-relwidth .33 -relheight .33
        bind $w.view.$i <Enter> {doNothing}
        if {($i%2) == 0} {
	    $w.view.$i configure -bg black -relief flat
	} else {
	    $w.view.$i configure -relief raised
	}
    }
    pack $w.view -padx 2m -pady 2m
}

# This procedure is used to disable the button <Enter> callback.
# If you try "bind widget <Enter> {}" it doesn't work -- it 
# simply uses the default binding.

proc doNothing {} {
}

# updateCritterView updates the squares that the critter can
# see.  Positions 1, 3, 5, 7 correspond to north, west, east, 
# south respectively.  The background is set to Bisque in case
# it was highlighted earlier with another color.

proc updateCritterView {w pos1 pos7 pos5 pos3} {
    for {set i 1} {$i < 9} {incr i 2} {
	$w.view.$i configure -text [set pos$i] -background Bisque
    }
    update
}

# This procedure takes a selected direction and highlights
# the corresponding square with the given highlight color.

proc highlightCritterViewSquare {w square highlight} {
    if       {$square == "north"} {
	$w.view.1 configure -background $highlight
    } elseif {$square == "south"} {
	$w.view.7 configure -background $highlight
    } elseif {$square == "east" } {
	$w.view.5 configure -background $highlight
    } elseif {$square == "west" } {
	$w.view.3 configure -background $highlight
    }
}

# selectDirection is invoked from a Soar production RHS call.
# It sends the control agent the selected direction.  The 
# corresponding square is also highlighted.

proc selectDirection {dir} {
  global interp_name

  # Note that "sim" is aliased to the master interpreter
  sim $interp_name $dir

  highlightCritterViewSquare .$interp_name $dir green
}

# Now, create the GUI.

global interp_name
makeCritterView $interp_name


