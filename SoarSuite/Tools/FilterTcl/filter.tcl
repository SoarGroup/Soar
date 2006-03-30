proc AgentCreatedCallback {id userData agent} {
	puts "Agent_callback:: [$agent GetAgentName] created"
}

proc MyFilter {id userData agent filterName commandLine} {
	puts "Command line $commandLine"
	return "print s1"
}

proc createFilter {} {
    global smlEVENT_AFTER_AGENT_CREATED smlEVENT_BEFORE_AGENT_REINITIALIZED 
    global _kernel
    global sml_Names_kFilterName
    
	# Then create a kernel
	#set _kernel [Kernel_CreateKernelInCurrentThread SoarKernelSML 0]

	# I don't know how to pass a null string in Tcl, so have to pass the IP address which indicates "this machine"
	set _kernel [Kernel_CreateRemoteConnection true 127.0.0.1]

	if {[$_kernel HadError]} {
		puts "Error creating kernel: "
		puts "[$_kernel GetLastErrorDescription]\n"
		return
	}

	puts "Created kernel\n"

	# Practice callbacks by registering for agent creation event
	set agentCallbackId0 [$_kernel RegisterForAgentEvent $smlEVENT_AFTER_AGENT_CREATED AgentCreatedCallback ""]
	
	
	# int clientFilter = pKernel->RegisterForClientMessageEvent(sml_Names::kFilterName, &MyFilter, 0) ;
	# Don't know how to access sml_Names::kFilterName in Tcl, so I'm using the string instead
	set filterCallbackId0 [$_kernel RegisterForClientMessageEvent $sml_Names_kFilterName MyFilter ""]
}

# Start by loading the tcl interface library
# It seems this has to happen outside of the proc or I get a failure
set soar_library [file join [pwd]]
set tcl_library [file join [pwd] Tcl_sml_ClientInterface]
lappend auto_path  $soar_library

puts "path is $auto_path"
puts $tcl_library

package require tcl_sml_clientinterface

createFilter
