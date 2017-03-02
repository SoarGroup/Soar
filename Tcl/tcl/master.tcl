##
# Initialize some globals
##
global tclScriptDir soarDir callbackIDs
set soarDir [file dirname [pwd]]
set tclScriptDir [file join $soarDir tcl]
array set callbackIDs {
  rhs -1
  filter -1
  agent_created -1
  agent_destroyed -1
}

lappend auto_path $soarDir

proc createSlave {agentName} {
  global tclScriptDir auto_path _kernel _soarInstance
  #puts "tcl::master::createSlave($agentName)"
  
  # Configure the slave interpreter
  
  # Look up the SML agent
  set client_agent [$_kernel GetAgent $agentName]

  set slave [interp create $agentName]
  
  # Set up aliases to common procs in master
  $slave eval [list lappend auto_path $tclScriptDir]
  
  # We want all the slaves using the same directory stack, otherwise
  # they can get out of synch and cause weird behavior
  $slave alias pushd pushd
  $slave alias popd popd
  $slave alias topd topd
  $slave alias dirs dirs
  $slave alias cd cd
  $slave eval [list set DIR_STACK {}]
  
  # Just for debugging.  Probably not a smart thing to do
  $slave alias mastereval eval
  $slave alias slaves eval [list interp slaves]
  
  # Make kernel, soar instance and agent commands available in slave
  $slave alias $_kernel $_kernel
  $slave alias $_soarInstance $_soarInstance
  $slave alias $client_agent $client_agent
  
  # Store kernel and agent in slave globals
  $slave eval [list set _kernel $_kernel]
  $slave eval [list set _agent $client_agent]
  $slave eval [list set _soarInstance $_soarInstance]
  
  pushd $tclScriptDir
  if { [catch {$slave eval source slave.tcl} result] } {
    puts "ERROR: Failed to load slave.tcl:\n$result"
  }
  
  # Initialize slave interpreter
  $slave eval [list initializeSlave]

  # Source tcl-setup file (can override Soar commands/aliases!)
  if { [catch {$slave eval source tcl-setup.soar} result] } {
    puts "ERROR: Failed to load tcl-setup.soar:\n$result"
  }
  popd

  # Send output to the "stdout" channel by default
  $slave eval [list output-strings-destination -push -channel stdout]

  #return $agentName
}

# This is a lazy function which will either return the interpreter
# already associated with this agent or create a new one for this agent
proc getSlave { agentName } {
  global _soarInstance
  if { ![interp exists $agentName] } {
      $_soarInstance Tcl_Message_Library [concat "create " $agentName]
  }
  return $agentName
}

proc destroySlave { slaveName } {
  #puts "tcl::master::destroySlave($slaveName)"

  if {[interp exists $slaveName]} {
    interp delete $slaveName
  }
}

##############################
### Callback utility functions

##
# Called on each SML command. Performs Tcl expansion.
##
proc smlfilter {agentName commandLine} {
  global _kernel

  set slave [getSlave $agentName]
  $slave eval clearOutputBuffer

  set commandName [lindex [split $commandLine " "] 0]
  set hasProcedure [$slave eval [list hasProcedure $commandName]]

  if { $hasProcedure == "true" } {
	# The slave has the procedure, try executing it
	if { [catch {$slave eval [concat uplevel #0 puts [list \[$commandLine\]]]} slaveResult] } {
		# There was a tcl error, so throw it along with the previous output
		error "[$slave eval getOutputBuffer]$slaveResult"
	}
  } else {
	  # The slave doesn't have the procedure, try sending it to soar
	  if { [catch {$slave eval [list executeCommandLine $commandLine]} soarResult] } {
		# There was a soar error, so throw it along with the previous output
		error "[$slave eval getOutputBuffer]$soarResult"
	  } else {
		# Soar properly handled the command, add the result to the output
		$slave eval [list appendOutputBuffer $soarResult]
	  }
  }

  return [$slave eval getOutputBuffer]
}

##
# Called when an agent performs the tcl RHS function
proc smltclrhsfunction { agentName expression } {
  set slave [getSlave $agentName]
  $slave eval clearOutputBuffer
  if { [catch {$slave eval [concat uplevel #0 puts [list \[$expression\]]]} returnVal] } {
    $slave eval [list appendOutputBuffer $returnVal]
  }
  
  return [$slave eval getOutputBuffer]
}

proc smlstartup {} {
    #puts "tcl::master::smlstartup()"
}

##
# Called when the system shuts down
proc smlshutdown {} {
  #puts "tcl::master::smlshutdown()"

  removeCallbackHandlers
  foreach slave [interp slaves] {
      #puts "     Deleting Slave $slave"
      interp delete $slave
  }
}

proc smlCreateAgentCallback { id userData agent } {
	#puts "tcl::master::smlCreateAgentCallback([$agent GetAgentName])"

    global _soarInstance
    set slaveName [$agent GetAgentName]
    $_soarInstance Tcl_Message_Library [concat "create " $slaveName]
}

proc smlDestroyAgentCallback { id userData agent } {
  #puts "tcl::master::smlDestroyAgentCallback([$agent GetAgentName])"

  global _soarInstance
  set slaveName [$agent GetAgentName]
  $_soarInstance Tcl_Message_Library [concat "destroy " $slaveName]
}

#####################
### Callback handlers

##
# Called on each SML command callback.
##
proc smlFilterCallback {id userData agent filterName commandXML} {
  global sml_Names_kFilterCommand
  global sml_Names_kFilterOutput
  global sml_Names_kFilterError
  
  set name [$agent GetAgentName]
  
  set xml [ElementXML_ParseXMLFromString $commandXML]
  set commandLine [$xml GetAttribute $sml_Names_kFilterCommand]
  
  # Any backslash characters should be preserved (I think) so that
  # source e:\file.soar is valid and the "\f" isn't interpreted as an escape character.
  set commandLine [string map {\\ \\\\} $commandLine]

  # This is a complicated line where all the action happens.
  # At the core is "$slave eval $commandLine"
  # That evaluates the command within the child interpreter for this agent
  # The catch around that places whether this call succeeds or fails into "error"
  # and the result of the command's execution (or an error message) into result.
  if { [catch {smlfilter $name $commandLine} result] } {
	  $xml AddAttribute $sml_Names_kFilterError "true"
  }
  
  # Return the result of the command as the output string for the command
  # which in turn will appear in the debugger
  $xml AddAttribute $sml_Names_kFilterOutput "$result"
  
  # Remove the original command from the XML that is returned, indicating that this command
  # has been consumed.  Otherwise it will continue on through other filters and eventually
  # be executed by the kernel's command line processor.
  $xml AddAttribute $sml_Names_kFilterCommand ""
  
  set outputXML [$xml GenerateXMLString 1]
  
  return $outputXML
}

##
# Called on RHS exec tcl from a rule firing
##
proc tcl {id userData agent functionName argument} {
  set return_val [smltclrhsfunction [$agent GetAgentName] $argument]
  return return_val
}

##
# Loads SWIG dll that provides SML interface
##
proc loadSmlLibrary {} {
  global auto_path soarDir
  
  lappend auto_path $soarDir
  
  pushd $soarDir
  package require tcl_sml_clientinterface
  popd
}

proc createCallbackHandlers {} {
  global _kernel callbackIDs
  global sml_Names_kFilterName smlEVENT_BEFORE_AGENT_DESTROYED smlEVENT_AFTER_AGENT_CREATED

  # Register main command processing callback function
  if {$callbackIDs(filter) == -1} {
    #$_kernel EnableFiltering 1
    set callbackIDs(filter) [$_kernel RegisterForClientMessageEvent $sml_Names_kFilterName smlFilterCallback ""]
  }

  # Install the tcl RHS function
  if {$callbackIDs(rhs) == -1} {
    set callbackIDs(rhs) [$_kernel AddRhsFunction tcl ""]
  }

  # Register agent creation callback function
  if {$callbackIDs(agent_created) == -1} {
    set callbackIDs(agent_created) [$_kernel RegisterForAgentEvent $smlEVENT_AFTER_AGENT_CREATED smlCreateAgentCallback ""]
  }
  
  # Register agent deletion callback function
  if {$callbackIDs(agent_destroyed) == -1} {
    set callbackIDs(agent_destroyed) [$_kernel RegisterForAgentEvent $smlEVENT_BEFORE_AGENT_DESTROYED smlDestroyAgentCallback ""]
  }
}

proc removeCallbackHandlers {} {
  global  _kernel callbackIDs
  
  # These should be re-enabled after someone fixes the issues with callback IDs.  Currently
  # I think there's a bug in our swig interface where it's not properly returning an
  # integer, which is what the ID should be.  Instead callback registration returns a string
  # that starts with "intptr_t".  That string does not seem to a proc that manipulates a C++ object
  # like the kernel.
  
  #  if {$callbackIDs(filter) != -1} {
    #    set result [$_kernel UnregisterForClientMessageEvent $callbackIDs(filter)]
    #    set callbackIDs(filter) -1
    #  }
  #  if {$callbackIDs(rhs) != -1} {
    #    $_kernel RemoveRhsFunction $callbackIDs(rhs);
    #    set callbackIDs(rhs) -1
    #  }
  #  if {$callbackIDs(agent_destroyed) != -1} {
    #    set result [$_kernel UnregisterForClientMessageEvent $callbackIDs(agent_destroyed)]
    #    set callbackIDs(agent_destroyed) -1
    #  }
}

##
# Initializes the kernel and registers for events
##
proc initializeMaster { } {
  global _kernel _soarInstance callbackIDs
  
  loadSmlLibrary
  
  set _soarInstance [getSoarInstance]
  set _kernel [$_soarInstance Get_Kernel]
}

