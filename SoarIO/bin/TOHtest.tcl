proc PrintCallback {id userData agent message} {
   puts -nonewline $message
}

proc ProductionExcisedCallback {id userData agent prodName instantiation} {
   puts "removing $prodName"
}

proc PhaseExecutedCallback {id userData agent phase} {
	puts "phase $phase executed"
}

proc AgentDestroyedCallback {id userData agent} {
	puts "[$agent GetAgentName] destroyed"
}

#load the sml stuff
lappend auto_path .
package require tcl_sml_clientinterface

#create an embedded kernel running in the kernel's thread (so we don't have to poll for messages)
set kernel [Kernel_CreateEmbeddedConnection KernelSML 0]
#create an agent named Soar1
set agent [$kernel CreateAgent Soar1]

set result [$agent RegisterForPrintEvent $smlEVENT_PRINT PrintCallback "" ""]
if { [expr [string length $result] != 0] } { puts $result }
set result [$agent RegisterForProductionEvent $smlEVENT_BEFORE_PRODUCTION_REMOVED ProductionExcisedCallback "" ""]
if { [expr [string length $result] != 0] } { puts $result }
#set result [$agent RegisterForRunEvent $smlEVENT_AFTER_PHASE_EXECUTED PhaseExecutedCallback "" ""]
#if { [expr [string length $result] != 0] } { puts $result }
set result [$agent RegisterForAgentEvent $smlEVENT_BEFORE_AGENT_DESTROYED AgentDestroyedCallback "" ""]
if { [expr [string length $result] != 0] } { puts $result }

cd demos/towers-of-hanoi
#load the TOH productions
set result [$agent LoadProductions towers-of-hanoi.soar]
#set the watch level to 0
set result [$kernel ExecuteCommandLine "watch 0" Soar1]
#excise the monitor production
set result [$kernel ExecuteCommandLine "excise towers-of-hanoi*monitor*operator-execution*move-disk" Soar1]

#run TOH and time it using Tcl's built-in timer
set speed [time {set result [$kernel ExecuteCommandLine "run 2048" Soar1]}]
cd ../..
puts "\n$speed"

#give Tcl object ownership of underlying C++ object so when we delete the Tcl object they both get deleted
set result [$kernel -acquire]
#delete kernel object (automatically deletes the agent)
set result [$kernel -delete]
#don't leave bad pointers around
unset agent
unset kernel