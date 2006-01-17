###
###
### tsiControlPanelCommon.tcl
###
###
### This file contains Control Panel helper functions which do not 
### refer to any control panel specific properties, and which are 
### thought to be useful for all (or many) controllers. 
###
### The idea here is that the control panel is segmented into 2 files,
### tsiControlPanelDefault.tcl and tsiControlPanelCommon.tcl
### tsiControlPanelDefault might be replaced with an application-
### specific control panel, but these helper functions should
### still be useful and save some code writing.  NOTE:  any
### new control panel definition MUST include:
###		proc registerAgent {name}
###		proc destroyAgent {name}


proc tsiSetControlPanelSIOMode { topFrame } {
    global tsiConfig tsi_library

  # Now we need to do the specialized setup for each particular mode
    if { ![string compare $tsiConfig(mode) client] } {
      # Client mode
      uplevel #0 [list source [file join $tsi_library tsiClient.tcl]]
      tsiInitClientControlPanel $tsiConfig(socketioHost) \
	  $tsiConfig(socketioPort) $topFrame

  } elseif { ![string compare $tsiConfig(mode) server] } {
      # Server mode
      uplevel #0 [list source [file join $tsi_library tsiServer.tcl]]
      tsiInitServerControlPanel $tsiConfig(socketioPort) $topFrame

  } else {
      # Socketio Off
      uplevel #0 [list source [file join $tsi_library tsiLocal.tcl]]
  }
}

proc tsiListAgents {} {
# changed b/c can't assume that all slave interps are Soar
#  - mazin	12/3/98

   if [catch {set x [interp slaves]} m] {
      error "Cannot compute list of agents: $m"
   }
   
   set finalList [list]
   foreach i $x {
   	if {[lsearch -exact [$i eval package names] Soar] != -1} {
   		lappend finalList $i
   	}
   }
   return $finalList
}

### need to worry about having whitespace in the agent name...that
### can break things RMJ

proc createNewAgent {name {filepath ""} {filename ""}} {
   global auto_path tsi_library soar_library soar_doc_dir tsiConfig \
       localAgents tsiAgentInfo tsiSuppressMenus agentCount

   puts "CreateNewAgent --> $name $filepath $filename"

   if {$name == ""} {
       incr agentCount
       set name "soar$agentCount"
   }

   if {$filename == "**NO FILES**.soar"} {
      tk_dialog .error {No File Specified} "Please specify an agent name." \
                error 0 Dismiss
      return
   }

   if [catch "interp create $name"] {
      error "Cannot create agent named '$name'"
   }

   if [info exists localAgents] { 
       lappend localAgents $name 
   } else {
       set localAgents $name
   }

   #load {} Tk $name
   #load tk83.dll Tk $name
   $name eval [list set auto_path $auto_path]
   if [catch "$name eval {package require Tk}"] {
	load {} Tk $name
   }
   $name eval {package require Soar}
   $name eval [list set soar_library $soar_library]
   $name eval [list set tsi_library $tsi_library]
   $name eval [list array set tsiSuppressMenus [array get tsiSuppressMenus]]
   if [info exists soar_doc_dir] {
      $name eval [list set soar_doc_dir $soar_doc_dir]
   }
   ### Configure the agent for the TSI
   set id [array startsearch tsiConfig]
   while {[array anymore tsiConfig $id]} {
      set elm [array nextelement tsiConfig $id]
      $name eval [list set tsiConfig($elm) $tsiConfig($elm)]
   }
   array donesearch tsiConfig $id
   $name eval [list set tcl_interactive 1]  ;# want command completion etc...
   $name eval [list set interp_name $name]
   $name eval [list set tsiAgentInfo($name,sourceDir) $filepath]
   $name eval [list set tsiAgentInfo($name,sourceFile) $filename]
   $name alias registerWithController registerAgent $name
   $name alias tsiListAgents tsiListAgents
   $name eval tsiInitAgent
   $name alias tsiLoadAgentSource tsiLoadAgentSource $name

   tsiSetupAgentVars $name

   set tsiAgentInfo($name,sourceDir) "$filepath"
   set tsiAgentInfo($name,sourceFile) "$filename"

   tsiLoadAgentSource $name
}  ;### end createNewAgent

proc tsiLoadAgentSource {name} {
    global tsiAgentInfo
    
    if { [string compare $tsiAgentInfo($name,sourceDir) ""] != 0 } {
				set initialPath "[pwd]"
				if [catch {cd "$tsiAgentInfo($name,sourceDir)"} msg] {
						error "Could not change to directory '$tsiAgentInfo($name,sourceDir)'"
						return
				}
				if { [string last ".rete" $tsiAgentInfo($name,sourceFile)] != -1 } {
						
						puts "Sending command 'rete-net -load $tsiAgentInfo($name,sourceFile)'"
						tsiSendAgent $name "rete-net -load $tsiAgentInfo($name,sourceFile)"

				} else {
						if [catch "$name eval [list uplevel #0 source $tsiAgentInfo($name,sourceFile)]" msg] {
						
								tk_dialog .error Warning \
										"Couldn't load agent code for $name from file '$tsiAgentInfo($name,sourceFile): $msg" warning 0 Ok
						}
				}
				
				if [catch {cd "$initialPath"} msg] {
						error "Could not change to directory '$initialPath'"
						return
				}
    }
		
    $name eval [list Debugger::UpdateSourceReloaded]
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
	    puts "Adding $i"
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
    cd [file join $tsiSimulatorPath $newDir]

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

    if { $names == {} } {
	
	if [winfo exists $menuName] {
	    [$menuName cget -menu] delete 0 end
	    [$menuName cget -menu] add command -label {*NONE*} -command {}
	} else {
	    tk_optionMenu $menuName {*NONE*}
	    [$menuName cget -menu] entryconfigure {*NONE*} -command {}
	}
	$menuName configure -state disabled
	set currentName {}
    } else {
	if [winfo exists $menuName] {
	    [$menuName cget -menu] delete 0 end
	    foreach i $names {
		[$menuName cget -menu] add command -label $i \
		    -command [list set tsiCurrentAgentSourceFile $i]
	    }
	    if { ![info exists currentName] || \
		     ([lsearch $names $tsiCurrentAgentSourceFile] < 0)} {
		set tsiCurrentAgentSourceFile [lindex $names 0]
	    } 
	    $menuName configure -state normal
	} else {
	    eval tk_optionMenu $menuName tsiCurrentAgentSourceFile $names
	}
    }
}

    

proc tsiSendAgent {agent command} {
   ### Need to use {} for catch, or $command gets concat'ed into a list
   if [catch {$agent eval $command} msg] {
      error "Error sending command '$command' to agent $agent!"
   }
}

### New version using slave interpreters in Soar 7.1
proc sendAllAgents {msg} {
   foreach i [tsiListAgents] {
      if [catch {$i eval $msg} m] {
         error "Error sending message $msg to agent $i: $m"
      }
   }
}


proc agentToggleCommand {name t on off} {
   if $t {
      tsiSendAgent $name $on
   } else {
      tsiSendAgent $name $off
   }
}

proc toggleWindow {win} {
   if {[string compare [wm state $win] normal] == 0 } {
      wm withdraw $win
   } else {
      wm deiconify $win
   }
}

### Need a tsiStop-Soar, because stop-soar is not defined in wish
### interpreters
proc tsiStop-soar {} {
   set a [tsiListAgents]
   if {$a == {}} {
      tk_dialog .error {Error} \
                       {Can't stop!  No Soar interpreters are running!}\
                error 0 Dismiss
      return
   }
   tsiSendAgent [lindex $a 0] stop-soar
}

proc tsiRun-soar {} {
   set a [tsiListAgents]
   if {$a == {}} {
      tk_dialog .error {Error} \
                       {Can't run!  No Soar interpreters exist!}\
                error 0 Dismiss
      return
   }
   tsiSendAgent [lindex $a 0] run
}
proc tsiStep-soar {} {
   set a [tsiListAgents]
   if {$a == {}} {
      tk_dialog .error {Error} \
                       {Can't step!  No Soar interpreters exist!}\
                error 0 Dismiss
      return
   }
   tsiSendAgent [lindex $a 0] "run 1"
}


proc temploadWindowPreferences {} {
    global soar_library windowPrefs

    if {[file exists [file join $soar_library tsiWindowPrefs.data]]} {
	set chanID [open [file join $soar_library tsiWindowPrefs.data] r]
	
	gets $chanID geometryRead
	wm geometry . $geometryRead
	while {![eof $chanID]} {
	    gets $chanID winLine
	    if {$winLine != ""} {
		# Window preferences are stored as windowPrefs(type,name)=geometry
		# type is either main or tsi, name is either window name or
		# agent name(for tsi type)
		set windowPrefs([lindex $winLine 2],[lindex $winLine 0]) [lindex $winLine 1]
		if {[lindex $winLine 2] == "main"} {
		    if {[winfo exists [lindex $winLine 0]]} {
			wm geometry [lindex $winLine 0] [lindex $winLine 1]
		    }
		} elseif {[interp exists [lindex $winLine 0]] &&\
			([lsearch -exact [[lindex $winLine 0] eval [list package names]] {Tk}] != -1)} {
		    [lindex $winLine 0] eval wm geometry .tsw [lindex $winLine 1]
		}
	    }
	}
	close $chanID
	return 1
    } else {
	return 0
    }
}

proc tempsaveWindowPreferences {} {
    global soar_library windowPrefs
    
    set chanID [open [file join $soar_library tsiWindowPrefs.data] w]
    puts $chanID [wm geometry .]
    
    foreach w [winfo children .] {
	if {[winfo toplevel $w] == $w} {
	    puts $chanID "$w [wm geometry $w] main"
	    set windowPrefs(main,$w) [wm geometry $w]
	}
    }

    foreach i [interp slaves] {
	if {([lsearch -exact [$i eval [list package names]] {Tk}] != -1) && \
		[$i eval winfo exists .tsw]} {
	    set windowPrefs(tsi,$i) [$i eval wm geometry .tsw]
	    puts $chanID "$i $windowPrefs(tsi,$i) tsi"
	}
    }
    close $chanID
}
   
proc stopAfterDecision {} {
    sendAllAgents {monitor -add after-decision-phase-cycle "stop-soar -self " dp1}
}

proc dontStopAfterDecision {} {
    sendAllAgents {monitor -delete after-decision-phase-cycle dp1}
}


