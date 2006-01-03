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

###KJC:  Need to rewrite this to work with SMLClientInterface
###KJC:  Default for now is SIO mode is -off

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
   	if {[lsearch -exact [$i eval package names] tcl_sml_clientinterface] != -1} {
   		lappend finalList $i
   	}
   }
   return $finalList
}

### need to worry about having whitespace in the agent name...that
### can break things RMJ

proc createNewAgent {name {filepath ""} {filename ""}} {
   global auto_path tsi_library soar_library tsiConfig \
       localAgents tsiAgentInfo tsiSuppressMenus agentCount hideAgentWindow

   global _agent _kernel localAgentPtrs
   global smlEVENT_PRINT smlEVENT_AFTER_PRODUCTION_FIRED smlEVENT_AFTER_PHASE_EXECUTED 
   global smlEVENT_XML_TRACE_OUTPUT 
   global smlEVENT_BEFORE_RUN_STARTS smlEVENT_AFTER_RUN_ENDS

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
      error "Cannot create tcl_interp named '$name'"
   }

   $name eval [list set auto_path $auto_path]
   load {} Tk $name
   $name eval [list set soar_library $soar_library]
   $name eval [list set tsi_library $tsi_library]
   $name eval [list array set tsiSuppressMenus [array get tsiSuppressMenus]]
   if [info exists soar_doc_dir] {
      $name eval [list set soar_doc_dir $soar_doc_dir]
   }

if {0} {
   ### KJC moved code to section after Soar loaded and cmds source'd
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
   $name eval tsiInitAgent  ; # generates TSI Agent Window if !hideAgentWin
   $name alias tsiLoadAgentSource tsiLoadAgentSource $name

   $name eval [list set version 8.6.0]
}

   $name eval [list set _kernel $_kernel]
   $name eval [list package require tcl_sml_clientinterface]

   # ask the kernel to create an agent for the new interp 
   if [catch "$name eval [list set _agent [$_kernel CreateAgent $name]]"] {
      error "Cannot create soar_agent named '$name'"
   }

    set _agent [$name eval [list set _agent]]
    set localAgentPtrs($name) [$name eval [list set _agent]]
   
   ###KJC  each agent needs its SML pointers to access Soar
   $name alias soar_agent  $_agent
   $name alias soar_kernel $_kernel

   if [info exists localAgents] { 
       lappend localAgents $name 
   } else {
       set localAgents $name
   }

if {[llength [array names localAgentPtrs]] == 1} {
       ## this is the first agent, so can launch Java Debugger
       tsiLaunchJavaDebugger $name
   }
   
  #now wait for the agent to report that it is ready
  $_kernel GetAllConnectionInfo
  while {[expr {[$_kernel GetAgentStatus java-debugger] ne "ready"}]} {
	  set delay 0
	  after 100 set delay [expr $delay + 1]
	  vwait delay
	  $_kernel GetAllConnectionInfo
        }
   

   # Register for the agent events
 
if (0) {
   $name eval [list set printCallbackId [$_agent RegisterForPrintEvent $smlEVENT_PRINT PrintCallback ""]]
    #$name eval [list set productionCallbackId [$_agent RegisterForProductionEvent $smlEVENT_BEFORE_PRODUCTION_REMOVED ProductionExcisedCallback ""]]
   $name eval [list set productionCallbackId [$_agent RegisterForProductionEvent $smlEVENT_AFTER_PRODUCTION_FIRED ProductionFiredCallback ""]]
   $name eval [list set runCallbackId [$_agent RegisterForRunEvent $smlEVENT_AFTER_PHASE_EXECUTED PhaseExecutedCallback ""]]
   $name eval [list set structuredCallbackId [$_agent RegisterForXMLEvent $smlEVENT_XML_TRACE_OUTPUT StructuredTraceCallback ""]]
   #registering this next callback will supplant normal output processing, causing things to not function properly
   #it is provided merely for illustration
   $name eval [list set outputCallbackId [$_agent AddOutputHandler "move" OutputCallback ""]]
}
   # these events let us know when to set the runningSimulation and smlStopNow global variables
   # we really should use system-level events instead of agent-level events (so the events don't fire for each agent),
   #  but my attempts to do that haven't worked out
   $name eval [list set beforeRunStartsCallbackId [$_agent RegisterForRunEvent $smlEVENT_BEFORE_RUN_STARTS SMLenvironmentRunEvent ""]]
   $name eval [list set beforeRunStartsCallbackId [$_agent RegisterForRunEvent $smlEVENT_AFTER_RUN_ENDS SMLenvironmentStopEvent ""]]
   
   tsiSetupAgentVars $name

   # load procs so Users can enter Soar cmds in agent window
    $name eval [list source "[file join $tsi_library tsiSoarCmds.tcl]"] 

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
   $name eval [list set version 8.6.0]

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
	    ###KJC if [catch "$name eval [list uplevel #0 source $tsiAgentInfo($name,sourceFile)]" msg] 
	    if [catch "$name eval [list soar_agent LoadProductions $tsiAgentInfo($name,sourceFile)]" msg] {
		
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
   tsiSendAgent [lindex $a 0] "soar_kernel ExecuteCommandLine StopAllAgents"
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

proc tsiLaunchJavaDebugger {name} {
  global tsiConfig soar_library tcl_platform env _kernel

  ## only launch a Java debugger if not using tsiAgentWindow
  if !($tsiConfig(hideAgentWindow)) {
    return
  }
  switch -exact $tcl_platform(platform) {
    windows {
       $name eval [list exec javaw -Xmx512m -jar [file join $soar_library SoarJavaDebugger.jar] -remote & ]}
    unix {
       if {($tcl_platform(os) == "Darwin")} {
           $name eval [list exec ./java_swt -classpath swt-carbon.jar:swt-pi-carbon.jar:sml.jar:SoarJavaDebugger.jar -Djava.library.path=/Applications/Soar-8.6.2/soar-library:/Applications/Soar-8.6.2/lib -Dorg.eclipse.swt.internal.carbon.smallFonts debugger.Application -remote & ]
       } else {
           $name eval [list exec java -Xmx512m -jar [file join $soar_library SoarJavaDebugger.jar] -remote & ]}}
  }
  
  #now wait for the debugger to report that it is ready
  $_kernel GetAllConnectionInfo
  #puts "debugger status = [$_kernel GetConnectionStatus java-debugger]"
  while {[expr {[$_kernel GetConnectionStatus java-debugger] ne "ready"}]} {
	  set delay 0
	  after 100 set delay [expr $delay + 1]
	  vwait delay
	  $_kernel GetAllConnectionInfo
	  #puts "debugger status = [$_kernel GetConnectionStatus java-debugger]"
        }
  
}

