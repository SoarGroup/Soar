###
### $Id$
###
### $Log$
### Revision 1.4  2005/06/10 04:13:49  kcoulter
### cleanup -- removed some puts
###
### Revision 1.3  2005/06/10 03:47:19  kcoulter
### converted to SML events
###
### Revision 1.2  2005/06/07 17:00:10  kcoulter
### fixed DestroyAgents, other SML compat
###
### Revision 1.1  2005/06/01 19:14:13  rmarinie
### initial commit of sml eaters
###
### Revision 1.4  2004/07/12 15:25:56  rmarinie
### fixed bugzilla bug 391
###
### Revision 1.3  2004/07/12 14:45:30  toolshed
### fixed erroneous stop after decision cycle monitor command
###
### Revision 1.2  2003/10/21 18:24:55  snason
### uncommented the restart map button
###
### Revision 1.1.1.1  2003/06/16 13:48:36  swallace
### eaters initial cvs version (3.0.5)
###
### Revision 1.6  1998/10/26 15:48:50  swallace
### Prior to Release for 494
###
### Revision 1.5  1998/10/25 14:52:30  swallace
### Synced w/ Mazin
###
### Revision 1.4  1998/10/08 13:54:45  swallace
### Released to John
###
### 
###
###
###
###

set possibleAgentColors [list red blue green yellow purple orange black]

global tsiSimulatorPath
set tsiSimulatorPath $ETCPConfig(SimulatorPath)
#puts "in et-controlpanel.tcl: tsiSimulatorPath = $tsiSimulatorPath"

source [file join $tsi_library tsiControlPanelCommon.tcl]

 
proc makeETControlPanel { {hide 0} {x -20} {y 1} } {
   global agentColorMenu possibleAgentColors currentAgentColor tsiConfig \
       ETCPConfig soarTimeUnit tsiSimulatorPath

#puts "in makeETControlPanel: tsiSimulatorPath = $tsiSimulatorPath"

   

    InstantiateETCPConfig

   if $hide {
      wm withdraw .
   } else {
      wm deiconify .
   }
   . configure -relief ridge -borderwidth 5
   wm title . "$ETCPConfig(AgentName) Control Panel"
   if {$x >= 0} { set x +$x }
   if {$y >= 0} { set y +$y }
   wm geometry . $x$y
   
   # ----------------------------------------------------------------------
   #				Main Menu
   # ----------------------------------------------------------------------

   frame .menu -relief raised -bd 2
   pack .menu -side top -fill x
   
   menubutton .menu.file -text {File} -menu .menu.file.m
   menu .menu.file.m 


   # .menu.file.m add command -label "Reload $ETCPConfig(PluralAgentName)"
   .menu.file.m add command -label Exit -command quitSoar
   
   pack .menu.file -side left -padx 5


   # ----------------------------------------------------------------------
   #				Buttons, etc
   # ----------------------------------------------------------------------

 
    frame .socketio 
    pack .socketio -side bottom -fill x
  
 #  menubutton .destroyAgent -relief raised -text {Destroy Agent} \
  \#   -menu .destroyAgent.m
  # menu .destroyAgent.m
   

   #pack .destroyAgent -side bottom -padx 5
   # create a frame for the general buttons
   
   # BUG BUG --------- These need to be changed to agree w/ my stuff
   # in tsi24beta tsiControlPanel
   frame .run -relief sunken -borderwidth 2
   #button .run.step -text Step -command environmentStep 
   #button .run.run -text Run -command environmentRun
   #button .run.stop -text Stop -command environmentStop
   button .run.step -text Step -command SMLenvironmentStep 
   button .run.run -text Run -command SMLenvironmentRun
   button .run.stop -text Stop -command SMLenvironmentStop
   button .run.quit -text Quit -command quitSoar
   
   # pack the frame, and its components
   pack .run.step .run.run .run.stop .run.quit -side top -anchor n -fill both
   pack .run -side right
   
  
 
   # create the agent frame and control for maing new agents.
   frame .agentcreation -borderwidth 2

   label .agentcreation.lPath -text {Path:} -width 6
   label .agentcreation.lFile -text {File:} -width 6

   PDFileMenus .agentcreation.file tsiCurrentAgentSourceFile \
       -pathMenu .agentcreation.path -pathVar tsiCurrentAgentSourceDir \
       -maxDepth 3 -path $ETCPConfig(AgentFolder) \
       -filter {*.soar} -filenameDisplay root

   grid .agentcreation.lPath .agentcreation.path
   grid .agentcreation.lFile .agentcreation.file
   grid .agentcreation.lPath -sticky w
   grid .agentcreation.path -sticky ew -padx 5
   grid .agentcreation.lFile -sticky w
   grid .agentcreation.file -sticky ew -padx 5

   label .agentcreation.lColor -text {Color:} -width 6
   set agentColorMenu [eval tk_optionMenu .agentcreation.color \
			      currentAgentColor $possibleAgentColors]
      
   foreach i $possibleAgentColors {
      $agentColorMenu entryconfigure $i -command \
	  "[list set currentAgentColor $i]; updateAvailableAgentColors"
   }
   
   grid .agentcreation.lColor .agentcreation.color
   grid .agentcreation.lColor -sticky w
   grid .agentcreation.color -sticky ew -padx 5

    menubutton .agentcreation.destroyAgent -relief raised \
       -text "Destroy $ETCPConfig(AgentName)" -menu \
       .agentcreation.destroyAgent.m -activebackground red 
    menu .agentcreation.destroyAgent.m
   
    button .agentcreation.createAgent -text "Create $ETCPConfig(AgentName)" \
       -command "createNewAgent $currentAgentColor" -activebackground green

   #grid .agentcreation.destroyAgent .agentcreation.createAgent 
    grid .agentcreation.createAgent -column 1 -sticky ew -padx 5   
    grid .agentcreation.destroyAgent -column 1 -sticky ew -padx 5
   

   
   # Force the column with the pull down mennus to be at least 100 pixels wide
   grid columnconfigure .agentcreation 1 -minsize 100 


   pack .agentcreation -side left

   # ----------------------------------------------------------------------
   #				Specialized Menus
   # ----------------------------------------------------------------------


    menubutton .menu.agents -text $ETCPConfig(PluralAgentName) \
	     -menu .menu.agents.m
    menu .menu.agents.m
   .menu.agents.m add command -label \
	     "Destroy All $ETCPConfig(PluralAgentName)" -command \
	     {destroyAllAgents}

 

   .menu.agents.m add checkbutton -label {Stop After Decision Phase} \
           -variable ETCPConfig(afterDecision) \
           -command "if \$ETCPConfig(afterDecision) { \
                        stopAfterDecision ; \
                        set ETCPConfig(runTilOutputGen) 0; \
                        set soarTimeUnit d; \
                     } else { \
                        dontStopAfterDecision \
                     };"

   .menu.agents.m add checkbutton -label {Run til Output Generated} \
           -variable ETCPConfig(runTilOutputGen) \
           -command "if \$ETCPConfig(runTilOutputGen) { \
                        set ETCPConfig(afterDecision) 0 ; \
                        dontStopAfterDecision ; \
                        set soarTimeUnit out ; \
                     } else { \
                        set soarTimeUnit d \
                     };"

   pack .menu.agents -side left -padx 5

   menubutton .menu.map -text {Map} -menu .menu.map.m
   
   menu .menu.map.m
   .menu.map.m add command -label {Load Map} -command {mapLoader}
   .menu.map.m add command -label {Restart Map} -command {restartMap}
   .menu.map.m add command -label {Random Map} -command {randomMap}
   
   pack .menu.map -side left -padx 5



   menubutton .menu.windowtoggle -text {Windows} -menu .menu.windowtoggle.m

   menu .menu.windowtoggle.m
   .menu.windowtoggle.m add command -label "Hide/Show All Windows" \
       -command {toggleAllWindows}
   .menu.windowtoggle.m add command -label {Save Window Prefs} -command \
       {saveWindowPreferences}
   .menu.windowtoggle.m add command -label {Load Window Prefs} -command \
       {loadWindowPreferences}
   .menu.windowtoggle.m add command -label {Hide/Show World Map} -command \
       {toggleWindow .wGlobal}
   .menu.windowtoggle.m add command -label "Hide/Show $ETCPConfig(AgentName) Info" \
       -command {toggleWindow .wAgentInfo}
    

   pack .menu.windowtoggle -side left -padx 5

    return .socketio


}
   
proc stopAfterDecision {} {
#    sendAllAgents {monitor -add after-decision-phase-cycle "stop-soar -self {}" dp1}
#   changed to fix bugzilla bug 391
    sendAllAgents {monitor -add after-decision-phase-cycle "stop-soar -self" dp1}
}
proc dontStopAfterDecision {} {
    sendAllAgents {monitor -delete after-decision-phase-cycle dp1}
}


proc registerAgent {name} {
   global showAgent ETCPConfig tsiAgentInfo
   
   .menu.agents.m add cascade -label $name -menu .menu.agents.m.$name
   menu .menu.agents.m.$name

   .menu.agents.m.$name add checkbutton -label \
       "Show $ETCPConfig(AgentName) Window" -variable showAgent($name) \
       -command "agentToggleCommand $name \$showAgent($name) \
                  {wm deiconify .tsw} {wm withdraw .tsw}"
   set showAgent($name) 1
   .menu.agents.m.$name add command -label \
       "Raise $ETCPConfig(AgentName) Window" \
       -command "tsiSendAgent $name {raise .tsw}"
   .menu.agents.m.$name add command -label {Reload Productions} \
       -command "tsiLoadAgentSource $name"
   .menu.agents.m.$name add command -label "Reset $ETCPConfig(AgentName)" \
       -command "tsiSendAgent $name {tsiDisplayAndSendCommand {excise -all}}; tsiLoadAgentSource $name"
   .menu.agents.m.$name add command -label {Excise All} \
       -command "tsiSendAgent $name {tsiDisplayAndSendCommand {excise -all}}"
   .menu.agents.m.$name add command -label {Excise Chunks} \
       -command "tsiSendAgent $name {tsiDisplayAndSendCommand {excise -chunks}}"

   .agentcreation.destroyAgent.m add command -label $name \
       -command [list destroyAgent $name]
  
    #set tsiAgentInfo($name,addFN) localAgentAddWME
    #set tsiAgentInfo($name,rmFN) localAgentRemoveWME

    set tsiAgentInfo($name,addFN) eatersAgentAddWME
    set tsiAgentInfo($name,rmFN)  eatersAgentRemoveWME
    
}
   

proc destroyAllAgents {} {
global tankList

   foreach name [tsiListAgents] {
	destroyAgent $name
   }
   # Only defined in tankSoar
   if {[info exists tankList] && ($tankList != {})} {
      foreach name $tankList {
         destroyAgent $name
      }
   }
}

proc destroyAgent {name} {
    global localAgents localAgentPtrs

	if {[info proc envDestroyAgent] == ""} {
		error "Note that you should create a 'envDestroyAgent' function to clean up stuff in your environment."
	} else {
		envDestroyAgent $name
	}
      ##puts "this is the proc that destroys eaters"
   
      #delete the agent object 
      set result [$name eval [list soar_kernel DestroyAgent $localAgentPtrs($name)]];
      # Give Tcl object ownership of underlying C++ object so when we 
      # delete the Tcl object, they both get deleted
      set result [$name eval [list soar_agent -acquire]]
      set result [$name eval [list soar_agent -delete]]
      #don't leave bad pointers around
      if [$name eval [list info exists soar_agent]]  {
	  [$name eval [list unset soar_agent]]
      }
  
   if {[lsearch [tsiListAgents] $name] >= 0} {
      if [catch "destroy-interpreter $name"] {
         if [catch "interp delete $name"] {
            error "Cannot delete agent '$name'!"
         	return
         }
      }
   }
	if { ![catch ".menu.agents.m index $name"] } {
		.menu.agents.m delete $name
		destroy .menu.agents.m.$name
	}

    .agentcreation.destroyAgent.m delete $name
   
    set localAgents [ldelete $localAgents $name]
    unset localAgentPtrs($name)

}



proc destroyRemoteAgent {name} { }



proc registerRemoteAgent {name socket} {
   global showAgent sio_agentStatus sio_socketToAgent sio_agentToSocket \
      tsiAgentInfo tsiConfig ETCPConfig

   
    if { $tsiConfig(sioDebug) > 3 } {
	puts "registerRemoteAgent: et-controlpanel  (begin) -- name: $name socket: $socket"
    }




  .menu.agents.m add cascade -label $name -menu .menu.agents.m.$name
   menu .menu.agents.m.$name

   .menu.agents.m.$name add checkbutton -label \
       "Show $ETCPConfig(AgentName) Window" -variable showAgent($name) \
       -command "agentToggleCommand $name \$showAgent($name) \
                  {wm deiconify .tsw} {wm withdraw .tsw}"
   set showAgent($name) 1
   .menu.agents.m.$name add command -label \
       "Raise $ETCPConfig(AgentName) Window" \
       -command "tsiSendAgent $name {raise .tsw}"
    .menu.agents.m.$name add checkbutton -label {Online} \
       -variable sio_agentStatus($name,online)
   .menu.agents.m.$name add command -label {Excise All} \
       -command "tsiSendAgent $name {tsiDisplayAndSendCommand {excise -all}}"
   .menu.agents.m.$name add command -label {Excise Chunks} \
       -command "tsiSendAgent $name {tsiDisplayAndSendCommand {excise -chunks}}"
   .agentcreation.destroyAgent.m add command -label $name \
       -command [list destroyRemoteAgent $name]
  
    set tsiAgentInfo($name,addFN) localAgentAddWME
    set tsiAgentInfo($name,rmFN) localAgentRemoveWME
    
   if { [info exists sio_agentToSocket($name)] } {
      error "In registerRemoteAgent: the agent $name already exists."
   }
  
   if { [info exists sio_socketToAgent($socket)] } {
      error "In registerRemoteAgent: the socket has already been assigned."
   }
   set sio_agentToSocket($name) $socket
   set sio_socketToAgent($socket) $name
   set tsiAgentInfo($name) remote
   set tsiAgentInfo($name,addFN) remoteAgentAddWME
   set tsiAgentInfo($name,rmFN) remoteAgentRemoveWME
  
    if { $tsiConfig(sioDebug) > 3 } {
	puts "registerRemoteAgent   (end) -- name: $name socket: $socket"
    }
}




proc InstantiateETCPConfig {} {
    global ETCPConfig tsiSimulatorPath

    # since this control panel will be used with at least two simulators
    # these configuration parameter are defined (ETCPConfig) so that all
    # occurances of 'Agent' can be replaced with something more specific
    # such as 'Eater' ... so far this is the only config parameter, if enough
    # become necessary, a file could be used.

    if { ![info exists ETCPConfig(AgentName)] } { 
	set ETCPConfig(AgentName) Agent
    }
    if { ![info exists ETCPConfig(PluralAgentName)] } {
	set ETCPConfig(PluralAgentName) [format "%ss" $ETCPConfig(AgentName)]
    }
    if { ![info exists ETCPConfig(SimulatorPath)] } {
	set ETCPConfig(SimulatorPath) $tsiSimulatorPath
    }
    if { ![info exists ETCPConfig(AgentFolder)] } {
	set ETCPConfig(AgentFolder) $tsiSimulatorPath
    }
    if { ![info exists ETCPConfig(MapFolder)] } {
	set ETCPConfig(MapFolder) $tsiSimulatorPath
    }  
    if { ![info exists ETCPConfig(afterDecision)] } {
	set ETCPConfig(afterDecision) 0
    }
    if { ![info exists ETCPConfig(runTilOutputGen)] } {
	set ETCPConfig(runTilOutputGen) 1
    }  
}
    

proc tsiLoadSubDirectories { menuName fileMenuName } {
    global tsiSimulatorPath tsiCurrentAgentSourceDir

    if { $tsiSimulatorPath == [pwd] } {
	set dirs [list .]
	foreach file [glob -nocomplain *] {
	    if { [file isdirectory $file] } {
		lappend dirs [file join . [file tail $file]]
	    }
	}

    } else {
	set currDir [file join . [file tail [pwd]]]
	set dirs [list $currDir [file join $currDir ..]]
    }
       
    if [winfo exists $menuName] {
	[$menuName cget -menu] delete 0 end
	foreach i $dirs {
	    [$menuName cget -menu] add command -label $i \
		-command [list tsiChangeAgentSourceDir $i $menuName $fileMenuName]
	}

	set tsiCurrentAgentSourceDir [lindex $dirs 0]
	
    } else {
	eval tk_optionMenu $menuName tsiCurrentAgentSourceDir $dirs 
	foreach dir $dirs {
	    [$menuName cget -menu] entryconfigure $dir -command \
		[list tsiChangeAgentSourceDir $dir $menuName $fileMenuName]
	}
    }
    
}


proc tsiChangeAgentSourceDir { newDir dirMenuName fileMenuName } {
    global tsiSimulatorPath

    set oldDir [pwd]
    cd "[file join $tsiSimulatorPath $newDir]"

    if { $oldDir == [pwd] } { return }

    tsiLoadAgentNames $fileMenuName
    tsiLoadSubDirectories $dirMenuName $fileMenuName


}

    

proc tsiLoadAgentNames { menuName } {
    global tsiCurrentAgentSourceFile 

    set allNames [glob -nocomplain {*.soar}]
    set names ""
    
    foreach i $allNames {
	lappend names [file rootname [file tail $i]]
    }

    set names [lsort -ascii $names]

    if { $names == {} } {
	
	if [winfo exists $menuName] {
	    [$menuName cget -menu] delete 0 end
	   set tsiCurrentAgentSourceFile {*NO AGENTS*}
	} else {
	    tk_optionMenu $menuName tsiCurrentAgentSourceFile {*NO AGENTS*}
	}
	$menuName configure -state disabled
     } else {
	if [winfo exists $menuName] {
	   [$menuName cget -menu] delete 0 end
	   foreach i $names {
	      [$menuName cget -menu] add command -label $i \
		  -command [list set tsiCurrentAgentSourceFile $i]
	   }
	   set tsiCurrentAgentSourceFile [lindex $names 0] 
	   $menuName configure -state normal
	} else {
	    eval tk_optionMenu $menuName tsiCurrentAgentSourceFile $names
	}
    }
}

proc mapLoader {} {
    global ETCPConfig ETCP_MapFile ETCP_MapPath

    if [winfo exists .wMapLoader] { 
	wm deiconify .wMapLoader	
	return
    }
	
 #   if [info exists ETCP_MapFile] { unset ETCP_MapFile }
 #   if [info exists ETCP_MapPath] { puts "$ETCP_MapPath" ; unset ETCP_MapPath }
    
    toplevel .wMapLoader
    wm  title .wMapLoader "Load Map"

    frame .wMapLoader.topFrame

    pack .wMapLoader.topFrame -side top
    

    label .wMapLoader.topFrame.lFile -text "File :"
    label .wMapLoader.topFrame.lPath -text "Path :"
    

    PDFileMenus .wMapLoader.topFrame.file ETCP_MapFile \
	-pathMenu .wMapLoader.topFrame.path -pathVar ETCP_MapPath \
	-maxDepth 3 -path $ETCPConfig(MapFolder) \
	-filter {*.map} -filenameDisplay root    

    pack .wMapLoader.topFrame.lPath .wMapLoader.topFrame.path \
	.wMapLoader.topFrame.lFile .wMapLoader.topFrame.file \
	-side left

    frame .wMapLoader.botFrame
    pack .wMapLoader.botFrame -side bottom

    button .wMapLoader.botFrame.bLoadMap -text "Load Map" \
	-command {MapLoader_LoadMap}
    button .wMapLoader.botFrame.bReturn -text "Back To Simulation" -command \
	{wm withdraw .wMapLoader}

    pack .wMapLoader.botFrame.bLoadMap .wMapLoader.botFrame.bReturn \
	-side left
    
}

proc MapLoader_LoadMap {} {
    global ETCP_MapFile ETCP_MapPath

    set initialPath [pwd]

   #if [info proc envLoadMap] {
      puts "files: $ETCP_MapPath $ETCP_MapFile"
      envLoadMap $ETCP_MapPath $ETCP_MapFile
   #}  

    cd $initialPath

}

proc toggleWindow {whichWindow} {
   
   if [winfo exists $whichWindow] {
   	set displayed [winfo ismapped $whichWindow]
	
	   if {$displayed == 1} {
	   	wm iconify $whichWindow
	   	raise $whichWindow
	   } else {
		   wm deiconify $whichWindow
   	}
   }
}


proc toggleAllWindows {} {

	set windowList [winfo children .]
	foreach w $windowList {
		if {[winfo toplevel "$w"] == "$w"} {
			if {[winfo ismapped $w]} {
				wm iconify $w
			} else {
				wm deiconify $w
			}
		}
	}
	set slaveList [interp slaves]
	foreach s $slaveList {
		if {[$s eval [list info proc toggleAgentWindows]] != ""} {
			$s eval [list toggleAgentWindows]
		}
	}
}	
