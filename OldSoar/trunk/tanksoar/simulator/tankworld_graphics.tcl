proc drawstat {whichAgent} {
	global wglobal_y1 wglobal_y2 wglobal_y3 wglobal_y4 wglobal_infowidth tankList \
		tank maxHealth maxEnergy wglobal_onestat_h realWidth \
		drawstat_x1 drawstat_x4 drawstat_x4_x1 gameType tsiConfig
	
	set tank_num [lsearch $tankList $whichAgent]
	if {$tank_num == -1} {
		Debug "$whichAgent not found."
		return
	}
	.wGlobal.infoc.status delete stats$whichAgent
	
	set x2 [expr $drawstat_x1 + ($drawstat_x4_x1 * (double($tank($whichAgent,health))/double($maxHealth)))]
	set x3 [expr $drawstat_x1 + ($drawstat_x4_x1 * (double($tank($whichAgent,energy))/double($maxEnergy)))]
	set y1 [expr ($tank_num * $wglobal_onestat_h) + $wglobal_y1]
	set y2 [expr ($tank_num * $wglobal_onestat_h) + $wglobal_y2]
	set y3 [expr ($tank_num * $wglobal_onestat_h) + $wglobal_y3]
	set y4 [expr ($tank_num * $wglobal_onestat_h) + $wglobal_y4]
	
	.wGlobal.infoc.status create rectangle \
		$drawstat_x1 $y1 \
		$drawstat_x4 $y3 \
		-outline $whichAgent -tags stats$whichAgent -width 3
	if {$x2 != $drawstat_x1} {
		.wGlobal.infoc.status create rectangle \
			[expr $drawstat_x1 + 3] [expr $y1 + 3]\
			[expr $x2 - 3] [expr $y2 + 1] \
			-fill tomato -width 0 -tags stats$whichAgent
	}
	if {$x3 != $drawstat_x1} {
		.wGlobal.infoc.status create rectangle \
			[expr $drawstat_x1 + 3] $y2 \
			[expr $x3 - 3] [expr $y3 - 3] \
			-fill steelblue -width 0 -tags stats$whichAgent
	}
	if {$gameType == "deathmatch"} {
		.wGlobal.infoc.status create text \
			$drawstat_x1 $y4 -anchor sw -tags stats$whichAgent \
			-text "Points $tank($whichAgent,score)|Missiles $tank($whichAgent,missiles)" 
	#		-text "P $tank($whichAgent,score)|L $tank($whichAgent,lives)|Ms $tank($whichAgent,missiles)" 
	} else {
		.wGlobal.infoc.status create text \
			$drawstat_x1 $y4 -anchor sw -tags stats$whichAgent \
			-text "P $tank($whichAgent,score)|Ms $tank($whichAgent,missiles)|Mi $tank($whichAgent,mines)" 
	#		-text "P $tank($whichAgent,score)|L $tank($whichAgent,lives)|Ms $tank($whichAgent,missiles)|Mi $tank($whichAgent,mines)" 
	}
	.wGlobal.infoc.status create line 0 [expr ($tank_num + 1) * $wglobal_onestat_h] \
		$realWidth [expr ($tank_num + 1) * $wglobal_onestat_h]
}

proc drawbar {bar_canvas bar_text bar_y \
	barlabel_tag bar_tag bar_setting bar_max {bar_color skyblue} {bar_info ""}} {
	global wtag_sensor_width buttonBackgroundColor
	
	$bar_canvas create text [expr $wtag_sensor_width / 2] $bar_y -text $bar_text \
		-tag $barlabel_tag -justify center

	$bar_canvas create rectangle 5 [expr $bar_y + 7] \
		[expr $wtag_sensor_width - 4] [expr $bar_y + 27] -width 3 -outline $buttonBackgroundColor -tag $bar_tag

	$bar_canvas create rectangle 8 [expr $bar_y + 10] \
		[expr 8 + ($wtag_sensor_width - 15) * (double($bar_setting)/double($bar_max))] \
		[expr $bar_y + 24] -fill $bar_color -outline $bar_color -tag $bar_tag

	if {$bar_info == ""} {
		set bar_info $bar_setting
	}
	
	$bar_canvas create text [expr $wtag_sensor_width / 2] [expr $bar_y + 18] \
		-text $bar_info -justify center -tag $bar_tag
		
}

proc makeFillColor {bool} {
global buttonBackgroundColor

	if {($bool == 1) || ($bool == "yes")} {
		return $buttonBackgroundColor
	} else {
		return white
	}
}

proc drawsensor {sensor_canvas sensor_text sensor_tag whichAgent tag_color tag_dir \
	upbox rightbox downbox leftbox} {
	global draw_sense_text draw_sense_x1 draw_sense_x2 draw_sense_x3 draw_sense_x4 \
			draw_sense_y1 draw_sense_y2 draw_sense_y3 draw_sense_y4 tsiConfig
	
		if { $tsiConfig(autorun) != 0 } {
				return 
		}


	$sensor_canvas create text $draw_sense_text 9 -text $sensor_text \
		-justify center -tags $sensor_tag
		
	$sensor_canvas create rectangle $draw_sense_x2 $draw_sense_y1 $draw_sense_x3 $draw_sense_y2 \
		-outline black -fill [makeFillColor $upbox] -tags $sensor_tag
	$sensor_canvas create rectangle $draw_sense_x3 $draw_sense_y2 $draw_sense_x4 $draw_sense_y3 \
		-outline black -fill [makeFillColor $rightbox] -tags $sensor_tag
	$sensor_canvas create rectangle $draw_sense_x2 $draw_sense_y3 $draw_sense_x3 $draw_sense_y4 \
		-outline black -fill [makeFillColor $downbox] -tags $sensor_tag
	$sensor_canvas create rectangle $draw_sense_x1 $draw_sense_y2 $draw_sense_x2 $draw_sense_y3 \
		-outline black -fill [makeFillColor $leftbox] -tags $sensor_tag
	
	$sensor_canvas create rectangle $draw_sense_x2 $draw_sense_y2 $draw_sense_x3 $draw_sense_y3  -tags $sensor_tag \
		-outline black -width 3 -fill $tag_color
	
}

proc drawscore {} {
    global tank tankList 
    
    .wGlobal.infoc.status delete all

    set tanks_done 0
    foreach curr_tank $tankList {
    	incr tanks_done
    	if {$tank($curr_tank,status) != "inactive"} {
			if {$tank($curr_tank,status) == "active"} {
				drawstat $curr_tank
			}
    	}
	}
}

proc drawRadarSensor {whichAgent} {
    global map tank gridSize mapdim current radar_box i_tank_small current tsiConfig

		if { $tsiConfig(autorun) != 0 } {
				return 
		}


    if {($whichAgent != $current) || 
    	($tank($whichAgent,radarOn) == 0) || 
    	($tank($whichAgent,radarSetting) == 0)} {
    	return
    }

    foreach seenItem $tank($whichAgent,radarInfo) {
    	eval [subst "drawSmallSpot $whichAgent $seenItem"]
    }

    # Print the tank and radar beam in the radar info
	.wAgentInfo.radar create rectangle \
		30  [expr ($mapdim - 2) * 22 + 28] \
		50  [expr ($mapdim - 1) * 22 + 26] \
		-width 2 -tags $whichAgent
		
}

proc drawSmallFlag {x y ownerColor occupierColor taglist} {
global gridSize powerUpList
	set x1 [expr $x + 6]
	set x2 [expr $x + 16]
	set y1 [expr $y + 3]
	set y2 [expr $y + 7]
	set y3 [expr $y + 10]
	set y4 [expr $y + 15]
	
	.wAgentInfo.radar create line $x1 $y1 $x1 $y4 -fill black -tags $taglist
	.wAgentInfo.radar create polygon $x1 $y1 $x2 $y2 $x1 $y2 $x1 $y1 -fill $ownerColor -tags $taglist
	.wAgentInfo.radar create polygon $x1 $y2 $x2 $y2 $x1 $y3 $x1 $y2 -fill $occupierColor -tags $taglist

}

proc drawSmallSpot {whichAgent filler dist side {color1 none} {color2 none}} {
    global map tank mapdim gridSize current gameType \
	    radar_box i_obstruct_small i_ground_small i_tank_small \
	    i_battery_small i_missile_small i_mine_small i_health_small i_buried_mine_small

    if {$side == "left"} {
		set pos 0
    } elseif {$side == "center"} {
		set pos 1
    } elseif {$side == "right"} {
		set pos 2
    }

	set x [expr $pos * $radar_box + ($radar_box / 4) + 3]
	set y [expr ($mapdim - $dist - 2) * $radar_box + 19 - 4]
	
	switch -exact -- $filler {
   		wall {
			set whichImage $i_obstruct_small
		}
		health {
			set whichImage $i_health_small
		}
		energy {
			set whichImage $i_battery_small
		}
		missiles {
			set whichImage $i_missile_small
		}
		mines {
			set whichImage $i_mine_small
		}
		open {
			set whichImage $i_ground_small
		}
		buriedMine {
			set whichImage $i_buried_mine_small
		}
		flag {
			if {$gameType == "flag"} {
				drawSmallFlag $x $y $color1 $color2 [list radar$whichAgent smallRadar]
			}
			 return
		}
	}
	if {![info exists whichImage]} {
		set whichImage $i_tank_small
	}
	
	.wAgentInfo.radar create image $x $y \
		 -image $whichImage -tags [list radar$whichAgent smallRadar] -anchor nw
	
	if {$whichImage == $i_tank_small} {
		set x3 [expr $x + $radar_box - 5]
		set x2 [expr $x + ($x3 - $x)/2]
		set y1 [expr $y + 3]
		set y3 [expr $y + $radar_box - 6]
		set y2 [expr $y + ($y3 - $y)/2]
		.wAgentInfo.radar create line $x $y2 $x2 $y1 $x3 $y2 $x2 $y3 $x $y2 \
			-fill $filler -tags [list radar$whichAgent smallRadar]
	}
}

proc drawOneRadar {w x1 y1 x2 y2 horiz change agentTag} {
global gridSize
	if {$horiz == 1} {
		set radar1 [expr $gridSize * $y1]
		set radar3 [expr $gridSize * ($y1 + 1)]
		if {$change == -1} { incr x1 1; incr x2 1 }
		set thisWave [expr $gridSize * $x1]
		set limitWave [expr $gridSize * $x2]
	} else {
		set radar1 [expr $gridSize * $x1]
		set radar3 [expr $gridSize * ($x1 + 1)]
		if {$change == -1} { incr y1 1; incr y2 1 }
		set thisWave [expr $gridSize * $y1]
		set limitWave [expr $gridSize * $y2]
	}
	set radar2 [expr $radar1 + (($radar3 - $radar1)/2)]

	while { [expr $change * $thisWave] < [expr $change * $limitWave] } {
		if {$horiz == 1} {
			$w create line $thisWave $radar1 \
							[expr $thisWave + ($change * 20)] $radar2 \
							$thisWave $radar3 -tags [list radar$agentTag radar $agentTag] -smooth true \
							-fill white -width 1
		} else {
			$w create line $radar1 $thisWave \
							$radar2 [expr $thisWave + ($change * 20)] \
							$radar3 $thisWave -tags [list radar$agentTag radar $agentTag] -smooth true \
							-fill white -width 1
		}
		incr thisWave [expr $change * 10]

	}
}

proc drawRadarSensorWaves {whichAgent} {
global map mapdim tank tsiConfig

		if { $tsiConfig(autorun) != 0 } { 
				return
		}
	
	if {$tank($whichAgent,radarOn) == 0} {
		return
	}
	
	switch -exact -- $tank($whichAgent,dir) {
		east { set k [expr $tank($whichAgent,x) + 1]; set change 1; set horiz 1 }
		west { set k [expr $tank($whichAgent,x) - 1]; set change -1; set horiz 1 }
		north { set k [expr $tank($whichAgent,y) - 1]; set change -1; set horiz 0 }
		south { set k [expr $tank($whichAgent,y) + 1]; set change 1; set horiz 0 }
	}
	
	set j $k
	if {$j == 0} {
		return
	}
	
	set done 0
	while {($done == 0) && ($j < $mapdim) && ([expr abs($j - $k)] < $tank($whichAgent,radarSetting)) && ($j > 0)} {
		if {$horiz == 1} { set filler $map($j,$tank($whichAgent,y)) } else { set filler $map($tank($whichAgent,x),$j) }
		if {$filler == 0} { incr j $change } else { set done 1 }
	}
	
	if {$horiz == 1} { 
		drawOneRadar .wGlobal.map $k $tank($whichAgent,y) $j $tank($whichAgent,y) $horiz $change $whichAgent
	} else {
		drawOneRadar .wGlobal.map $tank($whichAgent,x) $k $tank($whichAgent,x) $j $horiz $change $whichAgent
	}
}

proc drawTank {whichAgent} {
	global tank  map gridSize powerUpList gameType
	
	if {$tank($whichAgent,status) == "dead" } {
		return
	}
	
	set tankTag [.wGlobal.map find withtag $whichAgent]
	set radarTag [.wGlobal.map find withtag radar$whichAgent]
	set shieldTag [.wGlobal.map find withtag shields$whichAgent]
	if {$tankTag != ""} {
		eval [concat .wGlobal.map delete $tankTag]
		eval [concat .wGlobal.map delete $radarTag]
		eval [concat .wGlobal.map delete $shieldTag]
	}
	
	set x1 [expr $tank($whichAgent,x) * $gridSize]
	set x3 [expr (1 + $tank($whichAgent,x)) * $gridSize]
	set x2 [expr $x1 + ($x3 - $x1)/2]
	set y1 [expr $tank($whichAgent,y) * $gridSize]
	set y3 [expr (1 + $tank($whichAgent,y)) * $gridSize]
	set y2 [expr $y1 + ($y3 - $y1)/2]
	
	draw_mapsector $tank($whichAgent,x) $tank($whichAgent,y) $whichAgent
	drawRadarSensorWaves $whichAgent
	.wGlobal.map create line $x1 $y2 $x2 $y1 $x3 $y2 $x2 $y3 $x1 $y2 \
		-fill $whichAgent -width 2 -tags [list $whichAgent]

	if {($gameType == "flag") && \
		[info exists powerUpList($tank($whichAgent,x),$tank($whichAgent,y))] && \
		($powerUpList($tank($whichAgent,x),$tank($whichAgent,y)) == "flag")} {
		drawFlag $tank($whichAgent,x) $tank($whichAgent,y)
	}
	
	#>-=>
	# Draw the Shields
	#>-=>
	if {$tank($whichAgent,shields) == "on"} {
		set x1 [expr $x1 - 4]
		set x3 [expr $x3 + 6]
		set y1 [expr $y1 - 4]
		set y3 [expr $y3 + 6]
		.wGlobal.map create oval $x1 $y1 $x3 $y3 -outline $whichAgent -width 3 -tags [list $whichAgent shields$whichAgent]
		.wGlobal.map create oval $x1 $y1 $x3 $y3 -outline white -width 2 -tags [list $whichAgent shields$whichAgent]
		.wGlobal.map create oval $x1 $y1 $x3 $y3 -outline $whichAgent -width 1 -tags [list $whichAgent shields$whichAgent]
	}
}

proc eraseRadar {whichAgent} {

    .wGlobal.map delete radar$whichAgent 
    .wAgentInfo.radar delete radar$whichAgent
    set tank($whichAgent,radarInfo) [list]
}

proc eraseRadarSensor {whichAgent} {

    .wAgentInfo.radar delete radar$whichAgent
}

proc eraseTank {whichAgent} {
	eraseRadar $whichAgent
    .wGlobal.map delete $whichAgent
}

proc refreshRadar {whichAgent} {
	eraseRadar $whichAgent
	drawRadarSensor $whichAgent
	drawRadarSensorWaves $whichAgent
}

proc eraseAgentInfo {} {

    .wAgentInfo.statsensors.sense.c1.infrared delete all
    .wAgentInfo.statsensors.sense.c1.radar delete all
    .wAgentInfo.statsensors.sense.c2.sound delete all
    .wAgentInfo.statsensors.sense.c3.incoming delete all
    .wAgentInfo.statsensors.sense.smell delete all
    .wAgentInfo.statsensors.hstat delete all
    .wAgentInfo.statsensors.bstat delete all
    .wAgentInfo.statsensors.lstat delete all
    
}

proc drawAgentInfo {whichAgent} {
    global tank maxHealth maxRadar maxEnergy maxsmell \
    	gridSize  tsiConfig

		if { $tsiConfig(autorun) != 0 } {
				return 
		}

    eraseAgentInfo
    
    set color $whichAgent
    set dir $tank($whichAgent,dir)

    if {$tank($whichAgent,nearest) != ""} {
		drawbar .wAgentInfo.statsensors.sense.smell "Smell" \
			8 smelllabel smellbar $tank($whichAgent,smell) $maxsmell $tank($whichAgent,nearest) \
			"$tank($whichAgent,nearest) $tank($whichAgent,smell) away"
		eval drawsensor .wAgentInfo.statsensors.sense.c2.sound "Sound" soundsensors $whichAgent $color $dir \
				[dir-2-args $tank($whichAgent,sound)]
	} else {
		drawbar .wAgentInfo.statsensors.sense.smell "Smell" \
			8 smelllabel smellbar 0 $maxsmell
		drawsensor .wAgentInfo.statsensors.sense.c2.sound "Sound" soundsensors $whichAgent \
			$color $dir 0 0 0 0
    }
    
	drawsensor .wAgentInfo.statsensors.sense.c1.radar "RWaves" radarsensors $whichAgent $color $dir \
		$tank($whichAgent,RWavesforward) $tank($whichAgent,RWavesright) $tank($whichAgent,RWavesbackward) $tank($whichAgent,RWavesleft)
	drawsensor .wAgentInfo.statsensors.sense.c1.infrared "Blocked" irsensors $whichAgent $color $dir \
		$tank($whichAgent,Blockedforward) $tank($whichAgent,Blockedright) $tank($whichAgent,Blockedbackward) $tank($whichAgent,Blockedleft)
	drawsensor .wAgentInfo.statsensors.sense.c3.incoming "Incomng" incomingsensors $whichAgent $color $dir \
		$tank($whichAgent,Incomingforward) $tank($whichAgent,Incomingright) $tank($whichAgent,Incomingbackward) $tank($whichAgent,Incomingleft)

	drawbar .wAgentInfo.statsensors.hstat "Health" 8 healthlabel healthbar $tank($whichAgent,health) $maxHealth
    drawbar .wAgentInfo.statsensors.bstat "Radar Setting" \
    	8 radarlabel radarbar $tank($whichAgent,radarSetting) $maxRadar
    drawbar .wAgentInfo.statsensors.lstat "Energy" \
    	8 energylabel energybar $tank($whichAgent,energy) $maxEnergy

    .wControls.bottomframe.radar set $tank($whichAgent,radarSetting)
    refreshRadar $whichAgent
    
}

proc drawmap {} {
  global map mapdim tsiConfig
 
		if { $tsiConfig(autorun) != 0 } {
				return 
		}

  set i 0
  while {$i < $mapdim} {
    set j 0
    while {$j < $mapdim} {
        draw_mapsector $i $j
    incr j 1
    }
  incr i 1
  }
}

proc drawFlag {x y} {
global gridSize powerUpList

	set x1 [expr ($x*$gridSize) + 5]
	set x2 [expr ($x*$gridSize) + 27]
	set y1 [expr ($y*$gridSize) + 2]
	set y2 [expr ($y*$gridSize) + 11]
	set y3 [expr ($y*$gridSize) + 20]
	set y4 [expr ($y*$gridSize) + 30]
	
	.wGlobal.map create line [expr $x1 -2] $y1 [expr $x1 -2] $y4 -fill black -width 3
	.wGlobal.map create polygon $x1 $y1 $x2 $y2 $x1 $y2 $x1 $y1 -fill $powerUpList($x,$y,owner)
	.wGlobal.map create polygon $x1 $y2 $x2 $y2 $x1 $y3 $x1 $y2 -fill $powerUpList($x,$y,sender)
}


proc draw_actualSector {x y type tag hasFlag} {
	global map mapdim gridSize i_ground1 i_ground2 i_ground3 \
			i_oasisb i_oasish i_rechargeh i_rechargeb \
			t1_right t1_left t1_down t1_up i_explosion \
			i_obstruct1 i_obstruct2 i_obstruct3 i_obstruct4 \
			i_obstruct5 i_obstruct6 i_obstruct7 i_obstruct8 \
			i_obstruct9 i_obstruct10 \
			i_mountain1 i_mountain2 i_mountain3 \
			i_battery i_health i_missile i_mine i_buried_mine gameType
			
	switch -exact -- $type {
		0 { set randNum [random 3]
			set graphic i_ground$randNum
		}
		1 { set randNum [random 6]
			set graphic i_obstruct$randNum
			if {($x == 0) || ($y == 0) || ($x == [expr $mapdim -1]) || ($y == [expr $mapdim -1])} {
				set randNum [random 3]
				set graphic i_mountain$randNum
			}
		}
		2 { set graphic i_oasish }
		3 { set graphic i_oasisb }
		4 { set graphic i_rechargeh }
		5 { set graphic i_rechargeb }
		6 { set graphic i_explosion }
		7 { set graphic i_battery }
		8 { set graphic i_health }
		9 { set graphic i_missile }
		10 { set graphic i_mine }
		11 { set graphic i_buried_mine }

		north { set graphic t1_up }
		east { set graphic t1_right }
		south { set graphic t1_down }
		west  { set graphic t1_left }
		default { Debug "Invalid type $type passed to draw_mapsector" }
	}
	
	.wGlobal.map create image [expr $gridSize * $x] [expr $gridSize * $y] \
			-image [subst $$graphic] -anchor nw -tag $tag
	if {$hasFlag == "yes" && ($gameType == "flag")} {
		drawFlag $x $y
	}
}
	
proc draw_mapsector {x y {tag none}} {
	global map tank powerUpList
	
	set contents $map($x,$y)
	set type $contents
	set hasFlag "no"
	
	if {$contents == 1} { 
		set tag obstruction
	}

	if {($contents != 0) && ($contents != 1)} {
		# Must be a tank.  Check if it is recharging
		if {[onHealth $x $y]} {
			set type 4
		} elseif {[onEnergy $x $y]} {
			set type 5
		} else { 
			set type $tank($contents,dir)
		}
		if {$tank($contents,exploding) > 0} {
			set type 6
			if {$tank($contents,exploding)==3} {
				set tank($contents,exploding) 0
			} else {
				incr tank($contents,exploding) 1
			}
		}
	} else {
		if {[info exists powerUpList($x,$y)]} {
			switch $powerUpList($x,$y) {
				health {
					set type 8
				}
				energy {
					set type 7
				}
				missiles {
					set type 9
				}
				mines {
					set type 10
				}
				buriedMine {
					set type 11
				}
				flag {
					set hasFlag "yes"
				}
			}
		}
	}
	draw_actualSector $x $y $type $tag $hasFlag
}
