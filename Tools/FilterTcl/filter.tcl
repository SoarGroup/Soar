proc AgentCreatedCallback {id userData agent} {
	puts "Agent_callback:: [$agent GetAgentName] created"
}

# This is a lazy function which will either return the interpreter
# already associated with this agent or create a new one for this agent
proc getInterpreter {agentName _agent} {
	global _kernel

	# See if we already have an interpreter for this agent
	#puts "Looking for interpreter for $agentName"

	foreach i [interp slaves] {
		#puts "Have interpreter $i"
		if [string equal $i $agentName] {
			#puts "Found match for $i"
			return $i
		}
	}
	
	puts "No interpreter matched so creating one"
	set interpreter [interp create $agentName]
	
	#these lines don't seem to be required, and if they are you can probably put them in FilterTcl/commands.tcl 
	#$interpreter eval [list lappend auto_path .]
	#$interpreter eval [list package require tcl_sml_clientinterface]
	
	$interpreter eval [list set _kernel $_kernel]
	$interpreter eval [list set _agentName $agentName]
   	$interpreter alias soar_kernel $_kernel
	$interpreter alias soar_agent $_agent
	
	# We need to source the overloaded commands into the child interpreter
	# so they'll be correctly in scope within that interpreter.
	# Also, right now we're executing inside SoarLibrary\bin so we need
	# play this game to find the file to load.
	$interpreter eval source ../../Tools/FilterTcl/commands.tcl
			
	return $interpreter
}

proc MyFilter {id userData agent filterName commandLine} {
	global _kernel
	set name [$agent GetAgentName]
	set interpreter [getInterpreter $name $agent]

	puts "$name Command line $commandLine"

	# This is a complicated line where all the action happens.
	# At the core is "$interpreter eval $commandLine"
	# That evaluates the command within the child interpreter for this agent
	# The catch around that places whether this call succeeds or fails into "error"
	# and the result of the command's execution (or an error message) into result.
	set error [catch {$interpreter eval $commandLine} result]
	#set error [catch {$_kernel ExecuteCommandLine "$commandLine" $name} result]
	#set error [catch {$agent ExecuteCommandLine "$commandLine"} result ]
	puts "Error is $error and Result is $result"

	return ""
}

proc parentExecCommand { command args } {
	$_kernel ExecuteCommandLine $command $args
}

proc createFilter {} {
    global smlEVENT_AFTER_AGENT_CREATED smlEVENT_BEFORE_AGENT_REINITIALIZED smlEVENT_PRINT
    global _kernel
    global sml_Names_kFilterName
    
	# Then create a kernel
	#set _kernel [Kernel_CreateKernelInCurrentThread SoarKernelSML 0]

	set _kernel [Kernel_CreateRemoteConnection]

	if {[$_kernel HadError]} {
		puts "Error creating kernel: "
		puts "[$_kernel GetLastErrorDescription]\n"
		return
	}

	puts "Created kernel\n"

	# Practice callbacks by registering for agent creation event
	set agentCallbackId0 [$_kernel RegisterForAgentEvent $smlEVENT_AFTER_AGENT_CREATED AgentCreatedCallback ""]
	
	# Register the filter callback
	set filterCallbackId0 [$_kernel RegisterForClientMessageEvent $sml_Names_kFilterName MyFilter ""]
	
	# Execute a couple of simulated commands, so it's easier to see errors and problems
	set agent [$_kernel GetAgent soar1]
	
	set res1 [MyFilter 123 "" $agent $sml_Names_kFilterName "print s3"]
	set res2 [MyFilter 123 "" $agent $sml_Names_kFilterName "print s4"]
	set res3 [MyFilter 123 "" $agent $sml_Names_kFilterName "puts hello"]
	set res4 [MyFilter 123 "" $agent $sml_Names_kFilterName "pwd"]
	set res5 [MyFilter 123 "" $agent $sml_Names_kFilterName "set test hello"]
	set res6 [MyFilter 123 "" $agent $sml_Names_kFilterName "puts \$test"]
}

# Start by loading the tcl interface library
# It seems this has to happen outside of the proc or I get a failure
set soar_library [file join [pwd]]
lappend auto_path  $soar_library

package require tcl_sml_clientinterface

createFilter
