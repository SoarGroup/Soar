set initLives 10000
set initHealth 1000
set initialMissiles 15
set initialMines 1

set healthIncrease 150
set energyIncrease 200
set missilesIncrease 7
set minesIncrease 2

set powerUpList {}
set regenerateFreq 12
if {$gameType == "deathmatch"} {
	set PU_HealthMax 0.1
	set PU_EnergyMax 0.1
	set PU_MinesMax 0
} else {
	set PU_HealthMax 0
	set PU_EnergyMax 0
	set PU_MinesMax 2
}
set PU_MissilesMax 3

set maxHealth 1000
set maxEnergy 1000
set maxRadar 14

set projectileSpeed 48
set projectileDamage 400
set disarmErrorRate 0.5

set disarmCost 125
set shieldCost 20
set terrainDamage 100
set absorbDamage 250

set scoreLimit 50
set Pts_Kill_Bonus 3.0
set Pts_Death_Penalty -2.0
set Pts_Hit_Bonus 2.0
set Pts_Hit_Penalty -1.0

set Pts_Flag_Bonus 25.0
set Pts_Flag_Secured 100.0

set sounddist 7
set decisions_per_update 1

set cycle 1
set turn 0        
set worldCount 0
set worldCountLimit 4000
set tickDelay 0
set ticksPerMove 1
set ticksPerTankCycle 10
set soarTimePerTick 1
set soarTimeUnit d
set mapdim 16
set skipSimTick False

set possibleAgentColors {red blue yellow green orange purple black}

set backgroundColor white
set buttonColor slategrey
set buttonBackgroundColor steelblue

set current ""

set cardinalToRelative(north) forward
set cardinalToRelative(west) left
set cardinalToRelative(south) backward
set cardinalToRelative(east) right

set leftRelative(forward) right
set leftRelative(left) forward
set leftRelative(backward) left
set leftRelative(right) backward

set opposite(forward) backward
set opposite(left) right
set opposite(backward) forward
set opposite(right) left

set opposite(north) south
set opposite(south) north
set opposite(east) west
set opposite(west) east

set sensorList {move rotate radar fire radar-power shields remove-utterance}
set sensorInfo(move,actionName) moveDir
set sensorInfo(rotate,actionName) rotateDir
set sensorInfo(fire,actionName) fireWeapon
set sensorInfo(radar,actionName) activateRadar
set sensorInfo(radar-power,actionName) changeRadar
set sensorInfo(shields,actionName) activateShields
set sensorInfo(remove-utterance,actionName) removeUtterance
set sensorInfo(move,parameterName) direction
set sensorInfo(rotate,parameterName) direction
set sensorInfo(fire,parameterName) weapon
set sensorInfo(radar,parameterName) switch
set sensorInfo(radar-power,parameterName) setting
set sensorInfo(shields,parameterName) switch
set sensorInfo(remove-utterance,parameterName) ID
