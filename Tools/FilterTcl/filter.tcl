proc AgentCreatedCallback {id userData agent} {
	puts "Agent_callback:: [$agent GetAgentName] created"
}

# This is a lazy function which will either return the interpreter
# already associated with this agent or create a new one for this agent
proc getInterpreter {agentName} {
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
	return $interpreter
}

proc print {arg} {
	puts "Called internal print"
	puts $arg
}

proc MyFilter {id userData agent filterName commandLine} {
	set name [$agent GetAgentName]
	set interpreter [getInterpreter $name]

	puts "$name Command line $commandLine"
	
	# Break the command into its separate pieces
	# (not sure if this will handle embedded quotes correctly)
	set args [split $commandLine " "]
	
	# Extract the first one which is the underlying command
	set first [lindex $args 0]
	puts "First command is $first"

	set numberArgs [llength $args]
	set otherArgs [lrange $args 1 $numberArgs]
	puts "Other args are $otherArgs"
	
	set combinedArgs [join $otherArgs]	
	puts "Combined args are $combinedArgs"

#	set command "{"	
#	append command [join $args]
#	append command "}"

	# Maybe re-assembling the pieces will strip the surrounding quotes
	set command [join $args]

	puts "Command is $command"	
	set result [$interpreter eval $command]	
	puts "Result is $result"

	# This approach works for passing in "print s1" and calls to our
	# tcl print method.  But it fails for other tcl commands like "set x y"	
#	if { $numberArgs > 1 } {
#		set result [$interpreter eval [$first $otherArgs]]
#	} else {
#		puts "Executing single command $first"
#		set result [$interpreter eval [$first]]
#	}
	
	# For the moment, always return "print s1" as the result of the filter
	return "print s1"
}

proc createFilter {} {
    global smlEVENT_AFTER_AGENT_CREATED smlEVENT_BEFORE_AGENT_REINITIALIZED 
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
	set res3 [MyFilter 123 "" $agent $sml_Names_kFilterName "puts hello"]
	set res4 [MyFilter 123 "" $agent $sml_Names_kFilterName "pwd"]
	set res5 [MyFilter 123 "" $agent $sml_Names_kFilterName "set test hello"]
#	set res1 [MyFilter 123 "" $agent $sml_Names_kFilterName "print s3"]
#	set res2 [MyFilter 123 "" $agent $sml_Names_kFilterName "print s4"]
}

# Start by loading the tcl interface library
# It seems this has to happen outside of the proc or I get a failure
set soar_library [file join [pwd]]
lappend auto_path  $soar_library

package require tcl_sml_clientinterface

createFilter
