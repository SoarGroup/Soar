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

proc MyFilter {id userData agent filterName commandXML} {
	global _kernel
	global sml_Names_kFilterCommand
	global sml_Names_kFilterOutput

	set name [$agent GetAgentName]
	set interpreter [getInterpreter $name $agent]

	puts Hello
	puts $commandXML

	set xml [ElementXML_ParseXMLFromString $commandXML]
	set commandLine [$xml GetAttribute $sml_Names_kFilterCommand]

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

	# Appending 'Tcl' to the output so I can see that it's been filtered by Tcl.
	$xml AddAttribute $sml_Names_kFilterOutput "Tcl $result"
	$xml AddAttribute $sml_Names_kFilterCommand ""
	
	set outputXML [$xml GenerateXMLString 1]

	puts "Output $outputXML"

	return $outputXML
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

	puts "Filter registered"
}

# Start by loading the tcl interface library
# It seems this has to happen outside of the proc or I get a failure
set soar_library [file join [pwd]]
lappend auto_path  $soar_library

package require tcl_sml_clientinterface

createFilter
