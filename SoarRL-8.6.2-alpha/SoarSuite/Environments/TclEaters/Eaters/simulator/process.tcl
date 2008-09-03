#
# $Id$
# $Log$
# Revision 1.7  2005/06/24 20:32:05  rmarinie
# added some comments
#
# Revision 1.6  2005/06/24 19:50:44  rmarinie
# better handling of run/stop (avoids runSimulation not being defined if run from debugger)
#
# Revision 1.5  2005/06/12 21:37:02  rmarinie
# some workarounds to increase stability
#
# Revision 1.4  2005/06/10 05:05:35  kcoulter
# added  StopAllAgents when food gone
#
# Revision 1.3  2005/06/10 04:13:24  kcoulter
# added updateInputLink at agent creation
#
# Revision 1.2  2005/06/10 03:47:19  kcoulter
# converted to SML events
#
# Revision 1.1  2005/06/01 19:14:13  rmarinie
# initial commit of sml eaters
#
# Revision 1.3  2004/07/12 15:25:56  rmarinie
# fixed bugzilla bug 391
#
# Revision 1.2  2003/10/21 18:26:42  snason
#  hopefully eliminated error message after init-soar
#
# Revision 1.1.1.1  2003/06/16 13:48:35  swallace
# eaters initial cvs version (3.0.5)
#
# Revision 8.7  1998/10/26 15:48:50  swallace
# Prior to Release for 494
#
# Revision 8.6  1998/10/25 14:52:30  swallace
# Synced w/ Mazin
#
# Revision 8.5  1998/10/08 13:54:45  swallace
# Released to John
#
# Revision 8.4  1998/10/07 13:12:12  swallace
# 8.1 Safe
#
# Revision 8.3  1998/10/05 12:55:48  swallace
# Release 2.0a (w/ new tsi30alpha) and new control panel etc.
#
# Revision 8.2  1998/10/02 13:49:18  swallace
# *** empty log message ***
#
# Revision 8.1  1998/09/18 16:44:35  swallace
# Works fine, but code is dirty.  About to clean up.
#
# Revision 7.4  1998/08/21 12:10:46  swallace
# seems to work with remote agent proprioceptic feedback
#
# Revision 7.3  1998/08/19 15:44:12  swallace
# proprioceptic feedback added - movecount remains
#
# Revision 7.2  1998/08/19 13:03:54  swallace
#  All Fixes :) except for propreoceptic feedback
#
# Revision 7.1  1998/06/23 13:45:21  swallace
# Works w/ eaters in Run Mode & Step Mode
# Works with CLIPS & soar.
#
# Revision 6.1  1998/06/02 19:02:53  swallace
#  allows concurrent remote/local agents.
#
# Revision 5.1  1998/06/02 16:51:59  swallace
# 2 side step & run
#
# Revision 4.1  1998/06/01 22:15:42  swallace
# works using step on both sides.
#
# Revision 3.2  1998/05/28 20:21:29  swallace
# ready to work on WME wrappers
#
# Revision 3.1  1998/05/21 18:34:11  swallace
# Client/Server Interface allows info to be passed to client.
#
# Revision 2.1  1998/05/19 17:22:53  swallace
# *** empty log message ***
#
# Revision 1.2  1998/05/19 05:05:01  swallace
# works but soar is not completely seperated.
#
#
#

global smlStopNow
set smlStopNow 0

global heading
set heading(north) 90
set heading(east) 0
set heading(south) 270
set heading(west) 180

global oppositeDir
set oppositeDir(north) south
set oppositeDir(east) west
set oppositeDir(south) north
set oppositeDir(west) east


global deltax deltay
set deltax(north) 0
set deltay(north) -1
set deltax(east) 1
set deltay(east) 0
set deltax(south) 0
set deltay(south) 1
set deltax(west) -1
set deltay(west) 0

### cbc: counter for number of turns passed so far
global turn
set turn 0        


if {[info exists tsiConfig(sioDebug)] && $tsiConfig(sioDebug) > 2} { set boardWalls 0 } 

proc goalIsAccomplished {} {
    global eaterList eaterX eaterY gridSize numberOfMoves

    # The goal is Never Accomplished!
    return 0

}


proc envDestroyAgent { name } { 

# Nothing needed at this point

}
    
proc envLoadMap {path file} {
	
	cd $path
	source $file
}
	


proc initMap {} {
   global gridSize boardColor boardWalls boardX boardY eaterList
   
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
   frame .wGlobal.fScore
   
   
   pack .wGlobal.fScore -side left
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

    .wGlobal.c bind eater <Button-1> {selectEater %W %x %y}
    bind .wGlobal.c <Shift-ButtonPress-1> {setKeyInits %x %y}
    bind .wGlobal.c <Shift-ButtonRelease-1> {
	eval "startEater [getLocationVector %x %y] $tsiCurrentAgentSourceDir $tsiCurrentAgentSourceFile $currentAgentColor" 
	updateAvailableAgentColors
    }

    if [info exists tempEaterList] {
	return $tempEaterList
    }
    return 0

}

proc setKeyInits {x y} {
    global key_initialx key_initialy
	set key_initialx $x
	set key_initialy $y
}

proc getLocationVector {x y} {
    global key_initialx key_initialy gridSize
	
    # Determine dominant motion axis
    if {[expr abs($key_initialy - $y)] > [expr abs($key_initialx - $x)]} {
	set key_initialx -1
    } else {
	set key_initialy -1
    }
    
    # Set initial direction
    if {$key_initialy != -1} {
	if {$y < $key_initialy} {
	    # North
	    set dir 0		
	} else {
	    # South
	    set dir 2
	}
    }
    if {$key_initialx != -1} {
	if {$x < $key_initialx} {
	    # West
	    set dir 3
	    
	} else {
	    # East
	    set dir 1
	}
    }

    set x [.wGlobal.c canvasx $x]
    set y [.wGlobal.c canvasy $y]

    set x [expr floor($x/$gridSize)]
    set y [expr floor($y/$gridSize)]
    

    return [list $x $y $dir]
}
    

proc loadMap {} {
    source [file join simulator board1.map]
}
proc randomMap {} {
    global eaterList 

    set eaters [initMap]
  
    ## And add some random walls (these will delete any food they land on)
    randomWalls .wGlobal.c 
    
    if { $eaters != 0 } {
	set eaterList $eaters
    }
    
    resetEaters
}

proc selectEater {w x y} {
    global currentEater eaterObjectName

  
    set x [$w canvasx $x]
    set y [$w canvasy $y]
    
    set whichOne [$w find closest $x $y]
    if { [lsearch [$w gettags $whichOne] eater] < 0 } {
	return
    } else {

	if [info exists currentEater] {
	    pack forget .wAgentInfo.$currentEater 
	}
	set currentEater $eaterObjectName($whichOne)
	pack .wAgentInfo.$currentEater -side left -padx 3 -pady 3
    }

    return $whichOne
    
}
    
proc resetEaters {} {
   global eaterList eaterTick eaterScore worldCount agentMoved \
       numberOfMoves localAgents

   if [info exists eaterList] {
      foreach eater $eaterList {
         set eaterTick($eater) 0
         set eaterScore($eater) 0
         set numberOfMoves($eater) 0
	 eval placeEater $eater [eaterStartLocation]

      }
   }
    if [info exists localAgents] {
	foreach eater $localAgents {
	    $eater eval tsiDisplayAndSendCommand init-soar
	}
    }
   set worldCount 0
}

proc eaterStartLocation {} {
    return [list -1 -1 -1]
}

proc restartMap {} {
   global boardX boardY
   ## Refill the empty spaces in the world with food
   for {set i 1} {$i <= $boardX} {incr i} {
      for {set j 1} {$j <= $boardY} {incr j} {
         if {![wallExists .wGlobal.c $i $j] && ![foodExists .wGlobal.c $i $j]} {
            createFood .wGlobal.c $i $j
         }
      }
   }
   ## Put the eaters back down
   resetEaters
}

proc createWall {w x y} {
   global gridSize wallEdgeColor wallColor
   ## destroy anything that might be where we are about to put the wall
   set contents [$w find enclosed [expr $x*$gridSize-1] [expr $y*$gridSize-1] \
                           [expr ($x+1)*$gridSize] [expr ($y+1)*$gridSize]]
   deleteObjects $w $contents
   $w create rectangle [expr $x*$gridSize] [expr $y*$gridSize] \
                       [expr ($x+1)*$gridSize-1] [expr ($y+1)*$gridSize-1] \
                       -outline $wallEdgeColor -fill $wallColor -tags wall
}

proc deleteObjects {w objs} {
   ### We don't need to maintain a deleteList for the input link any more
   ### since input is sensor-based now
   eval $w delete $objs
}


proc createFood {w x y} {
   global gridSize foodSize foodEdgeColor normalfoodColor bonusfoodColor \
       tsiConfig
   ## squares whose x-coordinate is a multiple of 3, contain
   ## bonus food, while the rest of the squares contain normal food
   if {[expr $x%3] != 0} {
        $w create oval [expr ($x+0.5-($foodSize/2)) * $gridSize] \
                  [expr ($y+0.5-($foodSize/2)) * $gridSize] \
                  [expr ($x+0.5+($foodSize/2)) * $gridSize] \
                  [expr ($y+0.5+($foodSize/2)) * $gridSize] \
                  -outline $foodEdgeColor -fill $normalfoodColor \
	          -tags normalfood
   } else {
       $w create rectangle [expr ($x+0.5-($foodSize/2)) * $gridSize] \
                  [expr ($y+.5-($foodSize/2)) * $gridSize] \
                  [expr ($x+.5+($foodSize/2)) * $gridSize] \
                  [expr ($y+.5+($foodSize/2)) * $gridSize] \
                  -outline $foodEdgeColor -fill $bonusfoodColor \
                  -tags bonusfood
 #$w create text [expr ($x+.5)*$gridSize] [expr ($y+.5)*$gridSize] \
 #    -text {*} -font 8x13bold -fill $bonusfoodColor  -tags bonusfood


       }
}

proc wallExists {w x y} {
   global gridSize
   set contents [$w find enclosed [expr $x*$gridSize-1] [expr $y*$gridSize-1] \
                              [expr ($x+1)*$gridSize] [expr ($y+1)*$gridSize]]
   foreach c $contents {
      if {[lsearch [$w itemcget $c -tags] wall] >= 0} {
         return 1
      }
   }
   return 0
}

proc foodExists {w x y} {
   global gridSize
   set contents [$w find enclosed [expr $x*$gridSize-1] [expr $y*$gridSize-1] \
                              [expr ($x+1)*$gridSize] [expr ($y+1)*$gridSize]]
   foreach c $contents {
      if {([lsearch [$w itemcget $c -tags] normalfood] >= 0) || \
          ([lsearch [$w itemcget $c -tags] bonusfood] >= 0)} {
         return 1
      }
   }
   return 0
}

proc randomWalls {w} {
   global gridSize boardX boardY boardWalls


   for {set i 1} {$i <= $boardWalls} {incr i} {
      set x [expr [rand [expr $boardX-2]]+2]
      set y [expr [rand [expr $boardY-2]]+2]
      set length [expr [rand [expr ($boardX+$boardY)/4]] + 1]
      set dx 0
      set dy 0
      switch [rand 4] {
         0 {set dy -1;
            set length [expr [rand [expr $boardY-2]]+2]}
         1 {set dx 1;
            set length [expr [rand [expr $boardX-2]]+2]}
         2 {set dy 1;
            set length [expr [rand [expr $boardY-2]]+2]}
         3 {set dx -1;
            set length [expr [rand [expr $boardX-2]]+2]}
      }
      ## Don't let any walls touch any other walls, so we can make sure that
      ## all points on the board are reachable.
      ## Before actually drawing the wall, me find out which squares are
      ## ineligible
      set noWall ""
      set tx $x
      set ty $y
      for {set j 1} {$j <= $length} {incr j} {
         set stopLoop 0
         if {($tx < 1) || ($tx > $boardX) || ($ty < 1) || ($ty > $boardY)} {
            lappend noWall $j
            set stopLoop 1
         }
         if !$stopLoop {
            for {set k [expr $tx-1]} {$k <= ($tx+1)} {incr k} {
               for {set l [expr $ty-1]} {$l <= ($ty+1)} {incr l} {
                  if [wallExists $w $k $l] {
                     lappend noWall $j
                     set stopLoop 1
                     break
                   }
               }
               if $stopLoop {break}
            }
         }
         incr tx $dx
         incr ty $dy
      }
      ## Now let's actually draw the sucker
      for {set j 1} {$j <= $length} {incr j} {
         if {[lsearch $noWall $j] < 0} {
            createWall $w $x $y
         }
         incr x $dx
         incr y $dy
      }
   }
}

proc drawEater {w name} {
   global ticksPerEaterCycle eaterOpenMouth heading \
          eaterEdgeColor eaterSize gridSize \
          eaterX eaterY eaterColor eaterTick eaterDir eaterScore \
          eaterObjectName eaterObject openSize

   if {$eaterTick($name) < ($ticksPerEaterCycle*.5)} {

      set openSize [expr ($eaterTick($name)/($ticksPerEaterCycle*.5)) *  \
                         $eaterOpenMouth]
   } else {

      set openSize [expr (($ticksPerEaterCycle-$eaterTick($name)) /  \
                          ($ticksPerEaterCycle*.5))*$eaterOpenMouth]
   }

   set eater [$w find withtag $name]

   if {$eater != ""} {
      $w coords $eater [expr $eaterX($name)+$gridSize*.5*(1-$eaterSize)] \
                       [expr $eaterY($name)+$gridSize*.5*(1-$eaterSize)] \
                       [expr $eaterX($name)+$gridSize*.5*(1+$eaterSize)] \
                       [expr $eaterY($name)+$gridSize*.5*(1+$eaterSize)]
      $w itemconfigure $eater \
         -start [expr $heading($eaterDir($name))+($openSize/2)] \
         -extent [expr 359-$openSize]
   } else {
      set eater [$w create arc \
                   [expr $eaterX($name)+0.5*$gridSize*(1-$eaterSize)] \
                   [expr $eaterY($name)+0.5*$gridSize*(1-$eaterSize)] \
                   [expr $eaterX($name)+0.5*$gridSize*(1+$eaterSize)] \
                   [expr $eaterY($name)+0.5*$gridSize*(1+$eaterSize)] \
                   -start [expr $heading($eaterDir($name))+($openSize/2)] \
                   -extent [expr 359-$openSize] \
                   -outline $eaterEdgeColor -fill $eaterColor($name) \
                   -tags "eater $name"]
      set eaterObjectName($eater) $name
      set eaterObject($name) $eater
   }
}

proc tickSimulation {w} {
   global globalTick ticksPerMove gridSize ticksPerEaterCycle deltax deltay \
          eaterList eaterDir eaterColor eaterTick eaterScore \
          eaterX eaterY eaterOldX eaterOldY eaterObjectName \
          normalfoodScore bonusfoodScore currentBumpList deleteList \
          moveDir jumpDir initialdir oldtx oldty jumpPenalty agentMoved \
      tsiConfig numberOfMoves

   if { $tsiConfig(sioDebug) > 3 } { puts "tickSimulation -- (begin)" }

   if [info exists globalTick] {
      set globalTick [expr ($globalTick + 1) % $ticksPerMove]
   } else {
      set globalTick 1
   }
   
   if [info exists eaterList] {
      foreach eaterName $eaterList {
         set eater [$w find withtag $eaterName]
         if {$eater == ""} {
            tk_dialog .error Warning \
                      "$eaterName eater canvas object not found" warning 0 Ok
            continue
         }
         if [info exists eaterTick($eaterName)] {
            set eaterTick($eaterName) \
                [expr ($eaterTick($eaterName)+1)%$ticksPerEaterCycle]
         } else {
            set eaterTick($eaterName) 0
         }
         ## If we have completed a global tick cycle, move the eaters, 
         ## otherwise just redraw them with a new mouth
         if !$globalTick {

            #SB May 02 1997
            ## The direction should cause just a step-move instead of keep
            ## the agent moving forever

            ## oldtx and oldty are the eater's current location
            ## tx and ty correspond to the potential new location
            ## The eater can't move to the new location if there's a wall
            ## there, and all four numbers need to be saved in order to
            ## check whether any eaters bumped into each other
            set oldtx($eaterName) $eaterX($eaterName)
            set oldty($eaterName) $eaterY($eaterName)

	    if { $tsiConfig(sioDebug) > 5 } { puts " !Global Tick... " }
            if [info exists moveDir($eaterName)] {

               set tx($eaterName) \
                   [expr $eaterX($eaterName) +  \
                         $gridSize*$deltax($moveDir($eaterName))]
               set ty($eaterName) \
                   [expr $eaterY($eaterName) +  \
                         $gridSize*$deltay($moveDir($eaterName))]
               set oldtx($eaterName) $tx($eaterName)
               set oldty($eaterName) $ty($eaterName)
               set eaterDir($eaterName) $moveDir($eaterName)
	       if { $tsiConfig(sioDebug) > 4 } { 
		   puts " ! $eaterName Moved $moveDir($eaterName)" 
	       }
	       unset moveDir($eaterName)
               set agentMoved($eaterName) 1

	       incr numberOfMoves($eaterName)
            } elseif [info exists jumpDir($eaterName)] {
               set tx($eaterName) \
                   [expr $eaterX($eaterName) +  \
                         $gridSize*2*$deltax($jumpDir($eaterName))]
               set ty($eaterName) \
                   [expr $eaterY($eaterName) +  \
                         $gridSize*2*$deltay($jumpDir($eaterName))]
               set oldtx($eaterName) $tx($eaterName)
               set oldty($eaterName) $ty($eaterName)
               incr eaterScore($eaterName) -$jumpPenalty
               set eaterDir($eaterName) $jumpDir($eaterName)
               unset jumpDir($eaterName)
               set agentMoved($eaterName) 1
		incr numberOfMoves($eaterName)
            } elseif [info exists initialdir($eaterName)] {
               set eaterDir $initialdir($eaterName)
               unset initialdir($eaterName)
            } elseif [info exists oldtx($eaterName)] {
               set tx($eaterName) $oldtx($eaterName)
               set ty($eaterName) $oldty($eaterName)
            }

            ## cx and cy are the center coordinates of the tx,ty grid square
            set cx($eaterName) [expr $tx($eaterName) + 0.5*$gridSize]
            set cy($eaterName) [expr $ty($eaterName) + 0.5*$gridSize]
            ## find out if anything is in the destination square
            set contents \
                [$w find overlapping $cx($eaterName) $cy($eaterName) \
                                     $cx($eaterName) $cy($eaterName)]
            ## if there is not a wall there, we can move there (for the time
            ## being)...if there is a wall there, we can't move, but record 
            ## where we are in case we get bumped.
            set eaterOldX($eaterName) $eaterX($eaterName)
            set eaterOldY($eaterName) $eaterY($eaterName)
            set noWall 1
            ### This assumes nothing is ever in a square along with a wall
            if {[lsearch [$w itemcget $contents -tags] wall] < 0} {
               set eaterX($eaterName) $tx($eaterName)
               set eaterY($eaterName) $ty($eaterName)
            }
         }
         drawEater $w $eaterName
      }
      ## If the eaters have just moved, check whether any have bumped into each
      ## other...if so, put them back where they were.
      if !$globalTick {
         ## Erase any old bumps that are drawn on the board
         foreach bump [$w find withtag bump] {
            $w delete $bump
            foreach agent $eaterList {
               if [info exists deleteList($agent)] {
                  lappend deleteList($agent) $bump
               } else {
                  set deleteList($agent) $bump
               }
            }
            if [info exists currentBumpList] {
               unset currentBumpList
            }
         }
         ## Now check whether any eaters just tried to pass through each
         ## other.  If so, they bumped, so move them back.
         for {set i 0} {$i < [expr [llength $eaterList] - 1]} {incr i} {
            set e1 [lindex $eaterList $i]
            for {set j [expr $i + 1]} {$j < [llength $eaterList]} {incr j} {
               set e2 [lindex $eaterList $j]
               if {($eaterX($e1)==$eaterOldX($e2)) && \
                   ($eaterY($e1)==$eaterOldY($e2)) && \
                   ($eaterX($e2)==$eaterOldX($e1)) && \
                   ($eaterY($e2)==$eaterOldY($e1))} {
                  set obj [drawBump $w [expr 0.5*($eaterX($e1)+$eaterX($e2))] \
                                       [expr 0.5*($eaterY($e1)+$eaterY($e2))]]
                  if [info exists currentBumpList] {
                     lappend currentBumpList [list $obj [list $e1 $e2]]
                  } else {
                     set currentBumpList [list [list $obj [list $e1 $e2]]]
                  }
                  bumpEaters $w $e1 $e2
               }
            }
         }
         ## Now see if any eaters are on the same square as each other, and
         ## bump them back.
         set bumpList ""
         foreach e $eaterList {
            ## This check is to prevent reflexive bumps (i.e., if a bumps
            ## into b, we don't also want b to bump into a)
            if {[lsearch $bumpList $e] < 0} {
               set contents [eval $w find overlapping [$w coords $e]]
               set bumpers ""
               foreach c $contents {
                  if {[lsearch [$w itemcget $c -tags] eater] >= 0} {
                     lappend bumpers $eaterObjectName($c)
                     lappend bumpList $eaterObjectName($c)
                  }
                  if {[llength $bumpers] >= 2} {
                     set obj [drawBump $w $eaterX($e) $eaterY($e)]
                     if [info exists currentBumpList] {
                        lappend currentBumpList [list $obj $bumpers]
                     } else {
                        set currentBumpList [list [list $obj $bumpers]]
                     }
                     eval bumpEaters $w $bumpers
                  }
               }
            }
         }
         ## Finally, see if we are sitting on any food, and eat it.
         foreach e $eaterList {
            eatFood $w $e
         }
      }
   }
}

proc eatFood {w name} {
   global eaterScore normalfoodScore bonusfoodScore


   set contents [eval $w find overlapping [$w coords $name]]

   foreach c $contents {

      set lin [lsearch [$w itemcget $c -tags] normalfood]
      set lib [lsearch [$w itemcget $c -tags] bonusfood]

      if {[lsearch [$w itemcget $c -tags] normalfood] >= 0} {
         incr eaterScore($name) $normalfoodScore
         $w delete $c
      }

      if {[lsearch [$w itemcget $c -tags] bonusfood] >= 0} {
         incr eaterScore($name) $bonusfoodScore
         $w delete $c
      }
   }
}

proc bumpEaters {w args} {
   global eaterScore
#   puts "BUMP! $args"
   set totalScore 0
   foreach e $args {
      incr totalScore $eaterScore($e)
      eval placeEater $e
   }
   set averageScore [expr $totalScore/[llength $args]]
   foreach e $args {
      set eaterScore($e) $averageScore
   }
}


### Assume x,y describes the upper left corner of the grid square
proc drawBump {w x y} {
   global gridSize bumpOuterRadius bumpInnerRadius bumpColor
   set cx [expr $x+0.5*$gridSize]
   set cy [expr $y+0.5*$gridSize]
   set ox1 $cx
   set oy1 [expr $cy-$bumpOuterRadius]
   set ox2 [expr $cx+0.7*$bumpOuterRadius]
   set oy2 [expr $cy-0.7*$bumpOuterRadius]
   set ox3 [expr $cx+$bumpOuterRadius]
   set oy3 $cy
   set ox4 [expr $cx+0.7*$bumpOuterRadius]
   set oy4 [expr $cy+0.7*$bumpOuterRadius]
   set ox5 $cx
   set oy5 [expr $cy+$bumpOuterRadius]
   set ox6 [expr $cx-0.7*$bumpOuterRadius]
   set oy6 [expr $cy+0.7*$bumpOuterRadius]
   set ox7 [expr $cx-$bumpOuterRadius]
   set oy7 $cy
   set ox8 [expr $cx-0.7*$bumpOuterRadius]
   set oy8 [expr $cy-0.7*$bumpOuterRadius]
   set ix1 $cx
   set iy1 [expr $cy-$bumpInnerRadius]
   set ix2 [expr $cx+0.7*$bumpInnerRadius]
   set iy2 [expr $cy-0.7*$bumpInnerRadius]
   set ix3 [expr $cx+$bumpInnerRadius]
   set iy3 $cy
   set ix4 [expr $cx+0.7*$bumpInnerRadius]
   set iy4 [expr $cy+0.7*$bumpInnerRadius]
   set ix5 $cx
   set iy5 [expr $cy+$bumpInnerRadius]
   set ix6 [expr $cx-0.7*$bumpInnerRadius]
   set iy6 [expr $cy+0.7*$bumpInnerRadius]
   set ix7 [expr $cx-$bumpInnerRadius]
   set iy7 $cy
   set ix8 [expr $cx-0.7*$bumpInnerRadius]
   set iy8 [expr $cy-0.7*$bumpInnerRadius]
   return [$w create polygon $ox1 $oy1 $ix1 $iy1 $ox2 $oy2 $ix2 $iy2 \
                             $ox3 $oy3 $ix3 $iy3 $ox4 $oy4 $ix4 $iy4 \
                             $ox5 $oy5 $ix5 $iy5 $ox6 $oy6 $ix6 $iy6 \
                             $ox7 $oy7 $ix7 $iy7 $ox8 $oy8 $ix8 $iy8 \
                             -fill $bumpColor -tags bump]
}

#### simple pseudo-random number generator
global lastvalue
set lastvalue [expr [clock clicks]%65536]
proc rawrand {} {
    global lastvalue
# per Knuth 3.6:
# 65277 mod 8 = 5 (since 65536 is a power of 2)
# c/m = .5-(1/6)\sqrt{3}
# c = 0.21132*m = 13849, and should be odd.
    set lastvalue [expr (65277*$lastvalue+13849)%65536]
    set lastvalue [expr ($lastvalue+65536)%65536]
    return $lastvalue
}
proc rand {base} {
    set rr [rawrand]
    return [expr ($rr*$base)/65536]
}


proc newRange value {
   global eaterList sensorRange
   set sensorRange $value
   if [info exists eaterList] {
      foreach name $eaterList {
         drawSensorGrid $name
      }
   }
}

proc ldelete {eaterList name} {
      set ix [lsearch -exact $eaterList $name]
      if {$ix >= 0} {
        return [lreplace $eaterList $ix $ix]
      } else {
        return $eaterList
      }
}


rename destroyAgent ETCPDestroyAgent

proc destroyAgent {newname} {
   global eaterList eaterNameList eaterColor eaterColorByName

   ETCPDestroyAgent $newname

   set name [reverseEaterName $newname]
   set color [reverseEaterColor $newname]

    ### Clean up all the input-link stuff for this name, in case a new
    ### one comes along.  Have to do this before deleting the canvas
    ### object.
    removeInputLink $newname
    set obj [.wGlobal.c find withtag $newname]
    .wGlobal.c delete $obj
    destroy .wAgentInfo.$newname
    set eaterList [ldelete $eaterList $newname]
    if {$eaterList == {}} {
	destroy .wAgentInfo
    }
    set eaterNameList [ldelete $eaterNameList $name]
    #unset eaterColorByName($name)
    unset eaterColor($newname)

    updateAvailableAgentColors

}


proc eaterName {name color} {
   # return $name$color
   return $color
}

proc reverseEaterName {newname} {
   global eaterNameByNewname
   return $eaterNameByNewname($newname)
}

proc reverseEaterColor {newname} {
   global eaterColor
   return $eaterColor($newname)
}

proc addRemoteAgentToEnvironment {socket} {
   global possibleEaterColors eaterColor eaterTick eaterNameList agentMoved \
       eaterNameByNewname eaterColorByName soarTimeUnit moveCount \
       eaterScore eaterList tsiConfig numberOfMoves
   
    if { $tsiConfig(sioDebug) > 3 } {
	puts "addRemoteAgentToEnvironment  (begin)"
    }
    # First, grab an unused color.
    foreach color $possibleEaterColors {
	set newColor $color
	foreach {key usedColor} [array get eaterColor] {
	    if {[string compare $color $usedColor] == 0} {
		set newColor {nil}
		break 
	    }
	}
	if [string compare $newColor "nil"] { break }
    }

    if { $tsiConfig(sioDebug) } {
	puts "  -The Agent from socket: $socket is Colored: $newColor"
    }
    if { [string compare $newColor "nil"] == 0} {
	return "error"
    }
    set newname [eaterName $socket $newColor]
#    puts "CREATING REMOTE AGENT..."
    createNewRemoteAgent $newname $socket
#    puts "CREATED... REGISTERING"
    registerRemoteAgent $newname $socket
#    puts "REGISTERED"
    
    # This needs to be looked at, now it relys on the fact that
    # the eaters name corresponds to its socket id
    $newname alias sendInput sendInput $newname
 

    $newname alias setUpInputLink setUpInputLink $newname
    $newname alias updateInputLink updateInputLink $newname
    $newname alias updateOutputLink updateOutputLink $newname
    $newname alias removeInputLink removeInputLink $newname
    $newname alias setWalkDir setWalkDir $newname
    $newname alias setJumpDir setJumpDir $newname
 
    $newname alias run environmentRun
    $newname alias step environmentStep
    $newname alias stop environmentStop
  #  $newname alias stop-soar stop
 
    if {[info exists eaterColorByName($socket)]} {
	lappend eaterColorByName($socket) $color
	lappend eaterColor($newname) $color
    } else {
#	set eaterColorByName($socket)  $color
	set eaterColor($newname)  $color
    }

	set eaterNameByNewname($newname) $socket
	if [info exists eaterList] {
	    lappend eaterList $newname
	} else {
	    set eaterList $newname
	}
	if [info exists eaterNameList] {
	    lappend eaterNameList $socket
	} else {
	    set eaterNameList $socket
	}
	set eaterTick($newname) 0
	set eaterScore($newname) 0
	set agentMoved($newname) 0
        set numberOfMoves($newname) 0
        eval placeEater  $newname [eaterStartLocation]

	setupEaterSensor $newname $color
	
    updateAvailableAgentColors
	
	 if { $tsiConfig(sioDebug) > 3 } {
	     puts "addRemoteAgentToEnvironment (end)"
	 }
	return "ok $newname"

}




proc initEaterOutputFile {filename color eaterName} {

    global eaterOutputFile

    
    set fileID [open $filename-$color.log w 0660]
    puts "Opened EaterOutputFile for $eaterName"
    set eaterOutputFile($eaterName) $fileID
}

proc simulatorQuit {} {

    global eaterOutputFile


    if [info exists eaterOutputFile ] {
      foreach {agent file} [array get eaterOutputFile] {
  	puts "Closing $file"
  	close $file
      }
    }
}
    
proc startEater {x y dir path name color} {
   global eaterList eaterColor eaterTick eaterScore eaterNameList agentMoved \
          eaterNameByNewname eaterColorByName  \
          soarTimeUnit moveCount possibleEaterColors currentColor \
       tsiCurrentAgentSourceDir tsiSimulatorPath numberOfMoves \
       ETCPConfig soar_library

    if { $color == {} } {
	# we are out of colors.
	tk_dialog .error Error \
	    "The Maximum Number of Eaters has Already Been Created.  (Out of Colors)" error 0 Ok
	error "Out of Colors"
    } else {
	incr currentColor
    }

   set newname [eaterName $name $color]
   if {[info exists eaterList] && [lsearch $eaterList $newname] >= 0} {
           tk_dialog .error Error \
                     "An eater with color $newname already exists...not creating a new one\nYou must choose a different color." \
                     error 0 Ok
           error "Tried to create duplicate eaters"
   }

    # the new version of this routine should source this code automatically!
   createNewAgent $newname $tsiCurrentAgentSourceDir "$name.soar"

	if {![interp exists $newname]} {
	   return
	}

   #puts " adding proc aliases to agent:  $newname\n"
   $newname alias setUpInputLink setUpInputLink $newname
   $newname alias updateInputLink updateInputLink $newname
   $newname alias updateOutputLink updateOutputLink $newname
   $newname alias removeInputLink removeInputLink $newname
   $newname alias setWalkDir setWalkDir $newname
   $newname alias setJumpDir setJumpDir $newname
   ## $newname eval rename run run-soar
   $newname alias run environmentRun
   $newname alias step environmentStep
   $newname alias stop environmentStop
    #puts " after aliasing: \n [$newname aliases]"
#   $newname alias stop-soar stop
#   $newname eval {.tsw.frame.step configure \
#                         -command {tsiDisplayAndSendCommand step}}
    if $ETCPConfig(afterDecision) { 
#     ###KJC  $newname eval monitor -add after-decision-phase-cycle \"stop-soar -self \{\}\" dp1
#     changed to fix bugzilla bug 391
      ###KJC   $newname eval monitor -add after-decision-phase-cycle \"stop-soar -self" dp1
  	}

  ###KJC  FIXED AGENT.TCL to use new IO commands
   if [catch {$newname eval \
              [list source \
		   [file join $ETCPConfig(SimulatorPath) agent.tcl]]} msg] {
       tk_dialog .error Warning \
                "Couldn't load interface code: $msg" warning 0 Ok
   }
  # puts "after agent.tcl source'd"


    if {[info exists eaterColorByName($name)]} {
       lappend eaterColorByName($name) $color
       lappend eaterColor($newname) $color
   } else {
#       set eaterColorByName($name)  $color
       set eaterColor($newname)  $color
   }
   set eaterNameByNewname($newname) $name
   if [info exists eaterList] {
      lappend eaterList $newname
   } else {
      set eaterList $newname
   }
   if [info exists eaterNameList] {
      lappend eaterNameList $name
   } else {
      set eaterNameList $name
   }
   set eaterTick($newname) 0
   set eaterScore($newname) 0
   set agentMoved($newname) 0
   set numberOfMoves($newname) 0
   eval placeEater $newname $x $y $dir

   setupEaterSensor $newname $color
   $newname eval [list updateInputLink]

      ###KJC tried adding back in...
#	$newname eval addTSIProductionMenu .tsw

}

proc setupEaterSensor {name color} {
    global sensorGridSize sensorRange  currentEater


   ### Create the eater sensor display if it doesn't already exist.
   if ![winfo exists .wAgentInfo] {
      toplevel .wAgentInfo 
      ## Try to put the display in a convenient place
      set bx [winfo width .wGlobal]
      set my [winfo height .]
      set x [expr 15 + $bx]
      set y [expr 35 + $my]
      wm geometry .wAgentInfo +$x+$y
      wm title .wAgentInfo {Eaters Info}
      frame .wAgentInfo.general
      label .wAgentInfo.general.movelabel -text {World Count =}
      label .wAgentInfo.general.worldcount -textvariable worldCount
      pack .wAgentInfo.general.movelabel .wAgentInfo.general.worldcount \
           -side left
      frame .wAgentInfo.general.space -width 15
      pack .wAgentInfo.general.space -side left
    #  label .wAgentInfo.general.info -text {X = Reload agent code}
    #  pack .wAgentInfo.general.info -side right
      pack .wAgentInfo.general -side top -padx 3 -pady 3
   } 

   ### And then add in the sensor display for this eater.
   frame .wAgentInfo.$name
   frame .wAgentInfo.$name.info
  # button .wAgentInfo.$name.info.load -text "X" \
  \#        -command "loadController $name $color"
   frame .wAgentInfo.$name.info.numbers
   frame .wAgentInfo.$name.info.numbers.score
   label .wAgentInfo.$name.info.numbers.score.label -text "Score:"
   label .wAgentInfo.$name.info.numbers.score.value \
         -textvariable eaterScore($name)
   pack .wAgentInfo.$name.info.numbers.score.label \
        .wAgentInfo.$name.info.numbers.score.value -side left
   frame .wAgentInfo.$name.info.numbers.count
   label .wAgentInfo.$name.info.numbers.count.label -text "Moves:"
   label .wAgentInfo.$name.info.numbers.count.value \
         -textvariable numberOfMoves($name)
   pack .wAgentInfo.$name.info.numbers.count.label \
        .wAgentInfo.$name.info.numbers.count.value -side left
  
   pack .wAgentInfo.$name.info.numbers.score \
        .wAgentInfo.$name.info.numbers.count -side top
 #  pack .wAgentInfo.$name.info.load -padx 5 -pady 5 \
  \#      -side left
   pack .wAgentInfo.$name.info.numbers -side left
   canvas .wAgentInfo.$name.sensors \
          -width [expr $sensorGridSize*(2*$sensorRange+1)] \
          -height [expr $sensorGridSize*(2*$sensorRange+1)] \
          -relief raised 

   pack .wAgentInfo.$name.info .wAgentInfo.$name.sensors -side top
  

    
    if [info exists currentEater ] {

	pack forget .wAgentInfo.$currentEater
    }
   set currentEater $name
   pack .wAgentInfo.$name -side left -padx 3 -pady 3


   drawSensorGrid $name

}

proc drawSensorGrid {namecolor} {
   global drawnSensorRange sensorRange sensorGridSize
   if {![info exists drawnSensorRange($namecolor)] || \
       ($drawnSensorRange($namecolor) != $sensorRange)} {
      .wAgentInfo.$namecolor.sensors delete sensorGrid
      .wAgentInfo.$namecolor.sensors configure \
          -width [expr $sensorGridSize*(2*$sensorRange+1)] \
          -height [expr $sensorGridSize*(2*$sensorRange+1)]
   }
   for {set i -$sensorRange} {$i <= $sensorRange} {incr i} {
      for {set j -$sensorRange} {$j <= $sensorRange} {incr j} {
         .wAgentInfo.$namecolor.sensors create rectangle \
               [expr ($sensorRange+$i)*$sensorGridSize+2] \
               [expr ($sensorRange+$j)*$sensorGridSize+2] \
               [expr ($sensorRange+$i+1)*$sensorGridSize+1] \
               [expr ($sensorRange+$j+1)*$sensorGridSize+1] \
               -tags sensorGrid
      }
   }
   set drawnSensorRange($namecolor) $sensorRange
}

proc updateSensorDisplay {name} {
   global typeOfObj foodSize foodEdgeColor wallEdgeColor eaterEdgeColor \
          normalfoodColor bonusfoodColor wallColor \
          heading eaterDir openSize sensorRange sensorGridSize eaterList \
          eaterColor sensorEater eaterSize eaterObjectName
   .wAgentInfo.$name.sensors delete sensorInfo
   for {set i -$sensorRange} {$i <= $sensorRange} {incr i} {
      for {set j -$sensorRange} {$j <= $sensorRange} {incr j} {
         if [info exists typeOfObj($i,$j,$name)] {
            switch $typeOfObj($i,$j,$name) {
               bonusfood {
                  ## typeOfObj appears to use "normal" coordinates, but
                  ## that's not what the canvas uses.

                   #puts "Found Bonus food: $i, $j"
		   #.wAgentInfo.$name.sensors create text \
		  # 	[expr ($i+$sensorRange+.6)*$sensorGridSize] \
		#	[expr ($j+$sensorRange+.6)*$sensorGridSize] \
		#	-text {*} -font 8x13bold -fill $bonusfoodColor \
		#	-tags sensorInfo
		
                  .wAgentInfo.$name.sensors create rectangle \
                   [expr ($i+$sensorRange+.5-($foodSize/2))*$sensorGridSize+2] \
                   [expr ($j+$sensorRange+.5-($foodSize/2))*$sensorGridSize+2] \
                   [expr ($i+$sensorRange+.5+($foodSize/2))*$sensorGridSize+2] \
                   [expr ($j+$sensorRange+.5+($foodSize/2))*$sensorGridSize+2] \
                   -outline $foodEdgeColor -fill $bonusfoodColor \
                   -tags sensorInfo
               }
               normalfood {
                  .wAgentInfo.$name.sensors create oval \
                   [expr ($i+$sensorRange+.5-($foodSize/2))*$sensorGridSize+2] \
                   [expr ($j+$sensorRange+.5-($foodSize/2))*$sensorGridSize+2] \
                   [expr ($i+$sensorRange+.5+($foodSize/2))*$sensorGridSize+2] \
                   [expr ($j+$sensorRange+.5+($foodSize/2))*$sensorGridSize+2] \
                   -outline $foodEdgeColor -fill $normalfoodColor \
                   -tags sensorInfo
               }
               wall {
                  .wAgentInfo.$name.sensors create rectangle \
                        [expr ($i+$sensorRange)*$sensorGridSize+2] \
                        [expr ($j+$sensorRange)*$sensorGridSize+2] \
                        [expr ($i+$sensorRange+1)*$sensorGridSize+1] \
                        [expr ($j+$sensorRange+1)*$sensorGridSize+1] \
                        -outline $wallEdgeColor -fill $wallColor \
                        -tags sensorInfo
               }
               eater {
                .wAgentInfo.$name.sensors create arc \
                  [expr ($i+$sensorRange+.5-($eaterSize/2))*$sensorGridSize+2] \
                  [expr ($j+$sensorRange+.5-($eaterSize/2))*$sensorGridSize+2] \
                  [expr ($i+$sensorRange+.5+($eaterSize/2))*$sensorGridSize+2] \
                  [expr ($j+$sensorRange+.5+($eaterSize/2))*$sensorGridSize+2] \
                 -start [expr \
              $heading($eaterDir($eaterObjectName($sensorEater($i,$j,$name)))) \
                              + ($openSize/2)] \
                 -extent [expr 359-$openSize] \
                 -outline $eaterEdgeColor \
                 -fill $eaterColor($eaterObjectName($sensorEater($i,$j,$name)))\
                 -tags sensorInfo
               }
            }
         }
      }
   }
}


proc placeEater {name {xloc -1} {yloc -1} {orient -1}} {
   global boardX boardY gridSize eaterX eaterY eaterDir xplacer yplacer \
      tsiConfig

   set badlocation 1
    
   if { $tsiConfig(sioDebug) > 3 } { puts "placeEater -- (begin)" }
  
 
      
   if { $tsiConfig(sioDebug) > 2 } {
      puts "sioDebug -- placing 1st eater in NW corner"
      set x $gridSize
      set y $gridSize
   } else {
       if { $xloc > 0 && $xloc <= $boardX } {
	   set x [expr $xloc*$gridSize]
       } else {
	   set x [expr ([rand $boardX]+1)*$gridSize]
       }
       if { $yloc > 0 && $yloc <= $boardY } {
	   set y [expr $yloc*$gridSize]
       } else {
	   set y [expr ([rand $boardY]+1)*$gridSize]
       }
   }
   while {$badlocation} {

      set contents [.wGlobal.c find enclosed [expr $x-1] [expr $y-1] \
                                     [expr $x+$gridSize] [expr $y+$gridSize]]
      set gridOkay 1
      foreach c $contents {
         if {([lsearch [.wGlobal.c itemcget $c -tags] wall] >= 0) || \
             ([lsearch [.wGlobal.c itemcget $c -tags] eater] >= 0)} {
            set gridOkay 0
           break
         }
      }
      if $gridOkay {
         set eaterX($name) $x
         set eaterY($name) $y
         set badlocation 0
      }
      set x [expr ([rand $boardX]+1)*$gridSize]
      set y [expr ([rand $boardY]+1)*$gridSize]
   

   }

    if { $orient < 0 || $orient > 3 } {
	## pick a random direction to start, if one wasn't supplied
	set orient [rand 4]
    }
   if { $tsiConfig(sioDebug) > 2 } {
      puts "sioDegub -- setting eater direction 'East'"
      set orient 1 
   }
   switch $orient {
      0 {
         set eaterDir($name) north
        }
      1 {
         set eaterDir($name) east
        }
      2 {
         set eaterDir($name) south
        }
      3 {
        set eaterDir($name) west
        }
   }
   set initialdir($name) $eaterDir($name)
   drawEater .wGlobal.c $name
   eatFood .wGlobal.c $name

   if { $tsiConfig(sioDebug) > 3 } { puts "placeEater -- (end)" }
}

## These are the TSI versions.  If you wish to use them,
## you need to edit the button commands in et-controlpanel.tcl
proc environmentRun {args} {
       global runningSimulation

    tsiOnEnvironmentRun

       ### We'll try to be sneaky here, and assume that if someone gave run
       ### an argument (like in the TSI buttons and menus), they meant to
       ### step
       if {$args!=""} {
          set n 1
          scan $args %d n

	   for {set i 1} {$i <= $n} {incr i} {
             environmentStep
          }
          return
       }
       set runningSimulation 1
       runSimulation .wGlobal.c
}

proc environmentStep {} {
    global runningSimulation

    tsiOnEnvironmentStep
    set runningSimulation 0
    runSimulation .wGlobal.c
}

proc environmentStop {} {
    global runningSimulation

    # This can be called in one of three ways :
    # 1. By Pressing the STOP Button
    # 2. As a response to a client's stop-simulation request
    # 3. Via the RunSimulation Method as a result of runningSimulation == 0

    # in the first two cases, the simulation is still going, so all we must 
    # do is signal its halt by setting runningSimulation to 0
    # This will result in this method being called again at the proper halting
    # point, at which time tsiOnEnvironmentStop will be called.

    if { $runningSimulation } {
	set runningSimulation 0
#	puts "Stop Button Hit? Waiting for Cycle's end...."
	return
    }
    tsiOnEnvironmentStop
}

## new for Soar 8.6 SML events 
proc SMLenvironmentRun {args} {
       global runningSimulation _kernel smlStopNow

    tsiOnEnvironmentRun;  # included in case using tsiAgentWindow
    set runningSimulation 1
    set smlStopNow 0
    #   runSimulation .wGlobal.c
    $_kernel RunAllAgentsForever
}

proc SMLenvironmentStep {} {
    global runningSimulation _kernel smlStopNow

    tsiOnEnvironmentStep;  # included in case using tsiAgentWindow
    set runningSimulation 0
    #runSimulation .wGlobal.c
    $_kernel RunAllAgents 1
}

proc SMLenvironmentStop {} {
    global runningSimulation _kernel smlStopNow
    
    #if runningSimulation doesn't exist, then we know that we're stopped
    # since if we had run it would exist
    if { ![info exists runningSimulation] } {
	    set runningSimulation 0
    }
    
    set smlStopNow 1
    if { $runningSimulation } {
	set runningSimulation 0
#	puts "Stop Button Hit? Waiting for Cycle's end...."
	return
    }
    tsiOnEnvironmentStop
}

proc SMLenvironmentRunEvent {args} {
	global runningSimulation smlStopNow
	set runningSimulation 1
	set smlStopNow 0
}

proc SMLenvironmentStopEvent {args} {
	global runningSimulation smlStopNow
	set runningSimulation 0
	set smlStopNow 1
}

proc smlProcessUpdates {args} {
    global smlStopNow _kernel

    #puts "in smlProcessUpdates: "
    #puts " smlStopNow = $smlStopNow"

    ## the 8.6.1 event system doesn't fully implement stopping yet,
    ## so we explicitly check our own local var inside this callback
    if $smlStopNow {
	set smlStopNow 0
	$_kernel StopAllAgents
	$_kernel CheckForIncomingCommands
    }
    #puts "in smlProcessUpdates: calling runSimulation .wGlobal.c"
    runSimulation .wGlobal.c
}


###KJC modified June 2005 for SML compliance, order of I/O & updating changed
###The TSI version is below.

proc runSimulation {w} {
   global runningSimulation tickDelay soarTimePerTick soarTimeUnit \
          worldCount worldCountLimit tsiConfig agentUpdate
   global _kernel localAgents

   set agents [interp slaves]

    set agent0 [lindex $agents 0]
 #   puts "runSimulation -- 0 :: agent0 = $agent0"
   if {$agents == ""} {
      tk_dialog .error Warning "There are currently no eaters to run." \
                warning 0 Ok
      set runningSimulation 0
      SMLenvironmentStop
      return
   }
   if {([.wGlobal.c find withtag normalfood] == {}) && \
       ([.wGlobal.c find withtag bonusfood] == {})} {
      set runningSimulation 0
       SMLenvironmentStop; $_kernel StopAllAgents
       $_kernel CheckForIncomingCommands
      tk_dialog .info {Game Over} {Game over: All the food is gone.} info 0 Ok
      return
   }
    
   if {$worldCount >= $worldCountLimit} {
      set runningSimulation 0
      SMLenvironmentStop; $_kernel StopAllAgents
      $_kernel CheckForIncomingCommands
      tk_dialog .info {Game Over} \
                      {Game over: Move count limit reached.} info 0 Ok
      return
   }
    if [goalIsAccomplished] {
	
	set runningSimulation 0
	SMLenvironmentStop; $_kernel StopAllAgents
	$_kernel CheckForIncomingCommands
	tk_dialog .info {Goal Accomplished} \
	    {The goal has been accomplished. Game Over.} info 0 Ok
	return
    }

    ## process the agent output
    if [info exists localAgents] {
        foreach eater $localAgents {
	    $eater eval [list updateOutputLink]
	}
    }

    ## tick the environment and update the GUI
    incr worldCount
    tickSimulation $w
    update

    ## send new input to agent
    if [info exists localAgents] {
	if { $tsiConfig(sioDebug) > 4 } { puts "++ Soar Tick I/O..." }
	set an_agent [lindex $localAgents 0] 
        foreach eater $localAgents {
	    $eater eval [list updateInputLink]
	}
    }
    
    $_kernel CheckForIncomingCommands

}


##This is the routine that works like Eaters for Soar 8.5, that is,
##prior to changes for SML Soar.  Works with Tcl and TSI.
##Not used for SML
proc TSIrunSimulation {w} {
   global runningSimulation tickDelay soarTimePerTick soarTimeUnit \
          worldCount worldCountLimit tsiConfig
   set agents [interp slaves]

    set agent0 [lindex $agents 0]
 #   puts "runSimulation -- 0 :: agent0 = $agent0"
   if {$agents == ""} {
      tk_dialog .error Warning "There are currently no eaters to run." \
                warning 0 Ok
      set runningSimulation 0
      return
   }
   if {([.wGlobal.c find withtag normalfood] == {}) && \
       ([.wGlobal.c find withtag bonusfood] == {})} {
      set runningSimulation 0
      tk_dialog .info {Game Over} {Game over: All the food is gone.} info 0 Ok
      return
   }
   if {$worldCount >= $worldCountLimit} {
      set runningSimulation 0
       environmentStop
      tk_dialog .info {Game Over} \
                      {Game over: Move count limit reached.} info 0 Ok
      return
   }
    if [goalIsAccomplished] {
	
	set runningSimulation 0
	environmentStop
	tk_dialog .info {Goal Accomplished} \
	    {The goal has been accomplished. Game Over.} info 0 Ok
	return
    }
   incr worldCount
   tickSimulation $w
   update
   TSItickIO $agents
    
   if {[info exists runningSimulation] && $runningSimulation} {
      after $tickDelay runSimulation $w

   } else {
       #puts "Environment Stopping..."
       environmentStop
   }
}

##This is the routine that works like Eaters for Soar 8.5, that is,
##prior to changes for SML Soar.  Works with Tcl and TSI.
##Not used for SML

proc TSItickIO { agents } {
    global soarTimePerTick soarTimeUnit localAgents remoteAgents \
       sio_agentStatus tsiConfig agentUpdate _kernel

    if [info exists localAgents] {
	if { $tsiConfig(sioDebug) > 4 } { puts "++ Soar Tick I/O..." }
	set an_agent [lindex $localAgents 0] 
        foreach eater $localAgents {
	    $eater eval [list updateInputLink]
	}
	$_kernel ExecuteCommandLine "run -$soarTimeUnit $soarTimePerTick" $an_agent
	# non-SML code: [list run-soar $soarTimePerTick $soarTimeUnit]
        foreach eater $localAgents {
	    $eater eval [list updateOutputLink]
	}
        $_kernel CheckForIncomingCommands
	if { $tsiConfig(sioDebug) > 4 } { puts "++ End Soar Tick I/O" }
    }

    ### For Soar 8.6 release, Kcoulter deleted the remote
    ### agent SGIO code that used to be here, just to simplify.
}


proc displayReceivedOutputStatus {} {
    global remoteAgents sio_agentStatus

    set n_waiting 0
   
    foreach {name socket} [array get remoteAgents] {
	if { $sio_agentStatus($name,online) } {
	    if { $sio_agentStatus($name,receivedOutput) } {
#		puts "  - Received Output From Agent '$name'"
	    } else {
#		puts "  - Waiting for Output From Agent '$name'"
		incr n_waiting
	    }
	}
    }
#    puts "  - WAITING FOR OUTPUT FROM $n_waiting AGENT(S)."
}

proc receiveRemoteAgentOutput { socket } {
   global tsiConfig sio_socketToAgent sio_agentStatus agentUpdate \
       remoteAgents remoteAgentOutputLinks

  
    if { $tsiConfig(sioDebug) > 3 || $tsiConfig(sioWatch) > 1 } {
	puts "receiveRemoteAgentOutput agent: $sio_socketToAgent($socket) (begin)"
    }
    fileevent $socket readable {}

   set done 0
   while { $done == 0 } {
      
      gets $socket line
      
      if { [string compare $line "0"] != 0 } {
	 tk_messageBox -type ok -message "Expected control command but got garbage ($line)" -icon error
	 return
      }
      
      gets $socket line
      
      if { [string compare $line "tuples-begin"] == 0 } {
	 set done 1
      } else {
	 # todo: implement this!
	 #puts "** SIO_ReceiveAllChangedInput :: Got message: $line"
	 tk_messageBox -type ok -message "Cannot handle $line\nNot impl yet!" -icon error
	 SIO_HandleSocketCommand $line
      }
   }
    

   while {1} {
      
      # grab everything from the socket
      
      gets $socket idNumber
      
      if {$idNumber == 0} {
	 gets $socket line
	 if { [string compare $line "tuples-end"] == 0} {
	    break
	 } else {
	    tk_messageBox -type ok -message "Unexpected control command in tuple stream" -icon error
	    return
	 }
      }
      
      gets $socket attribute
      gets $socket valueType
      gets $socket value
      gets $socket action
      
      lappend tuples [list $idNumber $attribute $valueType $value $action]

          
   }


    # Now we remove this each decision cycle, this prevents the eaters from
    # continually moving, if only  single 'move' attribute is asserted
    # to the output-link.
    if { [info exists remoteAgentOutputLinks($sio_socketToAgent($socket))] } {
	unset remoteAgentOutputLinks($sio_socketToAgent($socket))
    }

   if [info exists tuples] {
      $tsiConfig(manageRemoteAgentOutputFN) $sio_socketToAgent($socket) $tuples
   } else {

       # if no tuples were sent, we just need to work with the old ones.
       $tsiConfig(actOnRemoteAgentOutputFN) $sio_socketToAgent($socket)
   }
    


   fileevent $socket readable [list SIO_GetSocketInput $socket]

   # puts "Setting agentStatus (receivedOutput) for $sio_socketToAgent($socket) to 1"
   set sio_agentStatus($sio_socketToAgent($socket),receivedOutput) 1

   # Check to see if all remote agents have receive output.
   set allReceivedOutput 1
   foreach {agent socket} [array get remoteAgents] {
      if { $sio_agentStatus($agent,online) && \
           !$sio_agentStatus($agent,receivedOutput) } {
	  
      #   puts "  -- Agent $agent has not sent output yet."
	 set allReceivedOutput 0
	 break
      }
   }
   
   if { $allReceivedOutput } { 
       set agentUpdate complete 
    #   puts " -- All agents have sent their output."
   }

   if { $tsiConfig(sioDebug) > 3 || $tsiConfig(sioWatch) > 1 } {
      puts "receiveRemoteAgentOutput agent: $sio_socketToAgent($socket) (end)"
   }
   
}

# I've modified this significantly. Now, 
# remoteAgentOutputLinks($agent) is deleted each decision cycle
# which means that a single (unchanged) structure on the ontput-link
# results in only a single action within the environment.
proc manageRemoteAgentOutput { agent tuples } {
   global tsiConfig remoteAgentOutputLinks
   
   if { $tsiConfig(sioDebug) > 3 } { puts "manageRemoteAgentOutput -- (begin)"}



#
#   if { [info exists remoteAgentOutputLinks($agent)] } {
#      
#      foreach tuple $tuples {
#
#	  if { $tsiConfig(sioWatch) > 5 } {
#	      puts " $agent Received: $tuple"
#	  }
#	 switch -exact [lindex $tuple 4] {
#	    1 {
#	       lappend remoteAgentOutputLinks($agent) [lrange $tuple 0 3]
#	    }
#	    2 {
#	       set remoteAgentOutputLinks($agent) \
#		  [ldelete $remoteAgentOutputLinks($agent) [lrange $tuple 0 3]]
#	    }
#	    3 { 
#	       error "Change not impl!"
#	    }
#	 }
#      }
#   } else {
      
      # We'll clean this up later so that the outputLink is already created.
      foreach tuple $tuples {
	  if { $tsiConfig(sioWatch) > 5 } {
	      puts " $agent Received: $tuple"
	  }

	 switch -exact [lindex $tuple 4] {
	    1 {
	       lappend remoteAgentOutputLinks($agent) [lrange $tuple 0 3]
	    }
	    3 {
	       error "Don't know what to do with a change $tuple...nothing to change."
	    }
	 }
      }
    #}
# Removed from switch:
#	    2 {
#	       error "Can't evaluate the Remove ($tuple)...nothing to remove"
#	    }


    $tsiConfig(actOnRemoteAgentOutputFN) $agent
}



proc actOnRemoteAgentOutput { agent } {
    global remoteAgentOutputLinks tsiConfig

    # This needs to be cleaned up so that this is guarenteed to exist 
    # at all times after agent creation....
    
    if { [info exists remoteAgentOutputLinks($agent)] } {

	if { $tsiConfig(sioWatch) > 2 } {
	    puts "AGENT $agent has the OUTPUTLINK:"
	}
	foreach tuple $remoteAgentOutputLinks($agent) {
	    
	    if { $tsiConfig(sioWatch) > 2 } {  
		puts "  $tuple"
	    }
	    switch -exact [lindex $tuple 1] {
		move {
		    set actionTuple $tuple
		    set action move
		    #setWalkDir $agent [lindex $tuple 3]
		}
		jump {
		    set actionTuple $tuple
		    set action jump
		    #setJumpDir $agent [lindex $tuple 3]
		}
	    }
	}
	if [info exists actionTuple] {

	    foreach tuple $remoteAgentOutputLinks($agent) {
		
		if { [lindex $tuple 0] == [lindex $actionTuple 3] && \
			 [lindex $tuple 1] == "direction" } {
		   # puts "DIRECTION IS: [lindex $tuple 3]"
		    
		    if { $action == "move" } {
			setWalkDir $agent [lindex $tuple 3] [lindex $tuple 0]
		    } else {
			setJumpDir $agent [lindex $tuple 3] [lindex $tuple 0]
		    }
		}
	    }
	}
	    

	if { $tsiConfig(sioWatch) > 2 } {
	    puts "END OF OUTPUTLINK"
	}
    } elseif { $tsiConfig(sioWatch) > 2 } {
    
	puts "REMOTE AGENT OUTPUT LINK NOT CREATED YET.." 
    }
}
   

proc loadController {agent color} {
   [eaterName $agent $color] eval {tsiDisplayAndSendCommand {excise -all}}
   if [catch "[eaterName $agent $color] eval \
       [list tsiLoadAgentSource $agent]" msg] {
      tk_dialog .error Warning \
                "Couldn't load control code for $agent: $msg" error 0 Ok
   }
}

proc logicalX {w obj} {
   global gridSize
   return [expr int([lindex [$w coords $obj] 0] / $gridSize)]
}

proc logicalY {w obj} {
   global gridSize
   return [expr int([lindex [$w coords $obj] 1] / $gridSize)]
}

proc convertToLogical {n} {
   global gridSize
   return [expr int($n/$gridSize)]
}

proc setWalkDir {agent dir id} {
   global moveDir eaterOutputFile actionID
   set moveDir($agent) $dir
   set actionID($agent) $id
   
 
    if [info exists eaterOutputFile($agent)] {
	    puts "  --LOG FILE EXISTS"
	puts $eaterOutputFile($agent) "move: $dir"
    }

}

proc setJumpDir {agent dir id} {
   global jumpDir eaterOutputFile actionID
   set jumpDir($agent) $dir
    set actionID($agent) $id

    if [info exists eaterOutputFile($agent)] {
	puts $eaterOutputFile($agent) "jump: $dir"
    }
}


proc loadEaterNames {button} {
   global currentName

   set allNames [glob -nocomplain {*.soar}]
   set names ""
   foreach i $allNames {
      lappend names [file rootname [file tail $i]]
   }

   if {$names == {}} {
      tk_dialog .error Error "There is no agent code in this directory" \
                error 0 Ok
      if [winfo exists $button] {
         [$button cget -menu] delete 0 end
         [$button cget -menu] add command -label {*NONE*} -command {}
      } else {
         tk_optionMenu $button {*NONE*}
         [$button cget -menu] entryconfigure {*NONE*} -command {}
      }
      $button configure -state disabled
      set currentName {}
   } else {
      if [winfo exists $button] {
         [$button cget -menu] delete 0 end
         foreach i $names {
            [$button cget -menu] add command -label $i \
                           -command [list set currentName $i]
         }
         if {![info exists currentName] ||
             ([lsearch $names $currentName] < 0)} {
            set currentName [lindex $names 0]
         }
      } else {
         eval tk_optionMenu .agents.create.e currentName $names
      }
   }
}
