#
#  Eight puzzle demo
#
#-----------------------------------------------------------------------
#
# Create top-level window for 8 puzzle demo.
#
# Any existing widget with the same name is destroyed.  The window
# consists of a title at the top, a panel of three boards and some
# control buttons.  The panel contains the starting state, current
# state, and goal state of the puzzle.
#package require Soar


proc make8Puzzle {{w .8} 
	          {start {2 8 3 1 6 4 7 0 5}}
	          {goal  {1 2 3 8 0 4 7 6 5}}} { 
    global solveslow

    catch {destroy $w}
    toplevel $w
    wm title $w "8-Puzzle Demonstration"
    wm iconname $w "8-Puzzle"

    message $w.msg -font -Adobe-times-medium-r-normal--*-180* \
            -aspect 400 -text "8-Puzzle Demonstration"

    frame $w.frame -width 360 -height 120 -borderwidth 4 \
	    -relief ridge -bg Grey70

    make8Board "Starting \nState" $w.frame.ss $start
    make8Board "Current \nState"  $w.frame.cs $start
    make8Board "Goal \nState"     $w.frame.gs $goal

    button $w.solve -text Solve -command "run"
    checkbutton $w.speed -text Slowly -variable solveslow
    button $w.stop -text "Stop" -command "stop-soar"
    button $w.init -text "Init Soar" \
           -command "init-soar; puzzleSet $w.frame.cs \{$start\}"
    button $w.reset -text Reset \
	    -command "reset8Puzzle $w \{$start\}"
    button $w.ok -text Dismiss -command "dismiss $w"
       
    pack $w.msg -side top
    pack $w.frame.ss $w.frame.cs $w.frame.gs -side left
    pack $w.frame -side top -padx 20
    pack $w.solve $w.speed $w.stop $w.init $w.reset \
         -side left -padx 1m -pady 2m
    pack $w.ok -side right -padx 1m -pady 2m 
}

# Procedure to make a single 8-puzzle board.  The name of 
# the state is used as a title for the board.  The board
# itself is simply nine titles in a square.

proc make8Board {title w state} {
    frame $w -width 160 -height 120 -borderwidth 0 
    message $w.msg -font -Adobe-times-medium-r-normal--*-180* \
            -aspect 300 -text $title
    frame $w.board -width 110 -height 110 -borderwidth 5 \
          -relief groove -bg Grey70

    for {set i 0} {$i < 9} {incr i} {
	set num [lindex $state $i]
	if {$num != 0} {
        set xpos($num) [expr ($i%3)*.33]
	set ypos($num) [expr ($i/3)*.33]
	button $w.board.$num -relief raised -text $num 
        place $w.board.$num -relx $xpos($num) -rely $ypos($num) \
	      -relwidth .33 -relheight .33
	}
    }

    pack $w.msg $w.board -side top -pady 5
}

#-----------------------------------------------------------------------
#
# Procedure to reorder the puzzle entries.  The buttons are 
# moved to correspond to the given order.  After the buttons
# are moved, the windows are updated to show the new positions
# (the Tk call "update" flushes pending updates to the X server).
# If the user has requested that the process move slowly, then
# sleep for a second.  It actually takes a little longer to
# exec the process, but whatever.

proc puzzleSet {w order} {
    global solveslow
    for {set i 0} {$i < 9} {incr i} {
	set num [lindex $order $i]
	if {$num != 0} {
          set xpos($num) [expr ($i%3)*.33]
	  set ypos($num) [expr ($i/3)*.33]
          place $w.board.$num -relx $xpos($num) -rely $ypos($num)
	}
    }

    update

    if {$solveslow != 0} {
	after 1000
    }
}

# This procedure resets a puzzle to the initial state.  It also
# removes all chunks and initializes Soar so that the puzzle can
# be run again.

proc reset8Puzzle {w start} {
    excise -chunks
    init-soar
    puzzleSet $w.frame.cs $start
}

# This procedure dismisses the 8puzzle demo.  It excises all 
# productions (maybe not a good thing to do?
# and does an init-soar so other demos can be run from the gui.

proc dismiss {w} {
    excise -all
    init-soar
    destroy $w
}
#-----------------------------------------------------------------------
#
# Initialization and GUI creation

# Load rules defining eight puzzle solution method

global soar_library
global interp_type

excise -all
source eight-puzzle.soar

set solveslow 0

make8Puzzle
reset8Puzzle .8 {2 8 3 1 6 4 7 0 5}

# Redefine one rule to use a Tcl call to update GUI:

sp {eight*monitor*state                   
  (state <s> ^problem-space <p> 
	     ^binding <x11> <x12> <x13>
	     <x21> <x22> <x23>
	     <x31> <x32> <x33>)
  (<p> ^name eight-puzzle)
  (<x11> ^cell.name c11 ^tile <o11>)
  (<o11> ^name <v11>)
  (<x12> ^cell.name c12 ^tile <o12>)
  (<o12> ^name <v12>)
  (<x13> ^cell.name c13 ^tile <o13>)
  (<o13> ^name <v13>)
  (<x21> ^cell.name c21 ^tile <o21>)
  (<o21> ^name <v21>)
  (<x22> ^cell.name c22 ^tile <o22>)
  (<o22> ^name <v22>)
  (<x23> ^cell.name c23 ^tile <o23>)
  (<o23> ^name <v23>)
  (<x31> ^cell.name c31 ^tile <o31>)
  (<o31> ^name <v31>)
  (<x32> ^cell.name c32 ^tile <o32>)
  (<o32> ^name <v32>)
  (<x33> ^cell.name c33 ^tile <o33>)
  (<o33> ^name <v33>)
  -->
  (tcl |puzzleSet .8.frame.cs {|
	 <v11> | | <v21> | | <v31> | |
	 <v12> | | <v22> | | <v32> | |
	 <v13> | | <v23> | | <v33> |}|)
  (write (crlf)  |      -------------|  | | (crlf) | | )
  (write |     \||  | | <v11>  | | |\||  | | <v21>  | | |\||  | | <v31>  | | |\||  | | (crlf) | | )
  (write |     \|---\|---\|---\||  | | (crlf) | | )
  (write |     \||  | | <v12>  | | |\||  | | <v22>  | | |\||  | | <v32>  | | |\||  | | (crlf) | | )
  (write |     \|---\|---\|---\||  | | (crlf) | | )
  (write |     \||  | | <v13>  | | |\||  | | <v23>  | | |\||  | | <v33>  | | |\||  | | (crlf) | | )
  (write |     -------------|  | | (crlf) | | )}
