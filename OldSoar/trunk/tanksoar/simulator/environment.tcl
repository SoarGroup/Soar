proc calcScore {whichAgent} {
    global tank Pts_Kill_Bonus Pts_Death_Penalty Pts_Flag_Bonus Pts_Flag_Secured gameType projectileDamage Pts_Hit_Bonus Pts_Hit_Penalty
 
   if {$gameType == "flag"} {
	set flagBonus [expr ($Pts_Flag_Secured * $tank($whichAgent,myFlagSecured)) \
		+ ($Pts_Flag_Bonus * [llength $tank($whichAgent,flagList)])]
    } else {
	set flagBonus 0
    }

    return  [expr  round( \
	    ($Pts_Hit_Bonus * ($tank($whichAgent,offensiveDamage)/$projectileDamage)) \
	    + ($Pts_Hit_Penalty * ($tank($whichAgent,defensiveDamage)/$projectileDamage)) \
	    + ($Pts_Kill_Bonus * $tank($whichAgent,kills)) \
	    + ($Pts_Death_Penalty * $tank($whichAgent,deaths)))]
	    #+ $flagBonus \
#	    - (0.5 * $tank($whichAgent,terrainDamage)) \
}

proc disarmMine {whichAgent} {
global tank powerUpList disarmErrorRate
	switch -exact -- $tank($whichAgent,dir) {
		east { set x [expr $tank($whichAgent,x) + 1]; set y $tank($whichAgent,y) }
		west { set x [expr $tank($whichAgent,x) - 1]; set y $tank($whichAgent,y) }
		north { set y [expr $tank($whichAgent,y) - 1]; set x $tank($whichAgent,x) }
		south { set y [expr $tank($whichAgent,y) + 1]; set x $tank($whichAgent,x) }
	}
	if {[info exists powerUpList($x,$y)] && $powerUpList($x,$y) == "buriedMine"} {
		unset powerUpList($x,$y)
		draw_mapsector $x $y
		if {[expr rand()] <= disarmErrorRate} {
			explodeTank $whichAgent $powerUpList($x,$y,sender)
		}
		unset powerUpList($x,$y,sender)
	}
}

proc calcWeaponStartPos {x y dir} {
   global gridSize
   
   switch -exact -- $dir {
      north {
         set retPos(x) [expr ($x * $gridSize)]
         set retPos(y) [expr (($y+1) * $gridSize) - 10]
      }
      south {
         set retPos(x) [expr ($x * $gridSize)]
         set retPos(y) [expr (($y-1) * $gridSize) + 10]
      }
      east {
         set retPos(x) [expr (($x-1) * $gridSize) + 10]
         set retPos(y) [expr ($y * $gridSize)]
      }
      west {
         set retPos(x) [expr (($x+1) * $gridSize) - 12]
         set retPos(y) [expr ($y * $gridSize)]
      }
   }

   return [array get retPos]
}

proc startWeapon {whichAgent weaponType} {
	global tank map gridSize powerUpList
	
	if {$weaponType == "missile"} {
   	array set newPos [calcWeaponStartPos $tank($whichAgent,x) $tank($whichAgent,y) $tank($whichAgent,dir)]
   	set x $newPos(x)
   	set y $newPos(y)
		set newProjectile(dir) $tank($whichAgent,dir)

	} else {
		if {![info exists powerUpList($tank($whichAgent,x),$tank($whichAgent,y))] || \
			($powerUpList($tank($whichAgent,x),$tank($whichAgent,y)) != "flag")} {
			set powerUpList($tank($whichAgent,x),$tank($whichAgent,y)) buriedMine
			set powerUpList($tank($whichAgent,x),$tank($whichAgent,y),sender) $whichAgent
		}
		return
	}

	if {($tank($whichAgent,dir) == "north") || ($tank($whichAgent,dir) == "west")} {
   	if {$map([convertToLogical2 $x],[convertToLogical2 $y]) == 1} {
      	 return
	   }
	} else {
   	if {$map([convertToLogical $x],[convertToLogical $y]) == 1} {
      	 return
	   }

	}

	set newProjectile(x) $x
	set newProjectile(y) $y
	set newProjectile(imageTag) [subst $whichAgent][subst $tank($whichAgent,dir)]fire[genSym]
	set newProjectile(sender) $whichAgent
	lappend map(projectiles) [array get newProjectile]
	
	drawProjectiles
}

proc calcBoundingBox {x1 y1 oldx oldy} {
   
   if {$x1 < $oldx} {
      set x1 [expr $x1 + 2]
      set oldx [expr $x1 + ($oldx - $x1)/2]
      set retX1 [expr $x1 + 10]
      set retX2 [expr $oldx + 23]
   } else {
      set x1 [expr $x1 - 2]
      set oldx [expr $oldx + ($x1 - $oldx)/2]
      set retX1 [expr $oldx + 10]
      set retX2 [expr $x1 + 23]
   }
      
   if {$y1 < $oldy} {
      set y1 [expr $y1 + 2]
      set oldy [expr $y1 + ($oldy - $y1)/2]
      set retY1 [expr $y1 + 8]
      set retY2 [expr $oldy + 25]
   } else { 
      set y1 [expr $y1 - 2]
      set oldy [expr $oldy + ($y1 - $oldy)/2]
      set retY1 [expr $oldy + 8]
      set retY2 [expr $y1 + 25]
   }

   set retList [list $retX1 $retY1 $retX2 $retY2]
   return $retList
}

proc moveProjectiles {} {
	global map projectileSpeed tankList tank absorbDamage
	
	foreach p $map(projectiles) {
		array set projectile $p
      set projectile(oldx) $projectile(x)
      set projectile(oldy) $projectile(y)
		switch -exact -- $projectile(dir) {
			east { set projectile(x) [expr $projectile(x) + $projectileSpeed] }
			west { set projectile(x) [expr $projectile(x) - $projectileSpeed] }
			north { set projectile(y) [expr $projectile(y) - $projectileSpeed] }
			south { set projectile(y) [expr $projectile(y) + $projectileSpeed] }
		}
		set map(projectiles) [ldelete $map(projectiles) $p]
		set explode 0
		set boundingBox [calcBoundingBox $projectile(x) $projectile(y) $projectile(oldx) $projectile(oldy)]
			foreach t $tankList {
				if {(($t != $projectile(sender)) &&
					([projectileContact $t $boundingBox] == 1)) } {
					set explode 1
					if {(($tank($t,shields) == "on")
							&& ![onHealth $tank($t,x) $tank($t,y)]
      					&& ![onEnergy $tank($t,x) $tank($t,y)])} {
						incr tank($t,energy) [expr 0 - $absorbDamage]
						if {$tank($t,energy) <= 0} {
							set tank($t,energy) 0
							set tank($t,shields) "off"
							set tank($t,radarOn) 0
						}
					} else {
						explodeTank $t $projectile(sender)
					}
				}
			}
		

		if {[projectileContact obstruction $boundingBox] == 1} {
			set explode 1
		}
		
		if {$explode == 0} {
			lappend map(projectiles) [array get projectile]
		} else {
			.wGlobal.map delete $projectile(imageTag)
		}	
	}
}

proc explodeTank {whichAgent sender} {
	global tank projectileDamage gameType
	set tank($whichAgent,exploding) 1

	#>-=>
	# If the tank is on an oasis, it is vulnerable and will be destroyed
	# otherwise, decrease health
	#>-=>
	if {($gameType == "deathmatch") &&
		([onHealth $tank($whichAgent,x) $tank($whichAgent,y)] ||
		[onEnergy $tank($whichAgent,x) $tank($whichAgent,y)])} {
	   set tank($whichAgent,health) 0
	} else {
		incr tank($whichAgent,health) [expr 0 - $projectileDamage]
	}

	if {[info exists tank($sender,offensiveDamage)]} {
		incr tank($sender,offensiveDamage) $projectileDamage
	}
	incr tank($whichAgent,defensiveDamage) $projectileDamage		
	if {$tank($whichAgent,health) <= 0} {
		set tank($whichAgent,health) 0
		#incr tank($whichAgent,deaths) 1
		if {[info exists tank($sender,kills)]} {
			incr tank($sender,kills) 1
		}
	}

	drawTank $whichAgent
}

proc drawProjectiles {} {
	global map i_fireball
	
	foreach p $map(projectiles) {
		array set projectile $p
		.wGlobal.map delete $projectile(imageTag)
		.wGlobal.map create image [expr $projectile(x) + 10] \
			[expr $projectile(y) + 8] -image $i_fireball \
			-anchor nw -tag $projectile(imageTag)
	}
}

proc projectileContact {tagTarget boundingBox} {
	set contactTags [eval ".wGlobal.map find overlapping $boundingBox"]
	foreach thisTag $contactTags {
		if {([lsearch [.wGlobal.map gettags $thisTag] $tagTarget] != -1) && \
			([lsearch [.wGlobal.map gettags $thisTag] radar] == -1)} {
			return 1
		}
	}
	return 0
}

proc restartMap {} {
	global boardX boardY globalTick powerUpList

	restartAllTanks
	setInitialPowerUps
	set globalTick 1

}

proc tickSimulation {w} {
	global globalTick ticksPerMove ticksPerTankCycle tankList tank
			  
	if [info exists globalTick] {
		set globalTick [expr ($globalTick + 1) % $ticksPerMove]
	} else {
		set globalTick 1
	}
	if [info exists tankList] {
		foreach tankName $tankList {
			if {$tank($tankName,status) == "active"} {
				set thisTanksObject [$w find withtag $tankName]
				if {$thisTanksObject == ""} {
					tk_dialog .error Warning \
								 "$tankName tank canvas object not found" warning 0 Ok
					continue
				}
				#if [info exists tank($tankName,tick)] {
				#	set tank($tankName,tick) \
				#		 [expr ($tank($tankName,tick)+1)%$ticksPerTankCycle]
				#} else {
					#set tank($tankName,tick) 0
				#}
			}
		}
		## If we have completed a global tick cycle, move the tanks, 
		## otherwise just do animation
		if !$globalTick {
			updateworld
		}
	}
}

proc dontStepSim {} {
    ##  we need this when stopAfterDecision is set, so sim doesn't
    ##  run twice as fast
    global skipSimTick
    set skipSimTick True
}

proc getRandomDir {} {

	set d [random 4]
	switch $d {
		1 { return north }
		2 { return east }
		3 { return south }
		4 { return west }
	}
}

proc regeneratePowerUps {} {
global tankList powerUpList PU_HealthMax PU_EnergyMax PU_MinesMax PU_MissilesMax
	set numTanks [llength tankList]
	set numMissiles 0
	#set numMines 0
	set numEnergy 0
	set numHealth 0
	set maxMissilesPU [expr ceil($PU_MissilesMax * $numTanks)]
	set maxHealthPU [expr ceil($PU_HealthMax * $numTanks)]
	#set maxMinesPU [expr ceil($PU_MinesMax * $numTanks)]
	set maxEnergyPU [expr ceil($PU_EnergyMax * $numTanks)]

	foreach {a b} [array get powerUpList] {
		switch $b {
			missiles { incr numMissiles }
			#mines { incr numMines }
			health { incr numHealth }
			energy { incr numEnergy }
		}
	}
	if {$numHealth < $maxHealthPU} {
		set genNum [expr [random [expr $maxHealthPU - $numHealth + 1]] - 1]
		for {set i 0} {$i < $genNum} {incr i} {
			array set newPowerUp [getRandomPos "yes"]
			set powerUpList($newPowerUp(x),$newPowerUp(y)) health
			draw_mapsector $newPowerUp(x) $newPowerUp(y)
		}
	}
	if {$numEnergy < $maxEnergyPU} {
		set genNum [expr [random [expr $maxEnergyPU - $numEnergy + 1]] - 1]
		for {set i 0} {$i < $genNum} {incr i} {
			array set newPowerUp [getRandomPos "yes"]
			set powerUpList($newPowerUp(x),$newPowerUp(y)) energy
			draw_mapsector $newPowerUp(x) $newPowerUp(y)
		}
	}
	if {$numMissiles < $maxMissilesPU} {
		set genNum [expr [random [expr $maxMissilesPU - $numMissiles + 1]] - 1]
		for {set i 0} {$i < $genNum} {incr i} {
			array set newPowerUp [getRandomPos "yes"]
			set powerUpList($newPowerUp(x),$newPowerUp(y)) missiles
			draw_mapsector $newPowerUp(x) $newPowerUp(y)
		}
	}
	#if {$numMines < $maxMinesPU} {
	#	set genNum [expr [random [expr $maxMinesPU - $numMines + 1]] - 1]
	#	for {set i 0} {$i < $genNum} {incr i} {
	#		array set newPowerUp [getRandomPos "yes"]
	#		set powerUpList($newPowerUp(x),$newPowerUp(y)) mines
	#		draw_mapsector $newPowerUp(x) $newPowerUp(y)
	#	}
	#}
}
	
proc setInitialPowerUps {} {
global PU_HealthMax PU_EnergyMax PU_MinesMax PU_MissilesMax powerUpList
	
	unset powerUpList
	for {set i 0} {$i < [expr ceil($PU_HealthMax)]} {incr i} {
		array set newPowerUp [getRandomPos "yes"]
		set powerUpList($newPowerUp(x),$newPowerUp(y)) health
	}
	for {set i 0} {$i < [expr ceil($PU_EnergyMax)]} {incr i} {
		array set newPowerUp [getRandomPos "yes"]
		set powerUpList($newPowerUp(x),$newPowerUp(y)) energy		
	}
	for {set i 0} {$i < [expr ceil($PU_MissilesMax)]} {incr i} {
		array set newPowerUp [getRandomPos "yes"]
		set powerUpList($newPowerUp(x),$newPowerUp(y)) missiles		
	}
	#for {set i 0} {$i < [expr ceil($PU_MinesMax)]} {incr i} {
	#	array set newPowerUp [getRandomPos "yes"]
	#	set powerUpList($newPowerUp(x),$newPowerUp(y)) mines		
	#}
}

proc getPowerUp {whichAgent type {sender none} } {
global tank healthIncrease maxHealth energyIncrease maxEnergy \
		missilesIncrease minesIncrease powerUpList

	switch -exact -- $type {
		missiles {
			incr tank($whichAgent,missiles) $missilesIncrease
		}
		#mines {
		#	incr tank($whichAgent,mines) $minesIncrease
		#}
		#buriedMine {
		#	explodeTank $whichAgent $sender
		#}
		#flag {
		#	captureFlag $whichAgent
		#}
	}
}

proc captureFlag {whichAgent} {
global powerUpList captureFlagBonus tank
	
	set occupier $powerUpList($tank($whichAgent,x),$tank($whichAgent,y),sender)
	set owner $powerUpList($tank($whichAgent,x),$tank($whichAgent,y),owner)
	
	if {$whichAgent != $occupier} {
		set powerUpList($tank($whichAgent,x),$tank($whichAgent,y),sender) $whichAgent
		#>-=>
		# Adjust flag ownership
		#>-=>
		if {$occupier != $owner} {
			set tank($occupier,flagList) [ldelete $tank($occupier,flagList) $owner]
		} else {
			set tank($owner,myFlagSecured) 0
		}
		if {$whichAgent == $owner} {
			set tank($whichAgent,myFlagSecured) 1
		} else {
			lappend tank($whichAgent,flagList) $owner			
		} 
	}
}
	
proc pickUpMapContents {whichAgent} {
global tank powerUpList

	if {[info exists powerUpList($tank($whichAgent,x),$tank($whichAgent,y))]} {
		if {[info exists powerUpList($tank($whichAgent,x),$tank($whichAgent,y),sender)]} {
			# Must be a flag or mine (because only they have a sender)
			if {$powerUpList($tank($whichAgent,x),$tank($whichAgent,y)) == "flag"} {
				captureFlag $whichAgent
				return
			} else {
				getPowerUp $whichAgent $powerUpList($tank($whichAgent,x),$tank($whichAgent,y)) \
					$powerUpList($tank($whichAgent,x),$tank($whichAgent,y),sender)
				unset powerUpList($tank($whichAgent,x),$tank($whichAgent,y),sender)
			}
		} else {
			if {($powerUpList($tank($whichAgent,x),$tank($whichAgent,y)) == "health") || \
				($powerUpList($tank($whichAgent,x),$tank($whichAgent,y)) == "energy")} {
				# Can't pick up a recharging oasis
				return
			}
			getPowerUp $whichAgent $powerUpList($tank($whichAgent,x),$tank($whichAgent,y))
		}
		unset powerUpList($tank($whichAgent,x),$tank($whichAgent,y))
	}
}

proc getRandomPos {considerPowerUps} {
	global mapdim map powerUpList
	
	set boardX $mapdim
	set boardY $mapdim
	set badlocationCnt 0
	set gridOkay 0
   
	while {!($gridOkay)} {
		set x [expr [random $boardX] -1]
		set y [expr [random $boardY] -1]
		set contents $map($x,$y)
		set gridOkay 1
		if {($contents == 1) || 
			(($considerPowerUps == "yes") && [info exists powerUpList($x,$y)])} {
			set gridOkay 0
			incr badlocationCnt
		} else {
			set gridOkay 1
		}
	
		if {$badlocationCnt > 2000} {
			Debug "A good random location was not found in $badlocationCnt tries."
			tk_dialog .error Warning "A good random location was not found in $badlocationCnt tries.." \
						 warning 0 Ok
			break
		}
	}

	set retValue(x) $x
	set retValue(y) $y
	return [array get retValue]

}

proc randomlyPlaceAgent {whichAgent} {
	global tank 
	
	array set newPos [getRandomPos "yes"]
	set tank($whichAgent,x) $newPos(x)
	set tank($whichAgent,y) $newPos(y)
	set tank($whichAgent,dir) [getRandomDir]
	pickUpMapContents $whichAgent

}

proc environmentRun {args} {
global runningSimulation

    tsiOnEnvironmentRun

	if {$args != ""} {
		set n 1
		scan $args %d n
		for {set i 1} {$i <= $n} {incr i} {
#                       changed next line from step to fix bugzilla bug 393
			environmentStep
		}
		return
	}
	set runningSimulation 1
	runSimulation .wGlobal.map
}

proc environmentStep {} {
global runningSimulation

    tsiOnEnvironmentStep
	set runningSimulation 0
	runSimulation .wGlobal.map
}

proc environmentStop {} {
global runningSimulation

	if { $runningSimulation } {
		set runningSimulation 0
	}
	
	tsiOnEnvironmentStop
}

proc runSimulation {w} {
global runningSimulation tickDelay soarTimePerTick soarTimeUnit tankList \
	 worldCount worldCountLimit winningTank scoreLimit tank skipSimTick tsiConfig

	set agents [interp slaves]
	set agents [ldelete $agents siu]

	if {$agents == ""} {
		tk_dialog .error Warning "There are currently no tanks to run." \
					 warning 0 Ok
		set runningSimulation 0
		return
		# could be a bug.  May need to call tsiOnEnvironmentStop
	}

	set hun	[expr $worldCount % 200]
		if { !$hun } {
				puts "World Count -> $worldCount"
		}

	if {$worldCount >= $worldCountLimit} {
		set runningSimulation 0
		
			if { $tsiConfig(autorun) == 0 } {
					tk_dialog .info {Game Over} \
							"Time limit reached. Winner is $winningTank" info 0 Ok

			} else {
					puts -nonewline "FINAL SCORES:"
					foreach t $tankList {
							set s $tank($t,score)

							puts -nonewline " $s"
					}
					puts "\n\n"
					exit
			}
			
		return
	}

	if {[info exists winningTank] && $tank($winningTank,score) >= $scoreLimit} {
		set runningSimulation 0
			if { $tsiConfig(autorun) == 0 } {

					tk_dialog .info {Game Over} \
							"$scoreLimit points achieved. Winner is $winningTank" info 0 Ok

			} else {
					puts -nonewline "FINAL SCORES:"
					foreach t $tankList {
							set s $tank($t,score)

							puts -nonewline " $s"

					}
					puts "\n\n"
					exit
			}

		set scoreLimit 9999
		return
	}

   if {$skipSimTick} {
		 set skipSimTick False
	} else {
		 incr worldCount
		 tickSimulation $w
			if { $tsiConfig(autorun) == 0 } {	
					update 
			}
	}
	[lindex $agents 0] eval [list run-soar $soarTimePerTick $soarTimeUnit]
	if {[info exists runningSimulation] && $runningSimulation} {
		after $tickDelay runSimulation $w
	} else {
		environmentStop
	}
}

proc loadController {agent color} {
	$color eval {tsiDisplayAndSendCommand {excise -all}}
	if [catch "$color eval \
		 [list tsiDisplayAndSendCommand [list [list source $agent.soar]]]" msg] {
		tk_dialog .error Warning \
					 "Couldn't load control code for $agent: $msg" error 0 Ok
	}
}

proc setActionRequest {agent action value ID} {
	global actionRequest
	if {$action == "removeUtterance"} {
		lappend actionRequest($agent,$action) $value
		lappend actionRequest($agent,$action,ID) $ID
		return
	}
	if {[info exists actionRequest($agent,$action)] && ([expr rand()] >= 0.5)} {
		set actionRequest($agent,$action) $value
		set actionRequest($agent,$action,ID) $ID
	} else {
		set actionRequest($agent,$action) $value
		set actionRequest($agent,$action,ID) $ID
	}
}

proc getRelativeStrafeDirectionX {direction velocityCommand} {
	
	if {($direction == "west" || $direction == "east")} {
		return 0
	}
	if {$direction == "south"} {
		set tempResult -1
	} else {
		set tempResult 1
	}
	if {$velocityCommand == "left"} {
		return [expr -1 * $tempResult]
	} else {
		return $tempResult
	}
}

proc getRelativeStrafeDirectionY {direction velocityCommand} {
	
	if {($direction == "north" || $direction == "south")} {
		return 0
	}
	if {$direction == "west"} {
		set tempResult -1
	} else {
		set tempResult 1
	}
	if {$velocityCommand == "left"} {
		return [expr -1 * $tempResult]
	} else {
		return $tempResult
	}
}
proc getRelativeMoveDirectionX {direction velocityCommand} {
	
	if {($direction == "north" || $direction == "south")} {
		return 0
	}
	if {$direction == "west"} {
		set tempResult -1
	} else {
		set tempResult 1
	}
	if {$velocityCommand == "backward"} {
		return [expr -1 * $tempResult]
	} else {
		return $tempResult
	}
}

proc getRelativeMoveDirectionY {direction velocityCommand} {
	
	if {($direction == "west" || $direction == "east")} {
		return 0
	}
	if {$direction == "north"} {
		set tempResult -1
	} else {
		set tempResult 1
	}
	if {$velocityCommand == "backward"} {
		return [expr -1 * $tempResult]
	} else {
		return $tempResult
	}
}
