###
# Create Windows
###

proc initIconify {whichWindow asIcon} {

	if {$asIcon == 1} {
		wm iconify $whichWindow
	}
}


if [winfo exists .wGlobal] {destroy .wGlobal}
if [winfo exists .wControls] {destroy .wControls}
if [winfo exists .wAgentInfo] {destroy .wAgentInfo}

toplevel .wGlobal
toplevel .wControls
toplevel .wAgentInfo

wm title .wGlobal "TankSoar Map and Global Information"
wm iconname .wGlobal "Map"
wm geometry .wGlobal ${wMap_w}x${wMap_h}+${wMap_x}+${wMap_y}

wm title .wControls "Manual Controls"
wm iconname .wControls "Controls"
wm geometry .wControls +${wControls_x}+${wControls_y}

wm title .wAgentInfo "Current Tank's Status and Sensors"
wm iconname .wAgentInfo "Sensors"

initIconify .wGlobal $minimizeMap
initIconify .wAgentInfo $minimizeAgentInfo
initIconify .wControls $minimizeManual

