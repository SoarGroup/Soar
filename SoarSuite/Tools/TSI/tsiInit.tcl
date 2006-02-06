##############################################################################
# 
# File            : tsiInit.tcl
# Author          : Gordon Baxter <gdb@vpsyc.psyc.nott.ac.uk>
# Created On      : 16 May 1996, 14:24:30
# 
# PURPOSE
# 
#   This is the initialization interface to the Tcl/TK Soar Interface (TSI),
#
###############################################################################
### Major contributors (in alphabetical order):
###    Mazin Assanie
###    Gordon D. Baxter
###    Karen Coulter
###    Randolph M. Jones
###    Douglas J. Pearson
###    Frank E. Ritter
###    Karl B. Schwamb
###    Glenn Taylor
###    Scott Wallace

### Modified for KernelSML April 05

# safety net in case of typos/misunderstandings
proc TSI {args} {eval tsi $args}

### Call "tsi" to start up the Tcl/Tk Soar Interface. 
###
### tsi [0|1|2|3]
###  
### The first optional argument specifies which version of the TSI is run
###  0 - Minimal TSI
###  1 - Traditional TSI
###  2 - TSI with graphical enhancements (default)
###  3 - TSI with graphical enhancements and experimental additions
### 
### Additionally, a number of flags can be set, each of which has a specified
### domain.  These flags are:
###  
###	-controlpanel	specifies the procedure which creates the 
###			controlpanel.
###	-x		the x location of the control panel
###	-y		the y location of the control panel

### KJC added for Soar 8.6 so can use Java Debugger instead of TSI agent windows
### The Agent window is still created and can be exposed if a Console is running
###    
###     -hideAgentWindow  if ==1, don't expose the agent window

###### For the first iteration with KernelSML, we only create
###### an embedded kernel using Kernel_CreateKernelInCurrentThread
###### The next three are deprecated SGIO args
###	-mode		determines the socketio mode 
###			(one of: off, client, server)
###	-host		the default host, used if mode is set to 'client'
###	-port		the connection port, used if mode is 'client' or 
###			'server'

###
###	NOTE: procedures which are used as values to the -controlpanel flag
###	should return the name of frame to which socketio controls will
###	be added when they are needed.  If the top level frame is indicated
###	the return value should be {}, otherwise the frame will be indicated
###	with .<frame-name> (e.g. .socketio)
###
### Furthermore, a number of options are set via the tsiConfig array

proc tsi {{hideController 0} args } {
   global interp_name interp_agentSocket _kernel tsi_library \
       tcl_platform tsiConfig tsiSimulationState tsiSimulatorPath \
       tsiClicksPerSecond hideAgentWindow globalTick
       
   global smlEVENT_AFTER_AGENT_CREATED smlEVENT_BEFORE_AGENT_REINITIALIZED 
   global smlEVENT_BEFORE_AGENT_DESTROYED smlEVENT_BEFORE_SHUTDOWN
   global smlEVENT_SYSTEM_START smlEVENT_SYSTEM_STOP
   global smlEVENT_AFTER_ALL_OUTPUT_PHASES EaterUpdateCallback smlStopNow

   # if we don't do this, then the agent's first output doesn't get processed
   set globalTick 0
   
   set tsiSimulatorPath $tsi_library
   
   ### Read in the default TSI parameters
   ### These *must* exist, or the TSI will not run properly
   uplevel #0 [list source [file join $tsi_library tsi-defaults.tcl]]

   ### Read in environment default parameters if they exist
   if [file exists [file join [pwd] tsi-defaults.tcl]] {
      uplevel #0 [list source [file join [pwd] tsi-defaults.tcl]]
   }
   
   ### now check the additional arguments, if any
    if { [expr ([llength $args] - 2) % 2] != 0 } { 
       error \
	   "Additional arguments to tsi must be defined with flags and values."
    }

    set tsiConfig(ControlPanelVersion) 5.0.0
    set controlPanelProc makeTSIDefaultControlPanel
    set x $tsiConfig(ControlPanelX)
    set y $tsiConfig(ControlPanelY)
    set tsiConfig(hideAgentWindow) 0

    foreach {key value} $args {
       switch -exact -- $key {
         -mode  { switch -exact $value {
	                  client   { set tsiConfig(mode) client }
	                  server   { set tsiConfig(mode) server }
	                  off      { set tsiConfig(mode) off }
	                  default  { error "Unrecognized mode '$value'" }
	               }
	      }
         -host         { set host $value }
         -port         { set port $port}
         -x            { set x $value }
         -y            { set y $value }
         -controlpanel { set controlPanelProc $value }
	 -hideagentwin { set tsiConfig(hideAgentWindow) $value }
       }
   }
 

   # auto_path should already have soar_library and tsi_library,
   # so should be able to find sml stuff
   package require tcl_sml_clientinterface

   #create an embedded kernel running in the kernel's thread (so we don't have to poll for messages)
   #set _kernel [Kernel_CreateKernelInNewThread SoarKernelSML]
   set _kernel [Kernel_CreateKernelInCurrentThread SoarKernelSML 0]

   if {[$_kernel HadError]} {
	puts "Error creating kernel: "
      puts "[$_kernel GetLastErrorDescription]\n."
	return
   }

    # DJP: We want to make sure to handle events in the Tcl thread
    # so we turn off the event thread and poll for events instead.
    #$_kernel StopEventThread
    checkForEvents $_kernel
    # DJP end

   # register the kernel callbacks
 
set agentCallbackId0 [$_kernel RegisterForAgentEvent $smlEVENT_AFTER_AGENT_CREATED AgentCreatedCallback ""]
#set agentCallbackId1 [$_kernel RegisterForAgentEvent $smlEVENT_BEFORE_AGENT_REINITIALIZED AgentReinitializedCallback ""]
#set agentCallbackId2 [$_kernel RegisterForAgentEvent $smlEVENT_BEFORE_AGENT_DESTROYED AgentDestroyedCallback ""]
#set systemCallbackId [$_kernel RegisterForSystemEvent $smlEVENT_BEFORE_SHUTDOWN SystemShutdownCallback ""]
#set rhsCallbackId [$_kernel AddRhsFunction RhsFunctionTest ""]

#we want to register for these events so we know when to set the runningSimulation and smlStopNow global vars
#however, it doesn't seem to work (probably because of some interpreter issue), so we're using agent-level events instead
#set systemCallbackId [$_kernel RegisterForSystemEvent $smlEVENT_SYSTEM_START SMLenvironmentRunEvent ""]
#set systemCallbackId [$_kernel RegisterForSystemEvent $smlEVENT_SYSTEM_STOP SMLenvironmentStopEvent ""]
 
set EaterUpdateCallback [$_kernel RegisterForUpdateEvent $smlEVENT_AFTER_ALL_OUTPUT_PHASES smlProcessUpdates ""]
  
   set smlStopNow 0
   set tsiSimulationState(running) 0
   
   if { $tsiConfig(calibrateClock) > 0 } {
      
##      puts "Calibrating System Clock: $tsiConfig(calibrateClock) seconds please..."
      set ms [expr $tsiConfig(calibrateClock) * 1000]
      set begin [clock clicks]
      after $ms
      set end [clock clicks]
      set tsiClicksPerSecond [expr ($end - $begin)/$tsiConfig(calibrateClock)]
   }

   tsiInitInterpreter
  
###KJC  assume we always want -mode OFF  for SIO
###KJC    # SocketIO added 5/98	S.Wallace

     #This next call creates the Control Panel
     set sioTopFrame [$controlPanelProc $hideController $x $y]
     tsiSetControlPanelSIOMode $sioTopFrame

   puts "TSI ver. $tsiConfig(ControlPanelVersion), expertise: $tsiConfig(expertise), \
debug: $tsiConfig(debug), Platform: $tcl_platform(platform)"

   if {$tsiConfig(debug) && [info exists env(DISPLAY)]} {
      global env
      puts "\nDisplay set to >>$env(DISPLAY)<<"
   }
 

   ### Read in user preferred parameters if they exist
   if [file exists [file join $tsi_library tsiConfigPrefs.data]] {
      uplevel #0 [list source [file join $tsi_library tsiConfigPrefs.data]]
   }

   resolveTSIConfigConflicts
   wm protocol . WM_DELETE_WINDOW { quitSoar }
}

# DJP: Explicitly check for incoming commands and events
proc checkForEvents {k} {
   $k CheckForIncomingCommands
   after 5 checkForEvents $k
}
# DJP end

proc switchTSIInterfacePrefs {useNewTSI} {
   global tsiConfig
   
   set tsiConfig(interfaceType) [expr 1 + $useNewTSI]
   writeTSIPrefs
}

proc writeTSIPrefs {} {
   global tsiConfig tsi_library
   puts "writing tsi prefs"
   if {![catch "set chanID [open [file join $tsi_library "tsiConfigPrefs.data"] w]"]} {
   puts "writing tsi prefs"
      foreach n [array names tsiConfig] {
         if {[string match "* *" $tsiConfig($n)]} {
            puts $chanID "set tsiConfig($n) \{$tsiConfig($n)\}"
         } elseif {[string trim $tsiConfig($n)] != ""} {
            puts $chanID "set tsiConfig($n) $tsiConfig($n)"
         }
      }
   	close $chanID
   }
}

proc tsiInitRemoteAgent {} {
    wm withdraw .    
    tsiInitInterpreter
}

### Call "tsiInitAgent" to initialize a Soar agent interpreter to use
### the TSI
proc tsiInitAgent {} {
   global tsi_library tsiConfig soar7km_aliases interp_name 
    
   #if { $tsiConfig(sioDebug) > 3 } { puts "tsiInitAgent -- (begin)" }
   wm withdraw . 

   tsiInitInterpreter

###KJC  need to change Alias  for new SML implementation

   rename watch tsiInternalWatch
   rename help tsiInternalHelp
   rename learn tsiInternalLearn
   rename init-soar tsiInternalInitSoar
   rename excise tsiInternalExcise
       
   if $tsiConfig(debug) {puts "\n\nPrinting watch\n\n"}
   if $tsiConfig(debug) {puts [tsiInternalWatch]}
   if $tsiConfig(debug) {puts "\n\nPrinted\n\n"}
    
   soar_agent ExecuteCommandLine "alias p2 print --depth 2"
   soar_agent ExecuteCommandLine "alias d1 d 1"
   soar_agent ExecuteCommandLine "alias 1 d 1"
   #alias 2 d 2
   #alias 3 d 3
   #alias 4 d 4
   #alias 5 d 5
   #alias 6 d 6
   #alias 7 d 7
   #alias 8 d 8
   #alias 9 d 9
   soar_agent ExecuteCommandLine "alias pso print --stack --operators"

    if { ![info exists soar7km_aliases] } {
	### this should test a global var to see if they are already loaded.
	## Load the standard aliases
	if [catch {source [file join $tsi_library soar704km-aliases.tcl]} msg] {
	    puts "Failed to load Soar KM aliases: $msg"
	} else {
	    puts {Soar KM aliases loaded.  Type 'help alias' or 'alias' for more info.}
	}
    }   
   
	set X [expr $tsiConfig(AgentWindowX) + \
			  $tsiConfig(window_count) * $tsiConfig(AgentWindowDeltaX)]
	set Y [expr $tsiConfig(AgentWindowY) + \
			  $tsiConfig(window_count) * $tsiConfig(AgentWindowDeltaY)]

#   if [catch {tsiDebugger} msg] {
#  	   error "Failed to create Soar Debugger window: $msg"
#   }
   # Deleted catch b/c this gives more error info when crashing
   tsiDebugger
   #bind .tsw <Destroy> [list destroyAgent $interp_name]
   #monitor -add before-init-soar cleanupMonitorWindows
    
   #if { $tsiConfig(sioDebug) > 3 } { puts "tsiInitAgent -- (end)" }

}


### Call tsiInitInterpreter to initialize use of the TSI from
### *any* interpreter (including the Control Panel).

### Any time a new interpreter is created, you have to give it the
### correct global auto_path value and then call this procedure to set
### all the other appropriate global variables.
proc tsiInitInterpreter {} {

   ### In KernelSML, all the help is handled by CommandLine
   # proc help is defined in agent creation.
   if {[info exists help]} {rename help oldHelp}
 
   ### Do our "nicer" version of the "source" command
 
  rename source tsiInternalSource
 
   proc source {args} {
      global tsiSourceArgLast
 
      if {$args == {}} {
         puts "\[source $tsiSourceArgLast\]"
         ### We have to put the uplevel here because some procedures (like
         ### the package loader) depend on the loaded file sharing a variable
         ### with the calling procedure.  How's that for good programming
         ### style?
         
         ##uplevel 1 $_kernel ExecuteCommandLine "source $args" red
         uplevel 1 tsiInternalSource "$tsiSourceArgLast"
	
      } else {
	
          set tsiSourceArgLast "$args"
          uplevel 1 tsiInternalSource $args
	
      }
   }
 
}


proc tsiSetupAgentVars { agent } {
   global tsiClicksPerSecond
   $agent eval [list set tsiAgentStats(clicksPerSecond) $tsiClicksPerSecond]
   tsiResetAgentVars $agent
}

proc tsiResetAgentVars { agent } {

   $agent eval [list set tsiAgentStats(inputTime) 0]
   $agent eval [list set tsiAgentStats(outputTime) 0]
   $agent eval [list set tsiAgentStats(inputCycles) 0]
   $agent eval [list set tsiAgentStats(outputCycles) 0]
}

proc localAgentAddWME { agent objectID attribute value } {
   global tsiConfig

   if { $tsiConfig(sioDebug) > 4 } {
     puts "Adding WME to local agent: ($agent $objectID $attribute $value)"
   }

   set kernel [$agent eval [list set _kernel]]
   return [$kernel ExecuteCommandLine ("add-wme $objectID $attribute $value") $agent]
   ###KJC return [$agent eval "add-wme $objectID $attribute $value"]

}

proc localAgentRemoveWME { agent timeTag } {
   global tsiConfig

   if { $tsiConfig(sioDebug) > 4 } {
     puts "Removing WME from local agent: ($timeTag)"
   }

   ###KJC   return [$agent eval "remove-wme $timeTag"]
   set kernel [$agent eval [list set _kernel]]
   return [$kernel ExecuteCommandLine ("remove-wme $timeTag") $agent]
}

proc ChangeWME { oldID newPoint } {
}

#
# ldelete
#
# This function is a little function copied from the Tcl/TK book.  It deletes
#   an item from a list based on value.
#

proc ldelete { list value } {
	 set ix [lsearch -exact $list $value]
	 if {$ix >= 0} {
		  return [lreplace $list $ix $ix]
	 } else {
		  return $list
	 }
}
    
proc pac {agent} {
	# print the chunks created
 ###KJC   output-strings-destination -push -append-to-result
   set userProds [$agent ExecuteCommandLine "print --chunks"]
 ###KJC   output-strings-destination -pop

	foreach p $userProds {
		[$agent ExecuteCommandLine "print $p"]
   }
}    

proc loadWindowPreferences {} {
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

proc saveWindowPreferences {} {
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


proc resolveTSIConfigConflicts {} {
  global tcl_version tsiConfig tsi_library tsiSuppressMenus
  
  if { $tcl_version < 8.0 } {
     
     set tsiConfig(interfaceType) 1
  }

    if { $tsiConfig(expertise) == 0 } {
      
      ## -- Saving and loading production memory
      set tsiSuppressMenus(file,3) 1    
      set tsiSuppressMenus(file,4) 1
     
       ## -- Print operator stack
       set tsiSuppressMenus(view,1) 1

      ## -- Listing the agents to console
       set tsiSuppressMenus(show,4,1) 1
       
       ## -- Printing out all command, variable and proc names defined to console
       set tsiSuppressMenus(show,4,7) 1
       set tsiSuppressMenus(show,4,8) 1
       set tsiSuppressMenus(show,4,9) 1

       ## -- Printing out attribute preference mode to console
       set tsiSuppressMenus(show,5,1) 1
       
       ## -- Printing out O-support mode to console
      set tsiSuppressMenus(show,5,7) 1     
      
      ## -- Add-wme and Select-operator items  
      set tsiSuppressMenus(memory,3) 1
      set tsiSuppressMenus(memory,3) 4
      
       ## -- Saving backtraces       
      set tsiSuppressMenus(show,5,8) 1       
      
      ## -- Tcl and Tk help menus
      set tsiSuppressMenus(help,4) 1
      set tsiSuppressMenus(help,5) 1
      

    }

}

# Define the sml kernel-event callbacks

proc AgentCreatedCallback {id userData agent} {
	puts "Agent_callback:: [$agent GetAgentName] created"
}
proc AgentReinitializedCallback {id userData agent} {
	puts "[$agent GetAgentName] reinitialized"
}
proc AgentDestroyedCallback {id userData agent} {
	puts "destroying agent [$agent GetAgentName]"
}
proc SystemShutdownCallback {id userData kernel} {
	puts "Shutting down kernel $kernel"
}
proc RhsFunctionTest {id userData agent functionName argument} {
	puts "Agent $agent called RHS function $functionName with argument(s) '$argument' and userData '$userData'"
	return "success"
}

# Define the sml Agent-event callbacks


proc PrintCallback {id userData agent message} {
	#puts -nonewline $message
        tsiOutputToWidget $message
}

proc ProductionExcisedCallback {id userData agent prodName instantiation} {
	puts "removing $prodName"
}

proc ProductionFiredCallback {id userData agent prodName instantiation} {
	puts "fired $prodName"
}

proc PhaseExecutedCallback {id userData agent phase} {
	puts "phase $phase executed"
}

proc StructuredTraceCallback {id userData agent pXML} {
	puts "structured data: [$pXML GenerateXMLString 1]"
}

proc OutputCallback {userData agent commandName outputWme} {
	puts "got a $commandName command"
	puts "id=[$outputWme GetIdentifierName] attribute=[$outputWme GetAttribute] value=[$outputWme GetValueAsString]"
}

