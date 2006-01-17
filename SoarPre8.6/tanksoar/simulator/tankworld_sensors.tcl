proc makeRelative {sensorDir tankDir} {
	global opposite leftRelative cardinalToRelative
	
	switch -exact -- $tankDir {
	
		west { return $leftRelative($cardinalToRelative($sensorDir)) }
		east { return $opposite($leftRelative($cardinalToRelative($sensorDir))) }
		south { return $opposite($cardinalToRelative($sensorDir)) }
		north	{ return $cardinalToRelative($sensorDir) }
	}
}

proc initSensor {whichAgent sensorValue soarObjectName sensorField link} {
	#>-=>
	# This function adds a wme with attribute name ^soarObjectName and value sensorValue
	# - It adds this information to the inputlink specified for a particular
	#	agent by its link attribute in the tank array
	#	- for nested augmentations the link will be the parent soar object
	# - It then adds entries to the sensor array, recording the value added and
	#	the wme value returned by add-wme
	#>-=>
	global tank sensor wmeRecord

	scan [$whichAgent eval "add-wme $tank($whichAgent,$link) $soarObjectName $sensorValue"] "%d" wmeadded
	set sensor($whichAgent,$sensorField) $sensorValue
	set wmeRecord($whichAgent,$sensorField) $wmeadded
	#set sensor($whichAgent,$sensorWmeField) $wmeadded
}

proc checkSensor {whichAgent sensorValue soarObjectName sensorField link} {
	#>-=>
	# The sensor array keeps track of the last io values set in soar
	# - This function minimizes changing of soar wmes by changing
	#	them only when they are different than what was there before
	#>-=>
	global sensor tank wmeRecord
	
	if {$sensor($whichAgent,$sensorField) != $sensorValue} {
		$whichAgent eval "remove-wme $wmeRecord($whichAgent,$sensorField)"
		scan [$whichAgent eval "add-wme $tank($whichAgent,$link) $soarObjectName $sensorValue"] "%d" wmeadded
		set sensor($whichAgent,$sensorField) $sensorValue
		set wmeRecord($whichAgent,$sensorField) $wmeadded
	}
}

proc checkSensor2 {whichAgent sensorValue soarObjectName sensorField link} {
	#>-=>
	# The sensor array keeps track of the last io values set in soar
	# - This function minimizes changing of soar wmes by changing
	#	them only when they are different than what was there before
	# - if a movement was taken in last move, it changes wme anyway
	#>-=>
	global sensor tank wmeRecord actionTaken
	
	if {($sensor($whichAgent,$sensorField) != $sensorValue) || ($actionTaken($whichAgent) == 1)} {
		$whichAgent eval "remove-wme $wmeRecord($whichAgent,$sensorField)"
		scan [$whichAgent eval "add-wme $tank($whichAgent,$link) $soarObjectName $sensorValue"] "%d" wmeadded
		set sensor($whichAgent,$sensorField) $sensorValue
		set wmeRecord($whichAgent,$sensorField) $wmeadded
	}
}

proc replaceSensor {whichAgent sensorValue soarObjectName sensorField link} {
	#>-=>
	# This function replaces a wme w/o checking whether the sensor
	# array has changed
	#>-=>
	global sensor tank wmeRecord
	
	$whichAgent eval "remove-wme $wmeRecord($whichAgent,$sensorField)"
	scan [$whichAgent eval "add-wme $tank($whichAgent,$link) $soarObjectName $sensorValue"] "%d" wmeadded
	set wmeRecord($whichAgent,$sensorField) $wmeadded
}

proc setUpInputLink {whichAgent io_id inID outID} {
	global tank tankList sensor topstate_id healthcharge energycharge turn wmeRecord agentsetup2 cycle errorInfo

	if {[info exists tank($whichAgent,inputlink)]} {
  	 	Debug "Input Link already setup.  Exiting setUpInputLink"
  	 	return
	}
	
	#Debug "SetupInputLink Called."

	#>-=>
	# Set up input-link and output-link
	#>-=>

#	set wme [$whichAgent eval "add-wme $io_id ^input-link *"]
#	set tank($whichAgent,inputlink) [lindex $wme 3]	
   set tank($whichAgent,inputlink)  $inID

#	set wme [$whichAgent eval "add-wme $io_id ^output-link *"]
#	set tank($whichAgent,outputlink) [lindex $wme 3]
   set tank($whichAgent,outputlink) $outID

	#>-=>
	# Set up the hierarchy for multi-level sensor objects
	#>-=>

	set incominginput [$whichAgent eval "add-wme $tank($whichAgent,inputlink) ^incoming *"]
	set tank($whichAgent,incominglink) [lindex $incominginput 3]
	set radarInput [$whichAgent eval "add-wme $tank($whichAgent,inputlink) ^rwaves *"]
	set tank($whichAgent,rWavesLink) [lindex $radarInput 3]
	set smellinput [$whichAgent eval "add-wme $tank($whichAgent,inputlink) ^smell *"]
	set tank($whichAgent,smelllink) [lindex $smellinput 3]
	set radarinput [$whichAgent eval "add-wme $tank($whichAgent,inputlink) ^radar *"]
	set tank($whichAgent,radarLink) [lindex $radarinput 3]
	set irinput [$whichAgent eval "add-wme $tank($whichAgent,inputlink) ^blocked *"]
	set tank($whichAgent,blockedlink) [lindex $irinput 3]
	set constinput [$whichAgent eval "add-wme $tank($whichAgent,inputlink) ^constants *"]
	set tank($whichAgent,constLink) [lindex $constinput 3]
#	set wme [$whichAgent eval "add-wme $tank($whichAgent,inputlink) ^action-count *"]
#	scan $wme "%d" wmeRecord($whichAgent,actionCountWME)
	set wme [$whichAgent eval "add-wme $tank($whichAgent,inputlink) ^my-color $whichAgent"]

	if {$tank($whichAgent,nearest) != ""} {
		set smelldist $tank($whichAgent,smell)
		set smellcolor $tank($whichAgent,nearest)
		set sound $tank($whichAgent,sound)
	} else {
		set smelldist none
		set smellcolor none
		set sound silent
	}

#	initSensor $whichAgent $tank($whichAgent,lives) lives livesval inputlink
	initSensor $whichAgent $tank($whichAgent,health) health healthval inputlink
	initSensor $whichAgent $tank($whichAgent,energy) energy energyval inputlink
	initSensor $whichAgent $tank($whichAgent,missiles) missiles missilesval inputlink
#	initSensor $whichAgent $tank($whichAgent,mines) mines minesval inputlink

#	initSensor $whichAgent $turn tick tickval inputlink
	initSensor $whichAgent $tank($whichAgent,x) x xval inputlink
	initSensor $whichAgent $tank($whichAgent,y) y yval inputlink
	initSensor $whichAgent $tank($whichAgent,dir) direction dirval inputlink

	initSensor $whichAgent [yesOrNo [onHealth $tank($whichAgent,x) $tank($whichAgent,y)]] \
		healthrecharger healthrechargerval inputlink
	initSensor $whichAgent [yesOrNo [onEnergy $tank($whichAgent,x) $tank($whichAgent,y)]] \
		energyrecharger energyrechargerval inputlink

	initSensor $whichAgent $tank($whichAgent,Blockedforward) forward BlockedUpval blockedlink
	initSensor $whichAgent $tank($whichAgent,Blockedbackward) backward BlockedDownval blockedlink
	initSensor $whichAgent $tank($whichAgent,Blockedleft) left BlockedLeftval blockedlink
	initSensor $whichAgent $tank($whichAgent,Blockedright) right BlockedRightval blockedlink

	initSensor $whichAgent $tank($whichAgent,Incomingforward) forward incomingupval incominglink
	initSensor $whichAgent $tank($whichAgent,Incomingbackward) backward incomingdownval incominglink
	initSensor $whichAgent $tank($whichAgent,Incomingleft) left incomingleftval incominglink
	initSensor $whichAgent $tank($whichAgent,Incomingright) right incomingrightval incominglink

	initSensor $whichAgent $tank($whichAgent,RWavesforward) forward RWavesUpval rWavesLink
	initSensor $whichAgent $tank($whichAgent,RWavesbackward) backward RWavesDownval rWavesLink
	initSensor $whichAgent $tank($whichAgent,RWavesleft) left RWavesLeftval rWavesLink
	initSensor $whichAgent $tank($whichAgent,RWavesright) right RWavesRightval rWavesLink

	initSensor $whichAgent $smelldist distance smelldistval smelllink
	initSensor $whichAgent $smellcolor color smellcolorval smelllink

	initSensor $whichAgent $sound sound soundval inputlink

	initSensor $whichAgent $tank($whichAgent,shields) shield-status shieldval inputlink

	initSensor $whichAgent $tank($whichAgent,radardist) radar-distance radardistval inputlink
	initSensor $whichAgent $tank($whichAgent,radarSetting) radar-setting powerval inputlink
	initSensor $whichAgent [onOrOff $tank($whichAgent,radarOn)] radar-status radarOnval inputlink
	
	setupConstants $whichAgent
	radarToInputLink $whichAgent

#	initSensor $whichAgent $tank($whichAgent,lastmove) lastmove lastmove inputlink
	initSensor $whichAgent yes resurrect resurrect inputlink
	initSensor $whichAgent [expr rand()] random random inputlink
	initSensor $whichAgent $cycle clock clock inputlink
	set agentsetup2($whichAgent) done
	   
}

proc setupConstants {whichAgent} {
	global tank
	
	# Removed disarmErrorRate disarmCost minesIncrease
	foreach c {maxHealth maxEnergy maxRadar shieldCost projectileDamage \
				terrainDamage absorbDamage worldCountLimit \
				absorbDamage healthIncrease energyIncrease missilesIncrease \
				sounddist mapdim} {
		global $c
		$whichAgent eval "add-wme $tank($whichAgent,constLink) [string tolower $c] [subst $$c]"
	}
}

proc updateInputLink {whichAgent} {
#>-=>
# This function updates a couple of the generic things on the input-link:
# - the random #, the clock, and lastmove
#>-=>
	global tank agentsetup agentsetup2 sensor cycle wmeRecord
	
	if {$agentsetup($whichAgent) != "done"} {
		set agentsetup($whichAgent) done
	}
	if {$agentsetup2($whichAgent) != "done"} {
# 		initSensor $whichAgent $tank($whichAgent,lastmove) lastmove lastmove inputlink
#		initSensor $whichAgent [expr rand()] random random inputlink
# 		initSensor $whichAgent $cycle clock clock inputlink
#		set agentsetup2($whichAgent) done
	} else {
#		checkSensor $whichAgent [expr rand()] random random inputlink
# 		checkSensor $whichAgent $cycle clock clock inputlink
#  		checkSensor $whichAgent $tank($whichAgent,lastmove) lastmove lastmove inputlink
	}
}

proc sensorsToInputLink {whichAgent} {
	global tank agentsetup sensor healthcharge energycharge turn sensor wmeRecord actionTaken cycle
	
	if {$agentsetup($whichAgent) != "done"} {
		#setUpInputLink $whichAgent
		set agentsetup($whichAgent) done
		return
	}

	if {$tank($whichAgent,nearest) != ""} {
		set smelldist $tank($whichAgent,smell)
 		set smellcolor $tank($whichAgent,nearest)
 		set sound $tank($whichAgent,sound)
	} else {
		set smelldist none
		set smellcolor none
  		set sound silent
	}

	checkSensor $whichAgent [expr rand()] random random inputlink
 	checkSensor $whichAgent $cycle clock clock inputlink
   if {$tank($whichAgent,lastmove) == "resurrect-transport"} {
    	checkSensor $whichAgent yes resurrect resurrect inputlink
   } else {
    	checkSensor $whichAgent no resurrect resurrect inputlink
   }

#	checkSensor $whichAgent $tank($whichAgent,lives) lives livesval inputlink
	checkSensor $whichAgent $tank($whichAgent,health) health healthval inputlink
	checkSensor $whichAgent $tank($whichAgent,energy) energy energyval inputlink
	checkSensor $whichAgent $tank($whichAgent,missiles) missiles missilesval inputlink
#	checkSensor $whichAgent $tank($whichAgent,mines) mines minesval inputlink

#	checkSensor $whichAgent $turn tick tickval inputlink
	checkSensor $whichAgent $tank($whichAgent,x) x xval inputlink
	checkSensor $whichAgent $tank($whichAgent,y) y yval inputlink
	
	checkSensor $whichAgent $tank($whichAgent,dir) direction dirval inputlink

	checkSensor $whichAgent [yesOrNo [onHealth $tank($whichAgent,x) $tank($whichAgent,y)]] \
		healthrecharger healthrechargerval inputlink
	checkSensor $whichAgent [yesOrNo [onEnergy $tank($whichAgent,x) $tank($whichAgent,y)]] \
		energyrecharger energyrechargerval inputlink

	checkSensor2 $whichAgent $tank($whichAgent,Blockedforward) forward BlockedUpval blockedlink
	checkSensor2 $whichAgent $tank($whichAgent,Blockedbackward) backward BlockedDownval blockedlink
	checkSensor2 $whichAgent $tank($whichAgent,Blockedleft) left BlockedLeftval blockedlink
	checkSensor2 $whichAgent $tank($whichAgent,Blockedright) right BlockedRightval blockedlink
 
	checkSensor2 $whichAgent $tank($whichAgent,Incomingforward) forward incomingupval incominglink
	checkSensor2 $whichAgent $tank($whichAgent,Incomingbackward) backward incomingdownval incominglink
	checkSensor2 $whichAgent $tank($whichAgent,Incomingleft) left incomingleftval incominglink
	checkSensor2 $whichAgent $tank($whichAgent,Incomingright) right incomingrightval incominglink

	checkSensor2 $whichAgent $tank($whichAgent,RWavesforward) forward RWavesUpval rWavesLink
	checkSensor2 $whichAgent $tank($whichAgent,RWavesbackward) backward RWavesDownval rWavesLink
	checkSensor2 $whichAgent $tank($whichAgent,RWavesleft) left RWavesLeftval rWavesLink
	checkSensor2 $whichAgent $tank($whichAgent,RWavesright) right RWavesRightval rWavesLink
 
 	checkSensor2 $whichAgent $smelldist distance smelldistval smelllink
	checkSensor2 $whichAgent $smellcolor color smellcolorval smelllink
	checkSensor $whichAgent $sound sound soundval inputlink

	checkSensor $whichAgent $tank($whichAgent,shields) shield-status shieldval inputlink

	checkSensor $whichAgent $tank($whichAgent,radarSetting) radar-setting powerval inputlink
	checkSensor $whichAgent [onOrOff $tank($whichAgent,radarOn)] radar-status radarOnval inputlink
	checkSensor2 $whichAgent $tank($whichAgent,radardist) radar-distance radardistval inputlink
	
	if {$sensor($whichAgent,radarval) != $tank($whichAgent,radarInfo)} {
		set templist [lsort -decreasing -integer $wmeRecord($whichAgent,radarwme)]
		foreach i $templist {
			$whichAgent eval "remove-wme $i"
		}
		radarToInputLink $whichAgent
	}

   #>-=>
   # Add WME representing the move count
   #>-=>

#   if {[info exists actionTaken($whichAgent)] && $actionTaken($whichAgent)} {
#      if [info exists wmeRecord($whichAgent,actionCountWME)] {
#         $whichAgent eval "remove-wme $wmeRecord($whichAgent,actionCountWME)"
#      }
#      set wme [$whichAgent eval "add-wme $tank($whichAgent,inputlink) ^action-count *"]
#      scan $wme "%d" wmeRecord($whichAgent,actionCountWME)
#      #set actionTaken($whichAgent) 0
#   }

}

proc radarItemToInputLink {whichAgent radarItem objectDetected {coloredObject ""} {coloredObject2 ""}} {
	global tank wmeRecord
	
	set doorinput [$whichAgent eval "add-wme $tank($whichAgent,radarLink) $objectDetected *"]
	scan $doorinput "%d" wmeadded
	lappend wmeRecord($whichAgent,radarwme) $wmeadded
	set doorlink [lindex $doorinput 3]
	scan [$whichAgent eval "add-wme $doorlink distance [lindex $radarItem 1]"] "%d" wmeadded
	lappend wmeRecord($whichAgent,radarwme) $wmeadded
	if {$coloredObject2 != ""} { ;# Must be a flag
		scan [$whichAgent eval "add-wme $doorlink owner $coloredObject"] "%d" wmeadded
		lappend wmeRecord($whichAgent,radarwme) $wmeadded
		scan [$whichAgent eval "add-wme $doorlink occupier $coloredObject2"] "%d" wmeadded
		lappend wmeRecord($whichAgent,radarwme) $wmeadded
	} elseif {$coloredObject != ""} {
		scan [$whichAgent eval "add-wme $doorlink color $coloredObject"] "%d" wmeadded
		lappend wmeRecord($whichAgent,radarwme) $wmeadded
	}
	scan [$whichAgent eval "add-wme $doorlink position [lindex $radarItem 2]"] "%d" wmeadded
	lappend wmeRecord($whichAgent,radarwme) $wmeadded
	
}

proc radarToInputLink {whichAgent} {
	global sensor tank wmeRecord
	
	set wmeRecord($whichAgent,radarwme) [list]
	set sensor($whichAgent,radarval) $tank($whichAgent,radarInfo)
	foreach i $tank($whichAgent,radarInfo) {
		if {([lindex $i 0] == "open")} {
			radarItemToInputLink $whichAgent $i open
		} elseif {[lindex $i 0] == "health"} {
			radarItemToInputLink $whichAgent $i health
		} elseif {[lindex $i 0] == "energy"} {
			radarItemToInputLink $whichAgent $i energy
		} elseif {[lindex $i 0] == "missiles"} {
			radarItemToInputLink $whichAgent $i missiles
#		} elseif {[lindex $i 0] == "mines"} {
#			radarItemToInputLink $whichAgent $i mines
#		} elseif {[lindex $i 0] == "buriedMine"} {
#			radarItemToInputLink $whichAgent $i buriedmine
		} elseif {[lindex $i 0] == "wall"} {
			radarItemToInputLink $whichAgent $i obstacle
#		} elseif {[lindex $i 0] == "flag"} {
#			radarItemToInputLink $whichAgent $i flag [lindex $i 4] [lindex $i 3]
		} elseif {([lindex $i 0] != "open")} {
			radarItemToInputLink $whichAgent $i tank [lindex $i 0]
		}
	}
}

proc findNearestTank {whichAgent} {
	global tank mapdim tankList randomorder
	
	if {[llength $tankList] == 1} {
		return ""
	}
	set nearest ""
	set dxy [expr $mapdim * 3]

	set randomNumberList [randList [llength $tankList]]
	set randomizedTankList [list]

	foreach n $randomNumberList {
		lappend randomizedTankList [lindex $tankList $n]
	}

	foreach t $randomizedTankList {
		if {(($tank($t,status) == "active") && ($t != $whichAgent))} {
			set dx [expr abs($tank($whichAgent,x) - $tank($t,x))]
			set dy [expr abs($tank($whichAgent,y) - $tank($t,y))]
			if {$dxy > [expr $dx + $dy]} {
				set dxy [expr $dx + $dy]
				set nearest $t
			}
		}
	}
	return $nearest
}

proc getRadarBoundry {horizontal x y delta} {
global gridSize
	if {$delta < 0} { set deltaDir 0 } else { set deltaDir 1 }
	if {$horizontal==0} {
		set x1 [expr ($x * $gridSize) + 1]
		set x2 [expr (($x + 1) * $gridSize) - 1]
		set y1 [expr ($y + $deltaDir) * $gridSize]
		set y2 [expr ($y + $delta) * $gridSize]
	} else {
		set x1 [expr ($x + $deltaDir) * $gridSize]
		set x2 [expr (($x + $delta) * $gridSize) - $deltaDir]
		set y1 [expr ($y * $gridSize) + 1]
		set y2 [expr (($y + 1) * $gridSize) - 1]
	}
	
	return [list $x1 $y1 $x2 $y2]
}
		

proc checkBlockedIncoming {whichAgent direction} {
	global tank opposite map gridSize

	set opp $opposite($direction)
	set x $tank($whichAgent,x)
	set y $tank($whichAgent,y)
	
	switch -exact -- $direction {
		east { set changey 0; set changex 1; set delta $changex; set horizontal 1 }
		west { set changey 0; set changex -1; set delta $changex; set horizontal 1 }
		north { set changex 0; set changey -1 ; set delta $changey; set horizontal 0 }
		south { set changex 0; set changey 1 ; set delta $changey; set horizontal 0 }
	}
	
	if {$map([expr $x + $changex],[expr $y + $changey]) != 0} {
		set tank($whichAgent,Blocked[makeRelative $direction $tank($whichAgent,dir)]) yes
	} else {
		set dx $changex
		set dy $changey
		while {$map([expr $x + $dx],[expr $y + $dy]) == 0} {
			incr dx $changex
			incr dy $changey
		}

		if {$dx == 0} { set delta $dy } else { set delta $dx }
		set numbers [eval .wGlobal.map find overlapping \
						[getRadarBoundry $horizontal $x $y $delta]]
		set tagList {}
		foreach j $numbers {
			set itsTags [.wGlobal.map gettags $j]
			if {[lsearch $itsTags none] == -1} {
				lappend tagList $itsTags
			}
		}
		foreach j $tagList {
			if {(([lsearch -glob $j *fire*] != -1) && \
				([lsearch -glob $j $whichAgent*] == -1) && \
				([lsearch -glob $j *$opp*] != -1))} {
				set tank($whichAgent,Incoming[makeRelative $direction $tank($whichAgent,dir)]) yes
				break
			}
		}
		#set numbers [eval .wGlobal.map find overlapping \
      #   [expr $tank($whichAgent,x) * $gridSize] \
      #   [expr $tank($whichAgent,y) * $gridSize] \
      #   [expr (1 + $tank($whichAgent,x)) * $gridSize] \
      #   [expr (1 + $tank($whichAgent,y)) * $gridSize]]
		#set tagList {}
		#foreach j $numbers {
		#	set itsTags [.wGlobal.map gettags $j]
		#	if {[lsearch $itsTags none] == -1} {
		#		lappend tagList $itsTags
		#	}
		#}
		#foreach j $tagList {
		#	if {([lsearch $j radar] != -1) && \
		#		([lsearch $j $whichAgent] == -1)} {
		#		set tank($whichAgent,RWaves[makeRelative $direction $tank($whichAgent,dir)]) yes
		#		break
		#	}
		#}

	}
}

proc clearAllAgentSensors {} {
   global tank
   
   foreach {a1 a2} [array get tank *,RWaves*] {
      set tank($a1) no
   }
   foreach {a1 a2} [array get tank *,Incoming*] {
      set tank($a1) no
   }
   foreach {a1 a2} [array get tank *,Blocked*] {
      set tank($a1) no
   }
}

proc updateAgentSensors {whichAgent} {
global tank map gridSize sounddist randomorder actionTaken
	#>-=>
	# This function examines the environment data and sets the sensor-stuff
	# of the tank array.
	#>-=>
	
	set tank($whichAgent,nearest) [findNearestTank $whichAgent]
	
	if {$tank($whichAgent,nearest) != ""} {
 
		set dx [expr ($tank($whichAgent,x) - $tank($tank($whichAgent,nearest),x))]
		set dy [expr ($tank($whichAgent,y) - $tank($tank($whichAgent,nearest),y))]
		set adx [expr abs($dx)]
		set ady [expr abs($dy)]
		set distance [expr $adx + $ady]
		set tank($whichAgent,smell) $distance
	 
		#>-=>
		# Set the sound sensor
		#>-=>
		if {$distance <= $sounddist} {
	 
			set dirlist [list north south west east]
			set randlist [randList 4]
			set randdirs [list]
			foreach n $randlist {
				lappend randdirs [lindex $dirlist [expr $n]]
			}
 
			set snd [search $tank($whichAgent,x) $tank($whichAgent,y) $randdirs $whichAgent]
			if {($snd != 0) && ($actionTaken($tank($whichAgent,nearest)) == 1)} {
				set tank($whichAgent,sound) [makeRelative $snd $tank($whichAgent,dir)]
			} else {
				set tank($whichAgent,sound) silent
			}
		} else {
			set tank($whichAgent,sound) silent
		}
	}
	
	#>-=>
	# Set the other sensors
	#>-=>
	checkBlockedIncoming $whichAgent south
	checkBlockedIncoming $whichAgent west
	checkBlockedIncoming $whichAgent east
	checkBlockedIncoming $whichAgent north
	
	updateRadarSensor $whichAgent
}

proc updateRadarSensor {whichAgent} {
    global map tank gridSize mapdim current radar_box i_tank_small

    set tank($whichAgent,radardist) 0
	 set tank($whichAgent,radarInfo) [list]
 
    if {($tank($whichAgent,radarOn) == 0) || ($tank($whichAgent,radarSetting) == 0)} {
     	 return
    }
    	
    set x $tank($whichAgent,x)
    set y $tank($whichAgent,y)
    set dir $tank($whichAgent,dir)

	 set map($tank($whichAgent,x),$tank($whichAgent,y)) 0
    if {$dir == "east"} {
	    set i 0
	    # This next part calls scanCoordinate on all the squares visible to the right (y+-1)
	    while {($i <= $tank($whichAgent,radarSetting)) && \
		         ($map([expr $tank($whichAgent,x) + $i],$tank($whichAgent,y)) == 0)} {
	       scanCoordinate $whichAgent $map([expr $tank($whichAgent,x) + $i],$tank($whichAgent,y)) \
		       $i 1 [expr $tank($whichAgent,x) + $i] $tank($whichAgent,y)
	       scanCoordinate $whichAgent $map([expr $tank($whichAgent,x) + $i],[expr $tank($whichAgent,y) - 1]) \
		       $i 0 [expr $tank($whichAgent,x) + $i] [expr $tank($whichAgent,y) - 1]
	       scanCoordinate $whichAgent $map([expr $tank($whichAgent,x) + $i],[expr $tank($whichAgent,y) + 1]) \
		       $i 2 [expr $tank($whichAgent,x) + $i] [expr $tank($whichAgent,y) + 1]
	        incr i 1
 	    }
	    if {[expr $i - 1] < $tank($whichAgent,radarSetting)} {
	        scanCoordinate $whichAgent $map([expr $tank($whichAgent,x) + $i],$tank($whichAgent,y)) \
	 	        $i 1 [expr $tank($whichAgent,x) + $i] $tank($whichAgent,y)
	    }
	    set tank($whichAgent,radardist) [expr $i-1]
	    set actualRadarDist $i
    } elseif {$dir == "west"} {
	    set i 0
	    while {($i <= $tank($whichAgent,radarSetting)) && \
		      ($map([expr $tank($whichAgent,x) - $i],$tank($whichAgent,y)) == 0)} {
	       scanCoordinate $whichAgent $map([expr $tank($whichAgent,x) - $i],$tank($whichAgent,y)) \
		       $i 1 [expr $tank($whichAgent,x) - $i] $tank($whichAgent,y)
	       scanCoordinate $whichAgent $map([expr $tank($whichAgent,x) - $i],[expr $tank($whichAgent,y) + 1]) \
		       $i 0 [expr $tank($whichAgent,x) - $i] [expr $tank($whichAgent,y) + 1]
	       scanCoordinate $whichAgent $map([expr $tank($whichAgent,x) - $i],[expr $tank($whichAgent,y) - 1]) \
		       $i 2 [expr $tank($whichAgent,x) - $i] [expr $tank($whichAgent,y) - 1]
	       incr i 1
	    }
	    if {[expr $i - 1] < $tank($whichAgent,radarSetting)} {
	       scanCoordinate $whichAgent $map([expr $tank($whichAgent,x) - $i],$tank($whichAgent,y)) \
		       $i 1 [expr $tank($whichAgent,x) - $i] $tank($whichAgent,y)
	    }
	    set tank($whichAgent,radardist) [expr $i-1]
	    set actualRadarDist $i
    } elseif {$dir == "north"} {
	    set i 0
	    while {($i <= $tank($whichAgent,radarSetting)) && \
		       ($map($tank($whichAgent,x),[expr $tank($whichAgent,y) - $i]) == 0)} {
	       scanCoordinate $whichAgent $map($tank($whichAgent,x),[expr $tank($whichAgent,y) - $i]) \
		       $i 1 $tank($whichAgent,x) [expr $tank($whichAgent,y) - $i]
	       scanCoordinate $whichAgent $map([expr $tank($whichAgent,x) - 1],[expr $tank($whichAgent,y) - $i]) \
		       $i 0 [expr $tank($whichAgent,x) - 1] [expr $tank($whichAgent,y) - $i]
	       scanCoordinate $whichAgent $map([expr $tank($whichAgent,x) + 1],[expr $tank($whichAgent,y) - $i]) \
		       $i 2 [expr $tank($whichAgent,x) + 1] [expr $tank($whichAgent,y) - $i]
	       incr i 1
	    }
	    if {[expr $i - 1] < $tank($whichAgent,radarSetting)} {
	       scanCoordinate $whichAgent $map($tank($whichAgent,x),[expr $tank($whichAgent,y) - $i]) \
		       $i 1 $tank($whichAgent,x) [expr $tank($whichAgent,y) - $i]
	    }
	    set tank($whichAgent,radardist) [expr $i-1]
	    set actualRadarDist $i
    } elseif {$dir == "south"} {
	    set i 0
	    while {($i <= $tank($whichAgent,radarSetting)) && \
		      ($map($tank($whichAgent,x),[expr $tank($whichAgent,y) + $i]) == 0)} {
	       scanCoordinate $whichAgent $map($tank($whichAgent,x),[expr $tank($whichAgent,y) + $i]) \
		      $i 1 $tank($whichAgent,x) [expr $tank($whichAgent,y) + $i]
	       scanCoordinate $whichAgent $map([expr $tank($whichAgent,x) - 1],[expr $tank($whichAgent,y) + $i]) \
		      $i 2 [expr $tank($whichAgent,x) - 1] [expr $tank($whichAgent,y) + $i]
	       scanCoordinate $whichAgent $map([expr $tank($whichAgent,x) + 1],[expr $tank($whichAgent,y) + $i]) \
		      $i 0 [expr $tank($whichAgent,x) + 1] [expr $tank($whichAgent,y) + $i]
	       incr i 1
	    }
	    if {[expr $i - 1] < $tank($whichAgent,radarSetting)} {
	       scanCoordinate $whichAgent $map($tank($whichAgent,x),[expr $tank($whichAgent,y) + $i]) \
		       $i 1 $tank($whichAgent,x) [expr $tank($whichAgent,y) + $i]
	    }
	    set tank($whichAgent,radardist) [expr $i-1]
	    set actualRadarDist $i
    }
	 set map($tank($whichAgent,x),$tank($whichAgent,y)) $whichAgent
}

proc scanCoordinate {whichAgent filler dist pos x y} {
    global map tank mapdim gridSize current healthcharge energycharge powerUpList\
	    radar_box i_obstruct_small i_ground_small i_tank_small i_oasish_small i_oasisb_small \
	    opposite

    if {$pos == 0} {
		set side left
    } elseif {$pos == 1} {
		set side center
    } elseif {$pos == 2} {
		set side right
    }
    if {$filler == 1} {
		lappend tank($whichAgent,radarInfo) [list wall $dist $side]
		return
    }
    if {$filler == 0} {
		lappend tank($whichAgent,radarInfo) [list open $dist $side]
    	if {[info exists powerUpList($x,$y)]} {
    		switch -exact -- $powerUpList($x,$y) {
				health {
					lappend tank($whichAgent,radarInfo) [list health $dist $side]
				}
				energy {
					lappend tank($whichAgent,radarInfo) [list energy $dist $side]
				}
				missiles {
					lappend tank($whichAgent,radarInfo) [list missiles $dist $side]
				}
				#mines {
				#	lappend tank($whichAgent,radarInfo) [list mines $dist $side]
				#}
				#buriedMine {
				#	lappend tank($whichAgent,radarInfo) [list buriedMine $dist $side]
				#}
				#flag {
				#	lappend tank($whichAgent,radarInfo) [list flag $dist $side $powerUpList($x,$y,owner) $powerUpList($x,$y,sender)] 
				#}
			}
		}
		return
    } else {
      # Another tank occupies this space
		lappend tank($whichAgent,radarInfo) [list open $dist $side]
		lappend tank($whichAgent,radarInfo) [list $filler $dist $side]
    	if {[info exists powerUpList($x,$y)] && ($powerUpList($x,$y) == "flag")} {
			lappend tank($whichAgent,radarInfo) [list flag $dist $side \
				$powerUpList($x,$y,sender) $powerUpList($x,$y,owner)]
		}
		# This is the one place where we are updating sensors for a different
		# agent.  If we see an agent with our radar, that agent's rwaves sensor is set
		set tank($filler,RWaves[makeRelative $opposite($tank($whichAgent,dir)) $tank($filler,dir)]) yes
    }
}
