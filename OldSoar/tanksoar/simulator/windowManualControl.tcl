proc hSwitchRadar {whichAgent} {
	global tank radarOn actionRequest
	
	if {$tank($whichAgent,agenttype) == "soar"} {
		return
	}
	
	if {$tank($whichAgent,radarOn) == 1} {
		set actionRequest($whichAgent,activateRadar) "off"
	} else {
		set actionRequest($whichAgent,activateRadar) "on"
	}
	manualUpdate $whichAgent
}
	
proc setRadar {radarSetting} {
    global tank current radarOn maxRadar actionRequest
   
    if {($current != "") && ($tank($current,agenttype) == "human")} {
		if {($radarSetting <= $tank($current,energy))} {
			set actionRequest($current,changeRadar) $radarSetting
			.wControls.bottomframe.radar set $radarSetting
		} else {
			set actionRequest($current,changeRadar) $tank($current,energy)
			.wControls.bottomframe.radar set $tank($current,energy)
		}
		manualUpdate $current
	    refreshRadar $current
	    drawAgentInfo $current
    }
}

proc hwalk {whichAgent dir} {
    global tank actionRequest
    
    if {$tank($whichAgent,agenttype) == "human"} {
		set actionRequest($whichAgent,moveDir) $dir
		manualUpdate $whichAgent
    }   
}

proc hstrafe {whichAgent dir} {
    global tank actionRequest
    
    if {$tank($whichAgent,agenttype) == "human"} {
		set actionRequest($whichAgent,moveDir) $dir
		manualUpdate $whichAgent
    }   
}

proc hfire {whichAgent type} {
    global tank actionRequest
    
    if {$tank($whichAgent,agenttype) == "human"} {
		set actionRequest($whichAgent,fireWeapon) missile
#		set actionRequest($whichAgent,fireWeapon) $type
		manualUpdate $whichAgent
    }   
}

proc hshields {whichAgent} {
    global tank actionRequest
    
    if {$tank($whichAgent,agenttype) == "human"} {
    
		set actionRequest($whichAgent,activateShields) [toggleOn $tank($whichAgent,shields)]
		manualUpdate $whichAgent
    }   
}

proc hrotate {whichAgent dir} {
    global tank actionRequest
    
    if {$tank($whichAgent,agenttype) == "human"} {
		set actionRequest($whichAgent,rotateDir) $dir
		manualUpdate $whichAgent
    }   
}

proc hIncreaseRadar {whichAgent amount} {
	global tank
	
    if {$tank($whichAgent,agenttype) == "human"} {
		setRadar [expr ($amount + [.wControls.bottomframe.radar get])]
    }   
}
	
proc displayManualWindow {} {
global current btn_frame_h buttonColor maxRadar wControls_w tcl_platform

	frame .wControls.frame1  
	frame .wControls.frame2  
	frame .wControls.frame3  
	button .wControls.frame1.moveup  -bitmap @./images/buttons/up.XBM \
		-relief sunken \
		-command {if {$current != ""} {hwalk $current forward}} -background $buttonColor
	button .wControls.frame3.movedown  -bitmap @./images/buttons/down.XBM \
		-relief sunken \
		-command {if {$current != ""} {hwalk $current backward}} -background $buttonColor
	button .wControls.frame2.moveleft  -bitmap @./images/buttons/left.XBM \
		-relief sunken \
		-command {if {$current != ""} {hstrafe $current left}} -background $buttonColor
	button .wControls.frame2.moveright  -bitmap @./images/buttons/right.XBM \
		-relief sunken \
		-command {if {$current != ""} {hstrafe $current right}} -background $buttonColor
	
	button .wControls.frame3.fire1  -bitmap @./images/buttons/fire.XBM \
		-relief sunken -command {if {$current != ""} {hfire $current missile}} -background $buttonColor
	button .wControls.frame3.fire2  -bitmap @./images/buttons/fire.XBM \
		-relief sunken -command {if {$current != ""} {hfire $current mine}} -background $buttonColor
	button .wControls.frame1.clockwise  -bitmap @./images/buttons/clockwise.XBM \
		-relief sunken -command {if {$current != ""} {hrotate $current right}} -background $buttonColor
	button .wControls.frame1.counterclockwise  -bitmap @./images/buttons/counterclockwise.XBM \
		-command {if {$current != ""} {hrotate $current left}} -relief sunken -background $buttonColor
	button .wControls.frame2.shields  -bitmap @./images/buttons/shields.XBM \
		-relief sunken -command {if {$current != ""} {hshields $current}} -background $buttonColor

	frame .wControls.radarframe -height [expr $btn_frame_h * 2]
	checkbutton .wControls.radarframe.radartoggle -text "Radar" -variable radarOn \
		-font "-*-helvetica-*-r-*-*-*-*-*-*-*-*-*-*" \
		-command {if {$current != ""} {hSwitchRadar $current}} -background $buttonColor

	frame .wControls.bottomframe -height [expr $btn_frame_h * 2]
	scale .wControls.bottomframe.radar \
		-from 0 -to $maxRadar -orient horizontal \
		-font "-*-helvetica-*-r-*-*-*-*-*-*-*-*-*-*" \
		-command setRadar -background $buttonColor -relief raised
	.wControls.bottomframe.radar set 1
	
	pack .wControls.frame1 -side top -fill x
	pack .wControls.frame1.counterclockwise .wControls.frame1.moveup \
		.wControls.frame1.clockwise -side left -fill x -expand 1
	pack .wControls.frame2 -side top -fill x
	pack .wControls.frame2.moveleft .wControls.frame2.shields \
		.wControls.frame2.moveright -side left -fill x -expand 1
	pack .wControls.frame3 -side top -fill x
	pack .wControls.frame3.fire1 .wControls.frame3.movedown \
		.wControls.frame3.fire2 -side left -fill x -expand 1

	pack .wControls.radarframe -side top -fill x
	pack .wControls.radarframe.radartoggle \
		-side left -fill both -expand 1
	
	pack .wControls.bottomframe -side top -fill x
	pack .wControls.bottomframe.radar -side top -fill both -expand 1

#>-=>
#  Bind keys for human movement
#>-=>
	foreach thisWin {.wGlobal .wControls .wAgentInfo .} {
		bind $thisWin <Key-Up> {if {$current != ""} {hwalk $current forward}}
		bind $thisWin <Key-Down> {if {$current != ""} {hwalk $current backward}}
		bind $thisWin <Key-Left> {if {$current != ""} {hstrafe $current left}}
		bind $thisWin <Key-Right> {if {$current != ""} {hstrafe $current right}}
		bind $thisWin <Key-8> {if {$current != ""} {hwalk $current forward}}
		bind $thisWin <Key-2> {if {$current != ""} {hwalk $current backward}}
		bind $thisWin <Key-4> {if {$current != ""} {hstrafe $current left}}
		bind $thisWin <Key-6> {if {$current != ""} {hstrafe $current right}}
		bind $thisWin <Key-7> {if {$current != ""} {hrotate $current left}}
		bind $thisWin <Key-9> {if {$current != ""} {hrotate $current right}}
		bind $thisWin <Key-5> {if {$current != ""} {hshields $current}}
		bind $thisWin <Key-0> {if {$current != ""} {hfire $current missile}}

		bind $thisWin <Key-y> {if {$current != ""} {hwalk $current forward}}
		bind $thisWin <Key-n> {if {$current != ""} {hwalk $current backward}}
		bind $thisWin <Key-g> {if {$current != ""} {hstrafe $current left}}
		bind $thisWin <Key-j> {if {$current != ""} {hstrafe $current right}}
		bind $thisWin <Key-t> {if {$current != ""} {hrotate $current left}}
		bind $thisWin <Key-u> {if {$current != ""} {hrotate $current right}}
		bind $thisWin <Key-h> {if {$current != ""} {hshields $current}}
		bind $thisWin <Key-b> {if {$current != ""} {hfire $current missile}}
		bind $thisWin <Key-m> {if {$current != ""} {hfire $current mine}}
		bind $thisWin <Key-d> {if {$current != ""} {hfire $current "mine-disarmer"}}
		bind $thisWin <Key-i> {if {$current != ""} {hIncreaseRadar $current 1}}
		bind $thisWin <Key-k> {if {$current != ""} {hIncreaseRadar $current -1}}
		bind $thisWin <Key-l> {if {$current != ""} {hSwitchRadar $current}}
		if {($tcl_platform(platform) == "macintosh")} {
		    bind $thisWin <Key-+> {if {$current != ""} {hIncreaseRadar $current 1}}
		    bind $thisWin <Key--> {if {$current != ""} {hIncreaseRadar $current -1}}
		}
	}
}
