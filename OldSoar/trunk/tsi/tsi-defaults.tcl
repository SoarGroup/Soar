#puts "Loading tsi-defaults.tcl"

## set tsiConfig(debug) to 1 when debugging, 0 when not
set tsiConfig(debug) 0

## if expertise is 1, it is the full interface,
## if 0, it is for very very first time users
## - Note:  expertise suppress certain menu items:
## -- Saving and loading production memory
## -- Print operator stack
## -- Listing the agents to console
## -- Printing out all command, variable and proc names defined to console
## -- Printing out attribute preference mode to console
## -- Printing out O-support mode to console
## -- Saving backtraces
## -- Eight-puzzle, critter world and GUI demos
## -- Add-wme and Select-operator items
## -- Tcl and Tk help menus
set tsiConfig(expertise) 1

if {$tcl_version >= 8.0} {
	 if {$tcl_platform(platform) == {unix}} {
		  set tsiConfig(normalFont) {courier 12 normal}
		  set tsiConfig(boldFont)   {courier 12 bold}
		  set tsiConfig(italicFont) {courier 12 italic}
		  set tsiConfig(helpFont)   {courier 12 bold}
		  set tsiConfig(searchTextFont) {courier 12 normal}
		  set tsiConfig(searchButtonsFont) {system 12}
        set tsiConfig(tsiFont) {courier 12 normal}
        set tsiConfig(buttonFont) {courier 12 normal}
        set tsiConfig(menuFont) {courier 12 normal}
        set tsiConfig(dialogFont) {system 12}
        set tsiConfig(dialogDefaultTextFont) {system 12}
        set tsiConfig(smallFont) {fixed 12}
	 } else {
		  set tsiConfig(normalFont) {courier 10 normal}
		  set tsiConfig(boldFont)   {courier 10 bold}
		  set tsiConfig(italicFont) {courier 10 italic}
		  set tsiConfig(helpFont)   {courier 10 bold}
		  set tsiConfig(searchTextFont) {courier 10 normal}
		  set tsiConfig(searchButtonsFont) {system 10}
        set tsiConfig(tsiFont) {courier 10 normal}

        set tsiConfig(buttonFont) {courier 10 normal}
        set tsiConfig(menuFont) {courier 10 normal}
        set tsiConfig(dialogFont) {system 10}
        set tsiConfig(dialogDefaultTextFont) {system 10}
        set tsiConfig(smallFont) {fixed 10}
	 }   
} else {
	 if {$tcl_platform(platform) == {unix}} {
		  set tsiConfig(normalFont) {*-Courier-Medium-R-Normal-*-120-*}
		  set tsiConfig(boldFont) {*-Courier-Bold-R-Normal-*-120-*}
		  set tsiConfig(italicFont) {*-Courier-Medium-O-Normal-*-120-*}
		  set tsiConfig(helpFont) {*-Courier-Bold-O-Normal-*-140-*}
        set tsiConfig(buttonFont) {*-Courier-Medium-R-Normal-*-120-*}
        set tsiConfig(menuFont) {*-Courier-Medium-R-Normal-*-120-*}
        set tsiConfig(dialogFont) {*-Courier-Medium-R-Normal-*-120-*}
        set tsiConfig(dialogDefaultTextFont) {*-Courier-Medium-R-Normal-*-120-*}
        set tsiConfig(smallFont) {*-Courier-Medium-R-Normal-*-80-*}
	 } else {
		  set tsiConfig(normalFont) {-*-Courier-Medium-R-Normal--*-120-*-*-*-*-*-*}
		  set tsiConfig(boldFont) {-*-Courier-Bold-R-Normal--*-120-*-*-*-*-*-*}
		  set tsiConfig(italicFont) {-*-Courier-Medium-O-Normal--*-120-*-*-*-*-*-*}
		  set tsiConfig(helpFont) {-*-Courier-Bold-O-Normal--*-120-*-*-*-*-*-*}
        set tsiConfig(buttonFont) {*-Courier-Medium-R-Normal-*-120-*}
        set tsiConfig(menuFont) {*-Courier-Medium-R-Normal-*-120-*}
        set tsiConfig(dialogFont) {*-Courier-Medium-R-Normal-*-120-*}
        set tsiConfig(dialogDefaultTextFont) {*-Courier-Medium-R-Normal-*-120-*}
        set tsiConfig(smallFont) {*-Courier-Medium-R-Normal-*-80-*}
	 }
	 set tsiConfig(searchTextFont) {8x13}
	 set tsiConfig(searchButtonsFont) {8x13bold}
}

set tsiConfig(promptConfigure) "-font \{$tsiConfig(boldFont)\}"
set tsiConfig(userTextConfigure) {-underline 1}
set tsiConfig(searchTextConfigure) {-background lightblue}

if {$tcl_version >= 8.0} {
	 #set tsiConfig(ControlPanelX) -20
	 set tsiConfig(ControlPanelX) [expr [winfo screenwidth .] - 375]
} else {
	 set tsiConfig(ControlPanelX) [expr [winfo screenwidth .] - 460]
}
set tsiConfig(ControlPanelY)  20
set tsiConfig(AgentWindowX)   0
if {$tcl_platform(platform) == {windows}} {
    set tsiConfig(AgentWindowY)   -30
} else {
    set tsiConfig(AgentWindowY)  [expr [winfo screenheight .] - 400]
}
set tsiConfig(AgentWindowDeltaX) 40
set tsiConfig(AgentWindowDeltaY)  0
set tsiConfig(window_count) 0

# Socketio Mode (client/server/off)
set tsiConfig(mode) off
set tsiConfig(socketioHost) ""
set tsiConfig(socketioPort) 3456 
set tsiConfig(calibrateClock) 3

set tsiConfig(sioDebug) 0
set tsiConfig(sioWatch) 0
set tsiConfig(manageRemoteAgentOutputFN) manageRemoteAgentOutput
set tsiConfig(actOnRemoteAgentOutputFN) actOnRemoteAgentOutput
set tsiConfig(sioTimeOut) -1

set tsiConfig(interfaceType) 2
set tsiConfig(hasEnvironment) 0

# max number of user prods to display in pulldownMenu
# zero means don't limit it at all
set tsiConfig(maxUserProdMenu) 0
