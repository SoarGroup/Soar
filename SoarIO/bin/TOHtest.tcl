proc PrintCallback {id userData agent message} {
   puts -nonewline $message
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

proc AgentCreatedCallback {id userData agent} {
	puts "[$agent GetAgentName] created"
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

#load the sml stuff
lappend auto_path .
package require tcl_sml_clientinterface

#create an embedded kernel running in the kernel's thread (so we don't have to poll for messages)
set kernel [Kernel_CreateKernelInCurrentThread KernelSML]

set agentCallbackId0 [$kernel RegisterForAgentEvent $smlEVENT_AFTER_AGENT_CREATED AgentCreatedCallback ""]
set agentCallbackId1 [$kernel RegisterForAgentEvent $smlEVENT_BEFORE_AGENT_REINITIALIZED AgentReinitializedCallback ""]
set agentCallbackId2 [$kernel RegisterForAgentEvent $smlEVENT_BEFORE_AGENT_DESTROYED AgentDestroyedCallback ""]
set systemCallbackId [$kernel RegisterForSystemEvent $smlEVENT_BEFORE_SHUTDOWN SystemShutdownCallback ""]
set rhsCallbackId [$kernel AddRhsFunction RhsFunctionTest ""]

#create an agent named Soar1
set agent [$kernel CreateAgent Soar1]

set printCallbackId [$agent RegisterForPrintEvent $smlEVENT_PRINT PrintCallback ""]
#set productionCallbackId [$agent RegisterForProductionEvent $smlEVENT_BEFORE_PRODUCTION_REMOVED ProductionExcisedCallback ""]
set productionCallbackId [$agent RegisterForProductionEvent $smlEVENT_AFTER_PRODUCTION_FIRED ProductionFiredCallback ""]
set runCallbackId [$agent RegisterForRunEvent $smlEVENT_AFTER_PHASE_EXECUTED PhaseExecutedCallback ""]

#load the TOH productions
set result [$agent LoadProductions demos/towers-of-hanoi/towers-of-hanoi.soar]
#loads a function to test the user-defined RHS function stuff
set result [$agent LoadProductions TOHtest.soar]

$kernel ExecuteCommandLine "run 2 -e" Soar1

$agent UnregisterForProductionEvent $productionCallbackId
$agent UnregisterForRunEvent $runCallbackId

$kernel ExecuteCommandLine "run 3" Soar1

puts ""

#set the watch level to 0
set result [$kernel ExecuteCommandLine "watch 0" Soar1]
#excise the monitor production
set result [$kernel ExecuteCommandLine "excise towers-of-hanoi*monitor*operator-execution*move-disk" Soar1]

#run TOH the rest of the way and time it using Tcl's built-in timer
set speed [time {set result [$kernel ExecuteCommandLine "run" Soar1]}]
puts "\n$speed"

#the output of "print s1" should contain "^rhstest success"
if { [string first "^rhstest success" [$kernel ExecuteCommandLine "print s1" Soar1]] == -1 } {
	puts "\nRHS test FAILED"
} else {
	puts "\nRHS test SUCCEEDED"
}

#set result [$kernel ExecuteCommandLine "init-soar" Soar1]

#remove all the remaining callback functions (not required, just to test)
puts "removing callbacks"
$kernel UnregisterForAgentEvent $agentCallbackId0
$kernel UnregisterForAgentEvent $agentCallbackId1
$kernel UnregisterForAgentEvent $agentCallbackId2
$kernel UnregisterForSystemEvent $systemCallbackId
$kernel RemoveRhsFunction $rhsCallbackId

$kernel DestroyAgent $agent

#give Tcl object ownership of underlying C++ object so when we delete the Tcl object they both get deleted
set result [$kernel -acquire]
#delete kernel object (this will also delete any agents that are still around)
set result [$kernel -delete]
#don't leave bad pointers around
unset agent
unset kernel