rename eaterStartLocation stdEaterStartLocation
proc eaterStartLocation {} {
   global testbedStartIndex

    puts "RESET EATERS!!!"
    switch $testbedStartIndex {
	
	1 {
	    set placementArgs [list 1 1 0]
	}
	2 {
	    set placementArgs [list 1 15 1]
	}
	3 {
	    set placementArgs [list 4 11 0]
	}
	4 {
	    set placementArgs [list 13 2 3]
	}
	5 {
	    set placementArgs [list 2 7 4]
	}
	default {
	    set placementArgs [list -1 -1 -1]
	}
    }

}



rename makeBoard stdMakeBoard
proc makeBoard {} {
    
    global testbedStartIndex testbedMapIndex gridSize boardColor \
	boardX boardY eaterList


    if { $testbedMapIndex < 6 } {
	set boardX 15
	set boardY 15
    }

        if [winfo exists .wGlobal] {
	destroy .wGlobal
      ## making the board gets confused if it thinks there are eaters
      ## around, so temporarily pretend there aren't any
      if [info exists eaterList] {
         set tempEaterList $eaterList
         unset eaterList
      }
   }  
   toplevel .wGlobal
   wm title .wGlobal "Eaters Playing Board"
   wm geometry .wGlobal +0+0
   ### Try to make the board a nice size
   set gridSize [expr [expr [winfo screenwidth .wGlobal] / 3] / \
                      [expr 2 + $boardX]]
   canvas .wGlobal.c -width [expr ($boardX+2) * $gridSize] \
                   -height [expr ($boardY+2) * $gridSize] \
                   -background $boardColor
   pack .wGlobal.c -side top
   ## First fill the world with food
   for {set i 1} {$i <= $boardX} {incr i} {
      for {set j 1} {$j <= $boardY} {incr j} {
         createFood .wGlobal.c $i $j
      }
   }
   ## Put down the border walls
   for {set i 0} {$i <= ($boardX+1)} {incr i} {
      createWall .wGlobal.c $i 0
      createWall .wGlobal.c $i [expr $boardY+1]
   }
   for {set j 1} {$j <= $boardY} {incr j} {
      createWall .wGlobal.c 0 $j
      createWall .wGlobal.c [expr $boardX+1] $j
   }

    puts "MAP INDEX: $testbedMapIndex"
    switch $testbedMapIndex {

	1 { 
	    # Empty
	}

	2 {
	    for {set x 3} {$x < 9} {incr x} {
		createWall .wGlobal.c $x 4 
	    }
	    for {set x 2} {$x < 6} {incr x} {
		createWall .wGlobal.c $x 7
	    }
	    for {set x 3} {$x < 13} {incr x} {
		createWall .wGlobal.c $x 12
	    }
	    for {set y 5} {$y < 9} {incr y} {
		createWall .wGlobal.c 11 $y
	    }
	    for {set y 6} {$y < 9} {incr y} {
		createWall .wGlobal.c 13 $y
	    }
	    for {set y 9} {$y < 10} {incr y} {
		createWall .wGlobal.c 3 $y
	    }
	}
	3 {
	    for {set x 5} {$x < 9} {incr x} {
		createWall .wGlobal.c $x 2 
	    }
	    for {set x 2} {$x < 10} {incr x} {
		createWall .wGlobal.c $x 5 
	    }
	    for {set x 7} {$x < 13} {incr x} {
		createWall .wGlobal.c $x 12
	    }
	    for {set y 7} {$y < 11} {incr y} {
		createWall .wGlobal.c 11 $y
	    }
	    for {set y 7} {$y < 12} {incr y} {
		createWall .wGlobal.c 4 $y
	    }
	    for {set y 9} {$y < 13} {incr y} {
		createWall .wGlobal.c 2 $y
	    }
	    createWall .wGlobal.c 11 3
	    createWall .wGlobal.c 13 9
	    
	}
	Random {
	    stdMakeBoard
	    return
	}
	
	default {}

	

    }
   ## Now put the eaters back
   if [info exists tempEaterList] {
      set eaterList $tempEaterList
   }
   resetEaters
}




global currentColor
set currentColor 0


global xplacer yplacer
set xplacer 100
set yplacer 100

### Change control panel name
wm title . "Eaters TestBed Control Panel"

### Change around the File menu
### delete the original Exit entry, so it can stay at the bottom
.menu.file.m delete Exit
.menu.file.m add command -label "Reload Eater name menu" \
       -command {loadEaterNames .agents.create.e}
.menu.file.m add command -label Exit -command exit



.agents.create.createAgentButton configure -text {Create Eater} \
    -command {startEater .wGlobal.c $tsiCurrentAgentSourceFile $currentColor }

   
pack forget .agents.create.createAgentButton
pack .agents.create.createAgentButton -side left

## Get rid of the normal TSI focus behavior
bind . <Enter> {}


frame .mapcontrol -borderwidth 2
frame .mapcontrol.inner
global eaterList eaterTick eaterScore

label .mapcontrol.inner.mapLbl -text {Map Index:}
tk_optionMenu .mapcontrol.inner.mapIndex testbedMapIndex 1 2 3 4 5 Random

label .mapcontrol.inner.startPosLbl -text {Starting Position:}
tk_optionMenu .mapcontrol.inner.startPosIndex testbedStartIndex 1 2 3 4 5 Random

button .mapcontrol.inner.setUpMap -text {Set Up Map} \
    -command makeBoard
 
pack .mapcontrol.inner.mapLbl .mapcontrol.inner.mapIndex \
     .mapcontrol.inner.startPosLbl .mapcontrol.inner.startPosIndex \
     .mapcontrol.inner.setUpMap -side left
pack .mapcontrol.inner -side top
pack .mapcontrol -side top -fill x -padx 10


### Now we can display the window
wm deiconify .


makeBoard