proc PrintCallback {id userData agent message} {
   puts -nonewline $message
}

proc ProductionExcisedCallback {id userData agent prodName instantiation} {
	puts "removing $prodName"
}

proc ProductionFiredCallback {id userData agent prodName instantiation} {
	puts "firing $prodName"
}

proc PhaseExecutedCallback {id userData agent phase} {
	puts "phase $phase executed"
}

proc AgentReinitializedCallback {id userData agent} {
	puts "[$agent GetAgentName] reinitialized"
}

proc SystemShutdownCallback {id userData kernel} {
	puts "Shutting down $kernel"
}

#load the sml stuff
lappend auto_path .
package require tcl_sml_clientinterface

#create an embedded kernel running in the kernel's thread (so we don't have to poll for messages)
set kernel [Kernel_CreateEmbeddedConnection KernelSML 0]
#create an agent named Soar1
set agent [$kernel CreateAgent Soar1]

set printCallbackId [$agent RegisterForPrintEvent $smlEVENT_PRINT PrintCallback ""]
#set productionCallbackId [$agent RegisterForProductionEvent $smlEVENT_BEFORE_PRODUCTION_REMOVED ProductionExcisedCallback ""]
set productionCallbackId [$agent RegisterForProductionEvent $smlEVENT_AFTER_PRODUCTION_FIRED ProductionFiredCallback ""]
set runCallbackId [$agent RegisterForRunEvent $smlEVENT_AFTER_PHASE_EXECUTED PhaseExecutedCallback ""]
set agentCallbackId [$agent RegisterForAgentEvent $smlEVENT_BEFORE_AGENT_REINITIALIZED AgentReinitializedCallback ""]
set systemCallbackId [$kernel RegisterForSystemEvent $smlEVENT_BEFORE_SHUTDOWN SystemShutdownCallback ""]
cd demos/towers-of-hanoi
#load the TOH productions
set result [$agent LoadProductions towers-of-hanoi.soar]

$kernel ExecuteCommandLine "run 2 -e" Soar1

$agent UnregisterForProductionEvent $smlEVENT_AFTER_PRODUCTION_FIRED $productionCallbackId
$agent UnregisterForRunEvent $smlEVENT_AFTER_PHASE_EXECUTED $runCallbackId

$kernel ExecuteCommandLine "run 3" Soar1

puts ""

#set the watch level to 0
set result [$kernel ExecuteCommandLine "watch 0" Soar1]
#excise the monitor production
set result [$kernel ExecuteCommandLine "excise towers-of-hanoi*monitor*operator-execution*move-disk" Soar1]

#run TOH the rest of the way and time it using Tcl's built-in timer
set speed [time {set result [$kernel ExecuteCommandLine "run" Soar1]}]
cd ../..
puts "\n$speed"

set result [$kernel ExecuteCommandLine "init-soar" Soar1]

#give Tcl object ownership of underlying C++ object so when we delete the Tcl object they both get deleted
set result [$kernel -acquire]
#delete kernel object (automatically deletes the agent)
set result [$kernel -delete]
#don't leave bad pointers around
unset agent
unset kernel