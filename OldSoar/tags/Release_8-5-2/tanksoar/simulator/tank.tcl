proc loadTankNames {button} {
   global currentName
   cd agents
	set allNames [glob -nocomplain {*.soar}]
   cd ..

   set names ""
   foreach i $allNames {
      if {[file extension $i]==".soar"} {
         lappend names [file rootname [file tail $i]]
      }
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

proc registerHumanAgent {name} {
   global showAgent ETCPConfig tsiAgentInfo localAgents
   
   .agentcreation.destroyAgent.m add command -label $name \
       -command [list destroyAgent $name]
  
    set tsiAgentInfo($name,addFN) localAgentAddWME
    set tsiAgentInfo($name,rmFN) localAgentRemoveWME

	lappend localAgents $name    
}


proc createTank {x y agentPath agentFile newname} {
	#>-=>
	# Sets up an agent
	# Input: x,y grid coordinates, file name to source and the name of the agent (color)
	#>-=>
	global map tank current tankList sensor agentsetup agentsetup2 \
		  soarTimeUnit actionTaken powerUpList sensorList sensorInfo \
              ETCPConfig soarTimeUnit windowPrefs tsiConfig
	
	#Debug "Starting createTank with parameters $x $y $agentFile $newname"
	#>-=>

	# Find a name (color) for the new tank, then create agent
	#>-=>

	if {$newname == ""} {
		puts "Can't create another tank"
		return
	}

	if {[info exists tankList] && [lsearch $tankList $newname] >= 0} {
			tk_dialog .error Error \
					 "An tank with color $newname already exists...not creating a new one\nYou must choose a different color." \
					 error 0 Ok
			error "Tried to create duplicate tanks"
	}

	if {[string last ".soar" $agentFile] == -1 && [string last ".rete" $agentFile] == -1 } { 
			set agentFile "$agentFile.soar"
	}

	createNewAgent $newname $agentPath $agentFile
	if {![interp exists $newname]} {
	   return 0
	}
	if [info exists windowPrefs($newname)] {
		$newname eval wm geometry .tsw $windowPrefs($newname)
	}
	if {$soarTimeUnit == {e}} {
	  $newname eval input-period 0
	} else {
	  $newname eval input-period 1
	}
	
	#>-=>
	# Create aliases for the new slave interpretor
	#>-=>
	$newname alias setUpInputLink setUpInputLink $newname
	$newname alias updateInputLink updateInputLink $newname
	$newname alias removeInputLink removeInputLink $newname
	$newname alias setActionRequest setActionRequest $newname
	$newname eval [list set sensorList $sensorList]
	$newname eval [list array set sensorInfo [array get sensorInfo]]
	if {$newname == "red"} {
		$newname alias Talk Talk
	} else {
		$newname alias Talk puts
	}
	$newname eval rename run run-soar
	$newname alias run environmentRun
	$newname alias step environmentStep
	$newname alias stop environmentStop
	#$newname alias stop-soar environmentStop
 	if $ETCPConfig(afterDecision) { 
#	   $newname eval monitor -add after-decision-phase-cycle \"stop-soar -self \{\}\" dp1
#         changed to fix bugzilla bug 392
	  $newname eval monitor -add after-decision-phase-cycle \"stop-soar -self\" dp1
  	}
   if $ETCPConfig(afterDecision) {
      stopAfterDecision
      set ETCPConfig(runTilOutputGen) 0
      set soarTimeUnit d
   } else {
      dontStopAfterDecision
   }
   if $ETCPConfig(runTilOutputGen) {
      set ETCPConfig(afterDecision) 0
      dontStopAfterDecision
      set soarTimeUnit out
   } else {
      set soarTimeUnit d
   }

	#$newname eval {.tsw.frame.step configure \
	#					 -command {tsiDisplayAndSendCommand step}}
	
	#>-=>
	# Source the agent.soar code and the specific behavior code
	#>-=>	
	if [catch {$newname eval \
			  [list uplevel #0 source agent.tcl]} msg] {
	  tk_dialog .error Warning \
				"Couldn't load interface code: $msg" warning 0 Ok
	}
	
	#>-=>
	# Update the name lookup array
	#>-=>
	if [info exists tankList] {
	  lappend tankList $newname
	} else {
	  set tankList $newname
	}


	if {$x == -1} {
		array set newPos [getRandomPos "yes"]
		set tank($newname,x) $newPos(x)
		set tank($newname,y) $newPos(y)
	} else {
		set tank($newname,x) $x
		set tank($newname,y) $y
	}

	set tank($newname,dir) [getRandomDir]
	set map($tank($newname,x),$tank($newname,y)) $newname	
	#set powerUpList($tank($newname,x),$tank($newname,y)) flag
	#set powerUpList($tank($newname,x),$tank($newname,y),sender) $newname
	#set powerUpList($tank($newname,x),$tank($newname,y),owner) $newname
	
	if {$current != ""} {
		set oldcurrent $current
	
		eraseAgentInfo
		eraseRadarSensor $oldcurrent
	}
	set current $newname

	set oldcurrent $current

	set current $newname
	if {$current == "none"} {
		if {$oldcurrent == ""} {
			return
		}
		set current $oldcurrent
		return 0
	}

	set tank($current,status) active
	
	set actionTaken($newname) 0

	set tank($current,initialx) $tank($current,x)
	set tank($current,initialy) $tank($current,y)
	set tank($current,oldx) $tank($current,x)
	set tank($current,oldy) $tank($current,y)
	set tank($current,type) "soar"

	setInitialTankInfo $current
	
	drawTank $newname
	refreshWorld

	updateAvailableAgentColors
	if {[winfo exists .siuWin]} {
		updateAvailAddrAgentsMenu	
	}

	#$newname eval addTSIProductionMenu $tsiConfig(tswFrame)
	$newname eval [list Debugger::UpdateStats]
	#$newname eval [list MatchTree::CreateProductionMenu]
	return 1
}


proc createSoarTank {x y agentPath agentFile color {startdir random}} {
	global tank map gridSize powerUpList
	
	if {$color == ""} {
		error "Can't create another tank"
		return
	}
	set xpos [expr round($x/$gridSize)]
	set ypos [expr round($y/$gridSize)]
	if [info exists powerUpList($xpos,$ypos)] {
			tk_dialog .error Error \
		 "Cannot create a tank on a powerup." error 0 Ok
		 return
   }	   
	if {$map($xpos,$ypos) == 0} {
		if {[createTank $xpos $ypos $agentPath $agentFile $color]} {
   		if {$startdir != "random"} {
	   		set tank($color,dir) $startdir
		   	drawTank $color
      	   refreshWorld
		   }
		}
	}
}

proc createHumanTank {x y newname {startdir random}} {
	global tank map gridSize map tank current tankList agentsetup agentsetup2 \
		  soarTimeUnit actionTaken powerUpList

	if {$newname == ""} {
		error "Can't create another tank"
		return
	}
	if {[info exists tankList] && [lsearch $tankList $newname] >= 0} {
			tk_dialog .error Error \
					 "An tank with color $newname already exists...not creating a new one\nYou must choose a different color." \
					 error 0 Ok
			#error "Tried to create duplicate tanks"
			return
	}

	set xpos [expr round($x/$gridSize)]
	set ypos [expr round($y/$gridSize)]
	if [info exists powerUpList($xpos,$ypos)] {
			tk_dialog .error Error \
		 "Cannot create a tank on a powerup." error 0 Ok
		 return
   }	   
	
	if {$x < 0} {
		array set newPos [getRandomPos "yes"]
		set xpos $newPos(x)
		set ypos $newPos(y)
	}
	
	if {$map($xpos,$ypos) != 0} {
		error "Cannot create a human tank there."
		return
	} else {
		set tank($newname,x) $xpos
		set tank($newname,y) $ypos
		if {$startdir != "random"} {
			set tank($newname,dir) $startdir
		} else {
			set tank($newname,dir) [getRandomDir]
		}
	}		

	if [info exists tankList] {
	  lappend tankList $newname
	} else {
	  set tankList $newname
	}

	set map($tank($newname,x),$tank($newname,y)) $newname	
	#set powerUpList($tank($newname,x),$tank($newname,y)) flag
	#set powerUpList($tank($newname,x),$tank($newname,y),sender) $newname
	#set powerUpList($tank($newname,x),$tank($newname,y),owner) $newname

	if {$current != ""} {
		set oldcurrent $current
		#drawTank $oldcurrent
	
		eraseAgentInfo
		eraseRadarSensor $oldcurrent
	}
	set current $newname

	set oldcurrent $current
	#.wGlobal.map dtag cur

	set current $newname
	if {$current == "none"} {
		if {$oldcurrent == ""} {
			return
		}
		set current $oldcurrent
		return
	}

	set tank($current,status) active
	
	set actionTaken($newname) 0

	set tank($current,initialx) $tank($current,x)
	set tank($current,initialy) $tank($current,y)
	set tank($current,oldx) $tank($current,x)
	set tank($current,oldy) $tank($current,y)
	set tank($current,type) "human"
	
	setInitialTankInfo $current
	
	updateAvailableAgentColors
	registerHumanAgent $newname
	
	if {[winfo exists .siuWin]} {
		updateAvailAddrAgentsMenu	
	}

	drawTank $newname
	refreshWorld

} 

 
proc restartTank {whichAgent} {
	global tank
	
	if {$tank($whichAgent,agenttype) == "soar"} {
		$whichAgent eval tsiDisplayAndSendCommand init-soar
	} else {
		initagent $whichAgent
	} 
	restartAgent $whichAgent
}

proc restartAllTanks {} {
	global tank tankList cycle turn

	foreach t $tankList {
	if {$tank($t,status) == "active"} {
		restartTank $t
	}
	}
	set worldCount 0
	set cycle 1
	set turn 1
}

proc restartAgent {whichAgent} {
	global tank tankList map agentsetup sensor maxHealth \
		maxEnergy sensor agentsetup agentsetup2 initHealth radarOn
	
	#setInitialTankInfo $whichAgent
	set x $tank($whichAgent,initialx)
	set y $tank($whichAgent,initialy)
	
	if {$map($x,$y) != 0} {
	if {$map($x,$y) != $whichAgent} {
		puts "Can't restart $whichAgent because starting position is occupied."
		puts "Resetting $whichAgent without changing position."
		return
	}
	}

	set map($tank($whichAgent,x),$tank($whichAgent,y)) 0
	
	set tank($whichAgent,x) $x
	set tank($whichAgent,y) $y

	eraseAgentInfo
	eraseTank $whichAgent
	eraseRadar $whichAgent
	
	set map($x,$y) $whichAgent
	set radarOn 1
#	set tank($whichAgent,tick) 0
	set tank($whichAgent,score) 0
	set actionTaken($whichAgent) 1

	foreach {ar1 ar2} [array get actionRequest] {
		unset actionRequest($ar1)
	}

	drawTank $whichAgent
	refreshWorld
}

proc resurrect {whichAgent} {
	global tank map initHealth maxEnergy originalMap initialMissiles initialMines

	set tank($whichAgent,lives) [expr $tank($whichAgent,lives) - 1]

	if {$tank($whichAgent,lives) <= 0} {
		totallyDeadTank $whichAgent
	} else {
		set tank($whichAgent,missiles) $initialMissiles
#		set tank($whichAgent,mines) $initialMines
		set tank($whichAgent,health) $initHealth
		set tank($whichAgent,radarSetting) 5
		set tank($whichAgent,energy) $maxEnergy
		set tank($whichAgent,radarOn) 1
		set tank($whichAgent,radarInfo) [list]
		set tank($whichAgent,tags) [list]
		set map($tank($whichAgent,x),$tank($whichAgent,y)) \
			$originalMap($tank($whichAgent,x),$tank($whichAgent,y))
		randomlyPlaceAgent $whichAgent
		set map($tank($whichAgent,x),$tank($whichAgent,y)) $whichAgent
		set tank($whichAgent,lastmove) resurrect-transport
		set tank($whichAgent,exploding) 0
		drawTank $whichAgent
	}
	refreshWorld
}

proc removeInputLink {whichAgent} {
   global inputLinkWME inputLinkID deleteList tank
   
   catch "unset inputLinkID($whichAgent)"
   catch "unset deleteList($whichAgent)"
   if {[info exists actionTaken($whichAgent)]} {
	   unset actionTaken($whichAgent)
	}

 }
 
proc unsetSensors {whichAgent} {
global sensor wmeRecord
# Note:  removed minesval lasmove livesval and tickval 
	if {[array get sensor $whichAgent*] != [list]} {
		foreach item { shieldval healthval energyval missilesval \
						xval yval dirval BlockedUpval BlockedDownval BlockedLeftval \
						BlockedRightval incomingupval incomingdownval incomingleftval incomingrightval \
						powerval radarOnval RWavesUpval RWavesDownval RWavesLeftval RWavesRightval \
						smelldistval smellcolorval soundval radardistval random clock \
						healthrechargerval energyrechargerval resurrect} {
         if [info exists sensor($whichAgent,$item)] {
			   unset sensor($whichAgent,$item)
			}
         if [info exists wmeRecord($whichAgent,$item)] {
   			unset wmeRecord($whichAgent,$item)
   		}
		}
      if [info exists wmeRecord($whichAgent,radarwme)] {
   		unset wmeRecord($whichAgent,radarwme)
		}
		#unset wmeRecord($whichAgent,actionCountWME)
      if [info exists sensor($whichAgent,radarval)] {
	   	unset sensor($whichAgent,radarval)
	   }
	}
}

proc unsetTankSensorInfo {whichAgent} {
global tank
	foreach item {RWavesforward RWavesbackward RWavesright RWavesleft Blockedforward \
					Blockedbackward Blockedright Blockedleft Incomingforward \
					Incomingbackward Incomingright Incomingleft} {
      if [info exists tank($whichAgent,$item)] {
   		unset tank($whichAgent,$item)
   	}
	}
	foreach item {smell sound nearest} {
		if {[info exists tank($whichAgent,$item)]} {
			unset tank($whichAgent,$item)
		}
	}
}

proc unsetTankInfo {whichAgent} {
global tank
# Note: removed mines tick from following list
	foreach item {lives health radarSetting shields energy missiles radarOn \
					myFlagSecured flagList dir x y initialx initialy oldx oldy \
					radarInfo agenttype type radardist exploding \
					lastmove running status offensiveDamage defensiveDamage \
					terrainDamage kills deaths score} {
		unset tank($whichAgent,$item)
	}		
	unsetTankSensorInfo $whichAgent
	if {[info exists tank($whichAgent,inputlink)]} {
		foreach item {inputlink outputlink smelllink incominglink rWavesLink \
						blockedlink radarLink constLink} {
			unset tank($whichAgent,$item)
		}
	}
}

proc setInitialTankInfo {whichAgent} {
global tank radarOn maxHealth maxEnergy initHealth initLives agentsetup agentsetup2 \
		initialMissiles initialMines

	set tank($whichAgent,health) $initHealth
	set tank($whichAgent,lives) $initLives
	set tank($whichAgent,energy) $maxEnergy
	set tank($whichAgent,missiles) $initialMissiles
#	set tank($whichAgent,mines) $initialMines
	
	set tank($whichAgent,kills) 0
	set tank($whichAgent,deaths) 0
	set tank($whichAgent,score) 0

	set tank($whichAgent,offensiveDamage) 0
	set tank($whichAgent,defensiveDamage) 0
	set tank($whichAgent,terrainDamage) 0
	set tank($whichAgent,flagList) {}
	set tank($whichAgent,myFlagSecured) 1
 
	set tank($whichAgent,radarSetting) 1
	set tank($whichAgent,radarOn) 1
	set tank($whichAgent,radarInfo) [list]
	set tank($whichAgent,radardist) 0
	set tank($whichAgent,lastmove) none
#	set tank($whichAgent,tick) 0
	set radarOn 1
	set tank($whichAgent,exploding) 0
	set tank($whichAgent,shields) "off"
	
   # Initialize the sensors to empty
   foreach s {RWaves Incoming Blocked} {
      foreach d {forward backward right left} {
         set tank($whichAgent,$s$d) no
      }
   }
   set tank($whichAgent,nearest) ""
   
	if {$tank($whichAgent,type) == "soar"} {
		set tank($whichAgent,agenttype) soar
		set tank($whichAgent,running) no
		set agentsetup($whichAgent) notdone
		set agentsetup2($whichAgent) notdone
	} else {
		set tank($whichAgent,agenttype) human
		set tank($whichAgent,running) yes
		set agentsetup($whichAgent) done
		set agentsetup2($whichAgent) done
	}
	
	set map($tank($whichAgent,x),$tank($whichAgent,y)) $whichAgent
}

proc initagent {whichAgent} {
	global tank tankList map agentsetup sensor maxHealth \
		maxEnergy sensor agentsetup agentsetup2 initHealth radarOn \
		map originalMap

	set x $tank($whichAgent,x)
	set y $tank($whichAgent,y)
	
	set map($tank($whichAgent,x),$tank($whichAgent,y)) \
		$originalMap($tank($whichAgent,x),$tank($whichAgent,y))
		
	eraseAgentInfo
	eraseTank $whichAgent
	eraseRadar $whichAgent
	
	set tank($whichAgent,status) inactive
	
	if {$tank($whichAgent,agenttype) == "soar"} {
		if {$agentsetup($whichAgent) == "done"} {
			unsetSensors $whichAgent
		}
	}
	
	unset agentsetup($whichAgent)
	unset agentsetup2($whichAgent)
	unsetTankSensorInfo $whichAgent
	setInitialTankInfo $whichAgent
	
	set tank($whichAgent,status) active
}

proc checkForKill {whichAgent} {
	global tank
 
     if {$tank($whichAgent,health) <= 0} {
	 	set tank($whichAgent,health) 0
 		incr tank($whichAgent,deaths) 1
     }
}

proc totallyDeadTank {whichAgent} {
	global tank tankList map current agentsetup sensor agentsetup2

	set map($tank($whichAgent,x),$tank($whichAgent,y)) 0
	
	eraseAgentInfo
	eraseTank $whichAgent
	eraseRadar $whichAgent
	
	set tank($whichAgent,status) dead

	if {$tank($whichAgent,agenttype) == "soar"} {
		if {$agentsetup($whichAgent) == "done"} {
			#unsetSensors $whichAgent
		}
		if {$agentsetup2($whichAgent) == "done"} {
			# unset sensor($whichAgent,lastmove)	
			# unset sensor($whichAgent,random)
			# unset sensor($whichAgent,clock)
		}	
	}
	
	#>-=>
	# To make sure that this dead agent is not involved in other agents
	# sensor readings
	#>-=>	
	set tank($whichAgent,x) 10000
	set tank($whichAgent,y) 10000

	if {$current == $whichAgent} {
		set current ""
	}
	
	refreshWorld
}



proc envDestroyAgent {whichAgent} {
global tank tankList map current agentsetup sensor map originalMap\
	agentsetup2 cycle turn worldCount powerUpList actionRequest winningTank
	
	set map($tank($whichAgent,x),$tank($whichAgent,y)) \
		$originalMap($tank($whichAgent,x),$tank($whichAgent,y)) 


	### Clean up all the input-link stuff for this name, in case a new
	### one comes along.  Have to do this before deleting the canvas
	### object.
		
	eraseAgentInfo
	eraseTank $whichAgent
	eraseRadar $whichAgent
	drawscore

	removeInputLink $whichAgent
	set obj [.wGlobal.map find withtag $whichAgent]
	.wGlobal.map delete $obj
	set tankList [ldelete $tankList $whichAgent]
	if {$tankList == {}} {
		set cycle 1
		set turn 0        
		set worldCount 0
		set scoreLimit 50
	}
	updateAvailableAgentColors
	if {[winfo exists .siuWin]} {
		updateAvailAddrAgentsMenu	
	}
		
	foreach t $tankList {
		if {[lsearch $tank($t,flagList) $whichAgent] != -1} {
			set tank($t,flagList) [ldelete $tank($t,flagList) $whichAgent]
			lappend tank($t,flagList) deadAgent
		}
	}
			
	
	set tank($whichAgent,status) inactive
	if {$tank($whichAgent,agenttype) == "soar"} {
		if {$agentsetup($whichAgent) == "done"} {
			unsetSensors $whichAgent
		}
	}
		
	#>-=>
	# Erase the tank's flag
	#>-=>
	if [info exists powerUpList($tank($whichAgent,initialx),$tank($whichAgent,initialy))] {
   	unset powerUpList($tank($whichAgent,initialx),$tank($whichAgent,initialy))
   }
	if [info exists powerUpList($tank($whichAgent,initialx),$tank($whichAgent,initialy),sender)] {
		unset powerUpList($tank($whichAgent,initialx),$tank($whichAgent,initialy),sender)
	}
	if [info exists powerUpList($tank($whichAgent,initialx),$tank($whichAgent,initialy),owner)] {
	unset powerUpList($tank($whichAgent,initialx),$tank($whichAgent,initialy),owner)
}
	draw_mapsector $tank($whichAgent,initialx) $tank($whichAgent,initialy)
	
	if {[string compare $tank($whichAgent,agenttype) "human"] == 0} {
		set returnVal 0
	} else {
		set returnVal 1
	}
	
	unset agentsetup($whichAgent)
	unset agentsetup2($whichAgent)
	unsetTankInfo $whichAgent
	foreach {ar1 ar2} [array get actionRequest] {
	   if {[string match $whichAgent* $ar1]} {
   		unset actionRequest($ar1)
   	}
	}
	
	if {$current == $whichAgent} {
		set current [lindex $tankList 0]
	}
   
   if {[info exists winningTank] && ($winningTank == $whichAgent)} {
      unset winningTank
   }
	refreshWorld

	return returnVal
}

proc selectTank {x y} {
	global tank current radarOn tankList

	set oldcurrent $current
	.wGlobal.map dtag cur
	.wGlobal.map addtag cur closest $x $y
	set current [lindex [.wGlobal.map gettags cur] 0]
	.wAgentInfo.radar delete smallRadar
	if {$oldcurrent != ""} {
		#drawTank $oldcurrent
		eraseAgentInfo
		eraseRadarSensor $oldcurrent
	}
	if {[lsearch $tankList $current] != -1} {
		if {$tank($current,radarOn) == 1} {
			set radarOn 1
		} else {
			set radarOn 0
		}
		#drawTank $current
		refreshRadar $current
		drawAgentInfo $current
	} else {
		set current ""
	}
}

proc moveTank {whichAgent moveCommand} {
	global gridSize tank map originalMap terrainDamage actionTaken

	set movement 0
	if {$tank($whichAgent,health) != 0} {
		set dx 0
		set dy 0
		if {($moveCommand == "forward" || $moveCommand == "backward")} {
			set dx [getRelativeMoveDirectionX $tank($whichAgent,dir) $moveCommand]
			set dy [getRelativeMoveDirectionY $tank($whichAgent,dir) $moveCommand]
		} else {
			set dx [getRelativeStrafeDirectionX $tank($whichAgent,dir) $moveCommand]
			set dy [getRelativeStrafeDirectionY $tank($whichAgent,dir) $moveCommand]
		}
		set newX [expr $tank($whichAgent,x) + $dx]
		set newY [expr $tank($whichAgent,y) + $dy]
		if {$map($newX,$newY) == 0} {
			set movement 1
			set tank($whichAgent,lastmove) physical-move
			set map($tank($whichAgent,x),$tank($whichAgent,y)) \
				$originalMap($tank($whichAgent,x),$tank($whichAgent,y))
			draw_mapsector $tank($whichAgent,x) $tank($whichAgent,y)
			set tank($whichAgent,x) $newX
			set tank($whichAgent,y) $newY 
			set map($tank($whichAgent,x),$tank($whichAgent,y)) $whichAgent
			pickUpMapContents $whichAgent
		} else {
			set tank($whichAgent,health) [expr ($tank($whichAgent,health) - $terrainDamage)]
			incr tank($whichAgent,terrainDamage) $terrainDamage
			Debug "Tank hit terrain. was at $tank($whichAgent,x) $tank($whichAgent,y) trying to go $moveCommand to $newX $newY"
			if {$tank($whichAgent,health) <= 0} {
				incr tank($whichAgent,terrainDamage) $tank($whichAgent,health)
				set tank($whichAgent,health) 0
				incr tank($whichAgent,deaths) 1
			}
		}
	}
	set actionTaken($whichAgent) 1
	return $movement
}

proc rotateTank {whichAgent direction} {
	global tank actionTaken

	if {$tank($whichAgent,health) != 0} {
		if {$direction == "right"} {
			if {$tank($whichAgent,dir) == "north"} {
				set tank($whichAgent,dir) east
			} elseif {$tank($whichAgent,dir) == "south"} {
				set tank($whichAgent,dir) west
			} elseif {$tank($whichAgent,dir) == "west"} {
				set tank($whichAgent,dir) north
			} elseif {$tank($whichAgent,dir) == "east"} {
				set tank($whichAgent,dir) south
			}
		}
		if {$direction == "left"} {
			if {$tank($whichAgent,dir) == "north"} {
				set tank($whichAgent,dir) west
			} elseif {$tank($whichAgent,dir) == "south"} {
				set tank($whichAgent,dir) east
			} elseif {$tank($whichAgent,dir) == "west"} {
				set tank($whichAgent,dir) south
			} elseif {$tank($whichAgent,dir) == "east"} {
				set tank($whichAgent,dir) north
			}
		}
		set actionTaken($whichAgent) 1
	}
}

