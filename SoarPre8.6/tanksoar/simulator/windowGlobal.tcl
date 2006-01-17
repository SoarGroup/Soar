proc envLoadMap {mapPath mapFile} {
global tankList mapheight mapdim tank originalMap
  
   if [catch {cd $mapPath} msg] {
	   error "Could not change directory to '$mapPath': $msg"
	   return
    }
   
   .wGlobal.map delete map
   foreach t $tankList {
	   if {$tank($t,status) == "active"} {
		   uplevel #0 destroyAgent $t
		}
   }
   uplevel #0 "source $mapFile.map"
   uplevel #0 {set map(projectiles) [list]}
   uplevel #0 {array set originalMap [array get map]}
   uplevel #0 setInitialPowerUps
   uplevel #0 drawmap
   uplevel #0 {set gridSize [expr $mapheight/$mapdim]}

}

proc randomMap {} {
    global tank map mapdim gridSize healthcharge energycharge tankList mapheight originalMap
#   added tank to list above to fix bugzilla bug 188

    set mapnames [glob maps/*.map]
    set mapcount [llength $mapnames]
    set mapfile [lindex $mapnames [expr [random $mapcount] - 1]]
    uplevel #0 source $mapfile

    .wGlobal.map delete map

    foreach t $tankList {
		if {$tank($t,status) == "active"} {
		    uplevel #0 destroyAgent $t
		}
    }
    
   uplevel #0 {set gridSize [expr $mapheight/$mapdim]}
    #set maxsmell [expr (2 * $mapdim) - 6]

   uplevel #0 {set map(projectiles) [list]}
   uplevel #0 {array set originalMap [array get map]}
   uplevel #0 setInitialPowerUps
   uplevel #0 drawmap
}

canvas .wGlobal.map -width $mapwidth -height $mapheight -relief sunken -background $backgroundColor

canvas .wGlobal.infoc -height $mapheight -width $wglobal_infowidth -relief raised -background $backgroundColor

canvas .wGlobal.infoc.status -height $wglobal_statusheight \
	-relief raised -background $backgroundColor

 canvas .wGlobal.infoc.bottom \
	-relief raised  -background lightSteelBlue  -width $wglobal_infowidth
  button .wGlobal.infoc.bottom.step -text Step -width $wglobal_infowidth -command environmentStep
  button .wGlobal.infoc.bottom.run -text Run -width $wglobal_infowidth -command environmentRun
#  button .wGlobal.infoc.bottom.stop -text Stop -width $wglobal_infowidth -command fake-stop-soar
  button .wGlobal.infoc.bottom.stop -text Stop -width $wglobal_infowidth -command environmentStop


pack .wGlobal.map -side right 
pack .wGlobal.infoc -side left


pack .wGlobal.infoc.bottom.step .wGlobal.infoc.bottom.run .wGlobal.infoc.bottom.stop -side top -fill x
pack .wGlobal.infoc.bottom -side top -fill y
pack .wGlobal.infoc.status -side top

setInitialPowerUps
drawmap

bind .wGlobal.map <ButtonPress-1> {selectTank %x %y}

set key_initialx -1
set key_initialy -1

bind .wGlobal.map <Shift-ButtonPress-1> {setKeyInits %x %y}
bind .wGlobal.map <Control-ButtonPress-1> {setKeyInits %x %y}
bind .wGlobal.map <Shift-ButtonRelease-1> {createAgentPress soar %x %y}
bind .wGlobal.map <Control-ButtonRelease-1> {createAgentPress human %x %y}
bind .wGlobal.map <Alt-ButtonPress-1> {toggleMap %x %y}
bind .wGlobal <Control-s> {saveMap}

proc setKeyInits {x y} {
global key_initialx key_initialy
	set key_initialx $x
	set key_initialy $y
}

proc createAgentPress {type x y} {
global key_initialx key_initialy tsiCurrentAgentSourceDir tsiCurrentAgentSourceFile currentAgentColor
	
	# Determine dominant motion axis
	if {[expr abs($key_initialy - $y)] > [expr abs($key_initialx - $x)]} {
		set key_initialx -1
		set east_motion 0
		set west_motion 0
	} else {
		set key_initialy -1
		set south_motion 0
		set north_motion 0
	}
	
	# Set initial direction
	if {$key_initialy != -1} {
		if {$y < $key_initialy} {
			set north_motion 1
			set south_motion 0
		} else {
			set north_motion 0
			set south_motion 1
		}
	}
	if {$key_initialx != -1} {
		if {$x < $key_initialx} {
			set west_motion 1
			set east_motion 0
		} else {
			set west_motion 0
			set east_motion 1
		}
	}

	set dir [args-2-dir $north_motion $south_motion $east_motion $west_motion]

	if {$type == "soar"} {
		createSoarTank $x $y $tsiCurrentAgentSourceDir $tsiCurrentAgentSourceFile $currentAgentColor $dir
	} else {
		createHumanTank $x $y $currentAgentColor $dir
	}
}

proc toggleMap {x y} {
global map originalMap gridSize

	set xpos [expr round($x/$gridSize)]
	set ypos [expr round($y/$gridSize)]
	
	if {$map($xpos,$ypos) == 1} {
		set map($xpos,$ypos) 0
		set originalMap($xpos,$ypos) 0
	} else {
		set map($xpos,$ypos) 1
		set originalMap($xpos,$ypos) 1
	}

	draw_mapsector $xpos $ypos
}

proc saveMap {} {
global mapdim map healthcharge energycharge

    set f [open "maps/newMap.map" w]
    puts $f "global mapdim map"
    puts $f "set mapdim $mapdim"
    for {set i 0} {$i < $mapdim} {incr i} {
		for {set j 0} {$j < $mapdim} {incr j} {
		    puts $f "set map($i,$j) $map($i,$j)"
		}
    }
    close $f
	tk_dialog .info {Save Map} \
		 {Map has been successfully saved as maps/newMap.map} info 0 Ok

}