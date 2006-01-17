#>-=>
# Intialize Tcl Soar Interface
#>-=>
set ETCPConfig(AgentName) Tank
set ETCPConfig(AgentFolder) [file join .. agents]

if { [llength $argv] < 1 } {
		set mode normal
		set tsiConfig(autorun) 0
		puts "Mode normal"
} else {
		set mode commandline 
		set tsiConfig(autorun) 1
}

source PDFileMenus.tcl
source et-controlpanel.tcl

if { $mode == "normal" } {

  tsi 1 -controlpanel makeETControlPanel

} else {
		
#		source [file join $tsi_library "tsiInit.tcl"]
		tsi 0 -controlpanel makeETControlPanel
}

#>-=>
# Set up utilities
#>-=>
source FSBox.tcl
source utilities.tcl
DebugOn

#>-=>
# Source the map data
#>-=>
source ./maps/defaultMap.map
#source ./maps/emptyMap.map

#>-=>
# Set up some map,tank arrays
#>-=>
array set originalMap [array get map]
set map(projectiles) [list]
set tankList {}
set runningSimulation 0
#>-=>
# Set up constants
#>-=>
source constants_game.tcl
source constants_windows.tcl
source constants_images.tcl

#>-=>
# Set up this specific environemt
#>-=>
source tankworld_graphics.tcl
source tank.tcl
source tankworld_update.tcl
source tankworld_search.tcl
source tankworld_sensors.tcl
source environment.tcl


#>-=>
# Source TSI and it's control panel
#>-=>
source tsi_display.tcl


#>-=>
# Set up this environment's windows
#>-=>
source initWindows.tcl
source windowGlobal.tcl
source windowAgentInfo.tcl
source windowManualControl.tcl
source windowConstants.tcl


if {[file exists "SIU_Interface.tcl"]} {
	interp create siu
	#load {} Tk siu
	#siu eval [list source constants_game.tcl]
	source SIU_Interface.tcl
    siu eval [list source load_SIU.tcl]
	setUpSIUWindow
	siu alias monitor dummyMonitor
}

displayManualWindow
loadWindowPreferences




if { $tsiConfig(autorun) != 0  } { 
		
		set names [list red green yellow blue orange purple magenta black cyan]

		set i 0
		foreach a $argv {
				
				# Make the path relative to the tanksoar top-level directory, not
				# to the simulator directory.
				set aname [lindex $names $i]

				createTank -1 -1 [file join .. [file dirname $a]] [file tail $a] $aname
				tsiSendAgent $aname "watch 0"
				incr i
		}
		environmentRun
		set worldCountLimit 1400
}

proc dummyMonitor {{arg1 0} {arg2 0} {arg3 0} {arg4 0} {arg5 0} {arg6 0} {arg7 0}} {
	return
}