proc finishAction {whichAgent whichAction} {
global actionRequest
	unset actionRequest($whichAgent,$whichAction)
	if {[info exists actionRequest($whichAgent,$whichAction,ID)]} {
		$whichAgent eval "add-wme $actionRequest($whichAgent,$whichAction,ID) status complete"
	unset actionRequest($whichAgent,$whichAction,ID)
	}
}

proc finishActionList {whichAgent whichAction} {
global actionRequest

	unset actionRequest($whichAgent,$whichAction)
	if {[info exists actionRequest($whichAgent,$whichAction,ID)]} {
		#puts "Attempting to handle $actionRequest($whichAgent,$whichAction,ID)"
		foreach i $actionRequest($whichAgent,$whichAction,ID) {
			#puts "Adding completion to $i"
			$whichAgent eval "add-wme $i status complete"
		}
	unset actionRequest($whichAgent,$whichAction,ID)
	}
}

proc changeState {whichAgent} {
	global tank current healthIncrease energyIncrease \
		maxHealth maxEnergy healthTax shieldCost actionRequest map radarOn \
		disarmCost powerUpList gameType mapdim
	
	#puts "Action requests:  [array get actionRequest]"
	
	set tank($whichAgent,oldx) $tank($whichAgent,x)
	set tank($whichAgent,oldy) $tank($whichAgent,y)
	
	if {[info exists actionRequest($whichAgent,removeUtterance)] && [winfo exists .siuWin]} {
		foreach u $actionRequest($whichAgent,removeUtterance) {
			puts "Removing utterance $u"
			removeUtterance $whichAgent $u
		}
		finishActionList $whichAgent removeUtterance
	}	

	#>-=>
	# Process the movement commands (can only do one per move)
	#>-=>
	if {[info exists actionRequest($whichAgent,rotateDir)]} {
		rotateTank $whichAgent $actionRequest($whichAgent,rotateDir)
		finishAction $whichAgent rotateDir
	} elseif {[info exists actionRequest($whichAgent,moveDir)]} {
		moveTank $whichAgent $actionRequest($whichAgent,moveDir)
		finishAction $whichAgent moveDir
	}

	if {[info exists actionRequest($whichAgent,activateShields)]} {
	   if {($actionRequest($whichAgent,activateShields) == "on") || ($actionRequest($whichAgent,activateShields) == "off")} {
   		set tank($whichAgent,shields) $actionRequest($whichAgent,activateShields)
	   	finishAction $whichAgent activateShields
	   }
	}	
  	if {[info exists actionRequest($whichAgent,fireWeapon)]} {
		if {($gameType == "flag") && ($actionRequest($whichAgent,fireWeapon) == "mine-disarmer")} {
			if {$tank($whichAgent,energy) >= $disarmCost} {
				incr tank($whichAgent,energy) [expr 0 - $disarmCost]
				disarmMine $whichAgent
			}
	  } else {
			if {($actionRequest($whichAgent,fireWeapon) == "missile") &&
			($tank($whichAgent,missiles) > 0)} {
				 incr tank($whichAgent,missiles) -1
				 startWeapon $whichAgent $actionRequest($whichAgent,fireWeapon)
			} elseif {(($gameType == "flag") && \
					  ($actionRequest($whichAgent,fireWeapon) == "mine") &&
				 ($tank($whichAgent,mines) > 0) &&
				 ($powerUpList($tank($whichAgent,x),$tank($whichAgent,y)) != "buriedMine")) } {
					  incr tank($whichAgent,mines) -1
					  startWeapon $whichAgent $actionRequest($whichAgent,fireWeapon)
			}
	 	}
		finishAction $whichAgent fireWeapon
	}	

	if {[info exists actionRequest($whichAgent,activateRadar)]} {
		#Debug "Attempting to do activateRadar command $actionRequest($whichAgent,activateRadar)"
		if {($actionRequest($whichAgent,activateRadar) == "off")} {
			set tank($whichAgent,radarOn) 0
		} elseif {($actionRequest($whichAgent,activateRadar) == "on")} {
			set tank($whichAgent,radarOn) 1
		}
		finishAction $whichAgent activateRadar
	}
	
	if {[info exists actionRequest($whichAgent,changeRadar)]} {
	   if {($actionRequest($whichAgent,changeRadar) >= 0) && ($actionRequest($whichAgent,changeRadar) <= $mapdim)} {
   		set tank($whichAgent,radarSetting) $actionRequest($whichAgent,changeRadar)
   	}
		finishAction $whichAgent changeRadar
	}
	
	if {[onHealth $tank($whichAgent,x) $tank($whichAgent,y)] && \
		($tank($whichAgent,health) < $maxHealth)} {
 		if {[expr $tank($whichAgent,health) + $healthIncrease] <= $maxHealth} {
			incr tank($whichAgent,health) $healthIncrease
		} else {
			set tank($whichAgent,health) $maxHealth
		}		
	}

	if {[onEnergy $tank($whichAgent,x) $tank($whichAgent,y)] && \
		($tank($whichAgent,energy) < $maxEnergy)} {
		if {[expr $tank($whichAgent,energy) + $energyIncrease] <= $maxEnergy} {
			incr tank($whichAgent,energy) $energyIncrease
		} else {
			set tank($whichAgent,energy) $maxEnergy
		}
	}

	#>-=>
	# Tally energy costs and turn off stuff if necessary
	#>-=>

	if {$tank($whichAgent,shields) == "on"} {
		if {$tank($whichAgent,energy) >= $shieldCost} {
	 		incr tank($whichAgent,energy) [expr 0 - $shieldCost]
	 	} else {
	 		set tank($whichAgent,shields) "off"
	 	}
	 }
		
	if {$tank($whichAgent,radarOn)} {
		if {$tank($whichAgent,radarSetting) >= $tank($whichAgent,energy)} {
			set tank($whichAgent,radarSetting) $tank($whichAgent,energy)
		}
 		incr tank($whichAgent,energy) [expr 0 - $tank($whichAgent,radarSetting)]
	}	

	checkForKill $whichAgent
	
	 if {($whichAgent == $current) && ($tank($whichAgent,agenttype) == "human")} {
		set radarOn $tank($whichAgent,radarOn)
		.wControls.bottomframe.radar set $tank($whichAgent,radarSetting)
	}
	
	drawTank $whichAgent
}
proc updateSensors {whichAgent} {
   global tank current agentsetup2
	if {($tank($whichAgent,agenttype) == "soar") && ($tank($whichAgent,running) == "yes") && \
		($agentsetup2($whichAgent) == "done")} {
		sensorsToInputLink $whichAgent
	}
	if {$whichAgent == $current} {
		drawAgentInfo $current
	}
}   

proc updateState {whichAgent} {
	global tank
	
	if {$tank($whichAgent,health) <= 0} {
		resurrect $whichAgent
	}
	updateAgentSensors $whichAgent
	if {($tank($whichAgent,agenttype) == "soar") && ($tank($whichAgent,running) == "no")} {
		set tank($whichAgent,running) yes
	}
}

proc updateworld {} {
	global tankList tank cycle decisions_per_update randomorder \
		 current turn shieldCost agentsetup2 regenerateFreq actionTaken \
		 winningTank

	set randomizedTankList [randList [llength $tankList]]
	set randnames [list]
	foreach n $randomizedTankList {
		lappend randnames [lindex $tankList $n]
	}
	
	
	if {[expr $cycle%$decisions_per_update] == 0} {
		incr turn 1
		foreach t $randnames {
			if {$tank($t,status) == "active"} {
				set tank($t,lastmove) none
				changeState $t
			}
		}
   	
		if {[expr $cycle%$regenerateFreq] == 0} {
			regeneratePowerUps
		}

   	moveProjectiles
	   drawProjectiles
		foreach t $randnames {
      	checkForKill $t
		}

      clearAllAgentSensors
		foreach t $randnames {
			if {$tank($t,status) == "active"} {
				updateState $t
			}
		}
		foreach t $randnames {
			if {$tank($t,status) == "active"} {
				updateSensors $t
			}
		}

		set highScore -99999
		foreach t $randnames {
			set actionTaken($t) 0
			set tank($t,score) [calcScore $t]
         if {$tank($t,score) > $highScore} {
            set highScore $tank($t,score)
            set winningTank $t
         }
		}
		drawscore
	}
#	foreach t $randnames {
#		if {($tank($t,status) == "active") && ($agentsetup2($t) == "done")} {
#			if {$tank($t,agenttype) == "soar"}  {
#				updateInputLink $t
#			}
#		}
#	}
	incr cycle 1
}

proc manualUpdate {whichAgent} {
	global tank current actionRequest tankList cycle
	
	#>-=>
	# Only do immediate updating if an all-human game.
	#>-=>
	
	foreach t $tankList {
		if {($tank($t,status) == "active") && ($tank($t,agenttype) == "soar")} {
			return
		}
	}

	if {$tank($whichAgent,status) == "active"} {
		set tank($whichAgent,lastmove) none
		changeState $whichAgent
	}
	
   moveProjectiles
	drawProjectiles
	foreach t $tankList {
      	checkForKill $t
	}
	refreshWorld
	incr cycle 1
}

proc refreshWorld {} {
	global tankList tank agentsetup2

	clearAllAgentSensors
	foreach t $tankList {
		if {($tank($t,status) == "active") && ($agentsetup2($t) == "done")} {
			updateState $t
		}
	}

	foreach t $tankList {
		if {$tank($t,status) == "active"} {
			updateSensors $t
		}
	}

	drawscore

}
