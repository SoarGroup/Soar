###############################################################################
#
# These are the first steps towards a full Tcl filter that will
# allow the use of Tcl commands within a Soar process--e.g.
# executing Tcl commands within the Java debugger.
#
# The basic execution cycle is that when launched, this filter
# connects to an existing kernel and registers a command line filter.
# As the user enters commands, those commands are sent through
# the filter where they are evaluated as Tcl commands.
#
# In order for Soar commands to still work they are registered
# with simple Tcl proxies that call over to the kernel to execute
# the command.
#
# In this way, Tcl commands are evaluated as Tcl and Soar commands
# are sent back to Soar.
#
# The hope is that someone who actually knows Tcl well will invest
# some time in taking this from a prototype into a fully functional filter.
# Some of the known issues:
#   -- the ls command is using the kernel's CWD, not Tcl's.  Tcl has its own
#	   dir command built in, so we should probably just use that instead.
#
#   -- there seems to be a very small maximum string size (512 chars maybe?) and going
#	   beyond that is causing problems.  Examples are "print --stack" for a stack 100 states deep
#	   produces an error when the strings are passed through Tcl.
#
#   -- Closing the wish window results in a crash.  Typing "exit" at the console
#      prompt does not cause a crash.  Presumably a call to "$_kernel Shutdown" or "exit" is needed
#	   when the wish window is closed.
#
#	-- Source only displays the result of the last command.  It should collect up the results of
#	   each command and return that.
#
#   -- alias with no args seems to cause problems.
#
#	-- In general each Soar command should be tested and examined to see whether it behaves
#	   as desired.  Some (source, sp?) will likely need to be intercepted and have special
#	   processing inserted if the default behavior isn't sufficient.  For example, we might
#	   want to offer additional expansion of the args passed to sp so you can embed clever
#	   things within a production.
# 
###############################################################################

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

	#puts $commandXML

	set xml [ElementXML_ParseXMLFromString $commandXML]
	set commandLine [$xml GetAttribute $sml_Names_kFilterCommand]

	# Any backslash characters should be preserved (I think) so that
	# source e:\file.soar is valid and the "\f" isn't interpreted as an escape character.	
	set commandLine [string map {\\ \\\\} $commandLine]
	
	puts $commandLine

	# This is a complicated line where all the action happens.
	# At the core is "$interpreter eval $commandLine"
	# That evaluates the command within the child interpreter for this agent
	# The catch around that places whether this call succeeds or fails into "error"
	# and the result of the command's execution (or an error message) into result.
	set error [catch {$interpreter eval $commandLine} result]

	puts "Error is $error and Result is $result"

	# Return the result of the command as the output string for the command
	# which in turn will appear in the debugger
	$xml AddAttribute $sml_Names_kFilterOutput "$result"

	# Remove the original command from the XML that is returned, indicating that this command
	# has been consumed.  Otherwise it will continue on through other filters and eventually
	# be executed by the kernel's command line processor.
	$xml AddAttribute $sml_Names_kFilterCommand ""
	
	set outputXML [$xml GenerateXMLString 1]

	#puts "Output $outputXML"

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

	# This is important.  We'll disable filtering on commands *sent* from this client.
	# Otherwise, when this client calls ExecuteCommandLine the commands will come back to
	# the filter again--causing an infinite loop.
	$_kernel EnableFiltering 0

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
