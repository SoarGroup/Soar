#>-=>
# Intialize Tcl Soar Interface
#>-=>
set ETCPConfig(AgentName) Tank
set ETCPConfig(AgentFolder) [file join .. agents]

set mode off
if { [llength $argv] > 0 } { set mode [lindex $argv 0] }

if { ![string compare $mode client] } {

  tsi 1 -mode client -controlpanel makeTSIFileSelectionControlPanel

  wm deiconify .

} else {

  source PDFileMenus.tcl
  source et-controlpanel.tcl

  tsi 1 -mode $mode -controlpanel makeETControlPanel

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
#source windowConstants.tcl

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

proc dummyMonitor {{arg1 0} {arg2 0} {arg3 0} {arg4 0} {arg5 0} {arg6 0} {arg7 0}} {
	return
}