#
# This is a test script which tests several aspects of the Tcl SML interface
#  including kernel and agent creation, running, registering and unregistering
#  several kinds of callbacks, inreinitializing, agent destruction, and kernel
#  destruction (and maybe some other things, too).
#
# In order for this to work, the Tcl_sml_ClientInterface package must be located
#  in auto_path (i.e. the directory which contains the Tcl_sml_ClientInterface must
#  be lappend'ed to auto_path).  On Windows, assuming this file is located in
#  SoarIO/bin, this means appending the current directory.  Other platforms may
#  require some other directory to be there.  Modify the line below appropriately.
#

#load the sml stuff
lappend auto_path .
#this next line for tests on winter
lappend auto_path ~/sandbox/lib
package require tcl_sml_clientinterface

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

proc StructuredTraceCallback {id userData agent pXML} {
	puts "structured data: [$pXML GenerateXMLString 1]"
}

#create an embedded kernel running in the kernel's thread (so we don't have to poll for messages)
set kernel [Kernel_CreateKernelInCurrentThread SoarKernelSML]

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
set structuredCallbackId [$agent RegisterForXMLEvent $smlEVENT_XML_TRACE_OUTPUT StructuredTraceCallback ""]

#load the TOH productions
set result [$agent LoadProductions demos/towers-of-hanoi/towers-of-hanoi.soar]
#loads a function to test the user-defined RHS function stuff
set result [$agent LoadProductions tests/TOHtest.soar]

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

set result [$kernel ExecuteCommandLine "init-soar" Soar1]

$kernel DestroyAgent $agent

#remove all the remaining kernel callback functions (not required, just to test)
puts "removing callbacks"
$kernel UnregisterForAgentEvent $agentCallbackId0
$kernel UnregisterForAgentEvent $agentCallbackId1
$kernel UnregisterForAgentEvent $agentCallbackId2
$kernel UnregisterForSystemEvent $systemCallbackId
$kernel RemoveRhsFunction $rhsCallbackId

#give Tcl object ownership of underlying C++ object so when we delete the Tcl object they both get deleted
set result [$kernel -acquire]
#delete kernel object (this will also delete any agents that are still around)
set result [$kernel -delete]
#don't leave bad pointers around
unset agent
unset kernel
