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
   global interp_name interp_agentSocket  tsi_library \
       tcl_platform tsiConfig tsiSimulationState tsiSimulatorPath \
       tsiClicksPerSecond 

   ## Make sure tk is loaded, and halt if it is not.

   if {[info commands tk]=={}} {
      puts {You are not running a version of Soar that supports the TSI. 
Run a version with TK, contact your system administrator, or email
soar-bugs@umich.edu for help.   

[Load aborted in tsi.tcl]}
      error "Cannot run TSI without the Tk package"
   }

   ### The tsi must be initialized from a non-soar interpreter
   foreach i [info loaded {}] {
      if {[lindex $i 1]=={Soar}} {
         tk_dialog .error Error {The TSI interface can only be initialized from the controlling interpreter.  This should be the top-level interpreter, and it should not have the Soar package loaded.} \
                   error 0 Dismiss
         error "Cannot start TSI from a Soar agent interpreter."
      }
   }

   set tsiSimulatorPath [pwd]
   
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

    set tsiConfig(ControlPanelVersion) 4.0.1
    set controlPanelProc makeTSIDefaultControlPanel
    set x $tsiConfig(ControlPanelX)
    set y $tsiConfig(ControlPanelY)

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
       }
   }


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
  
   # SocketIO added 5/98	S.Wallace
   set sioTopFrame [$controlPanelProc $hideController $x $y]
   tsiSetControlPanelSIOMode $sioTopFrame

   puts "TSI ver. $tsiConfig(ControlPanelVersion), expertise: $tsiConfig(expertise), \
debug: $tsiConfig(debug), Platform: $tcl_platform(platform)"

   if {$tsiConfig(debug) && [info exists env(DISPLAY)]} {
      global env
      puts "\nDisplay set to >>$env(DISPLAY)<<"
   }
   # Set up doc directories
   global doc_dir tcl_man_dir tk_man_dir soar_man_dir soar_library
   
   if {![info exists doc_dir]} {
      if {![info exists tcl_man_dir]} {
   	   set tsiConfig(tcl_man_dir) ""
      } else {
         set tsiConfig(tcl_man_dir) $tcl_man_dir
      }
      if {![info exists tk_man_dir]} {
   	   set tsiConfig(tk_man_dir) ""
      } else {
         set tsiConfig(tk_man_dir) $tk_man_dir
      }
      set tsiConfig(soar_man_dir) [file join $soar_library .. doc man]
   } else {
      set tsiConfig(tcl_man_dir) [file join $doc_dir tcl]
      set tsiConfig(tk_man_dir) [file join $doc_dir tk]
      set tsiConfig(soar_man_dir) [file join $doc_dir man]
   }

   ### Read in user preferred parameters if they exist
   if [file exists [file join $tsi_library tsiConfigPrefs.data]] {
      uplevel #0 [list source [file join $tsi_library tsiConfigPrefs.data]]
   }

   resolveTSIConfigConflicts
   wm protocol . WM_DELETE_WINDOW { quitSoar }
}

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

   rename watch tsiInternalWatch
   rename help tsiInternalHelp
   rename learn tsiInternalLearn
   rename init-soar tsiInternalInitSoar
   rename excise tsiInternalExcise
       
   if $tsiConfig(debug) {puts "\n\nPrinting watch\n\n"}
   if $tsiConfig(debug) {puts [tsiInternalWatch]}
   if $tsiConfig(debug) {puts "\n\nPrinted\n\n"}
    
   alias p2 print -depth 2
   alias d1 d 1
   alias 1 d 1
   alias 2 d 2
   alias 3 d 3
   alias 4 d 4
   alias 5 d 5
   alias 6 d 6
   alias 7 d 7
   alias 8 d 8
   alias 9 d 9
   alias pso print -stack -operators

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
         uplevel 1 tsiInternalSource "$tsiSourceArgLast"
	
      } else {
	
          set tsiSourceArgLast $args
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

 
   return [$agent eval "add-wme $objectID $attribute $value"]

}

proc localAgentRemoveWME { agent timeTag } {
   return [$agent eval "remove-wme $timeTag"]
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
    
proc pac {} {
	
   output-strings-destination -push -append-to-result
   set userProds [print -chunk]
   output-strings-destination -pop

	foreach p $userProds {
		print $p
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

