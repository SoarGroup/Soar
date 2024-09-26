#
# This is a test script which tests several aspects of the Tcl SML interface
#  including kernel and agent creation, running, registering and unregistering
#  several kinds of callbacks, inreinitializing, agent destruction, and kernel
#  destruction (and maybe some other things, too).
#
# It also demonstrates how to set up a simple loop to check for events (which
#  is generally required when using CreateKernelInCurrentThread, which is
#  generally required with Tcl since it doesn't play well with multiple threads).
#
# There is a second example at the end which demonstrates using the SML stuff in
#  a slave interpreter.  This short example also shows how to add a WME to the
#  input-link.
#
# In order for this to work, the Tcl_sml_ClientInterface package must be located
#  in auto_path (i.e. the directory which contains the Tcl_sml_ClientInterface must
#  be lappend'ed to auto_path).  On Windows, assuming this file is located in
#  SoarIO/bin, this means appending the current directory.  Other platforms may
#  require some other directory to be there.  Modify the line below appropriately.
#
#
# load the SML lib from the current directory
lappend auto_path .
lappend auto_path [pwd]
package require tcl_sml_clientinterface

variable myLocation [file normalize [info script]]

if { $argc != 1 } {
    set agentDir [file join [file dirname $myLocation] SoarUnitTests]
    puts "agent directory not provided on command line; assuming $agentDir"
} else {
    set agentDir [lindex $argv 0]
}

set towersOfHanoiFile [file join $agentDir Chunking tests towers-of-hanoi-recursive towers-of-hanoi-recursive towers-of-hanoi-recursive_source.soar]
set tohTestFile [file join $agentDir TOHtest.soar]

if {[expr {![file exists $towersOfHanoiFile]}]} {
    puts stderr "Error: $towersOfHanoiFile does not exist"
    exit 1
} elseif {[expr {![file exists $tohTestFile]}]} {
    puts stderr "Error: $tohTestFile does not exist"
    exit 1
}

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

proc UpdateEventCallback {id userData kernel runFlags} {
	puts "update event fired with flags $runFlags"
}

set user_message_received 0
proc UserMessageCallback {id userData agent clientName message} {
    variable user_message_received
	set user_message_received 1
    puts "Agent $agent received UserMessage event for clientName '$clientName', userData '$userData' and message '$message'"
    # assert clientName is TestMessage and message is 'This is a "quoted" message'
    if { $clientName != "TestMessage" || $message != "This is a \"quoted\" message" } {
        puts "ERROR: UserMessage event received with incorrect parameters"
        exit 1
    }
}

# this event loop periodically checks for new events from the kernel
# ensuring that they get executed on the client side
proc CheckForEvents {k} {
	$k CheckForIncomingCommands
	after 50 CheckForEvents $k
}

# create an embedded kernel running in the current thread (running in a new thread isn't supported in Tcl because of Tcl threading issues))
set kernel [Kernel_CreateKernelInCurrentThread]

# start event loop (apparently not necessary in this example, presumably because there's no GUI, but illustrative)
CheckForEvents $kernel

set agentCallbackId0 [$kernel RegisterForAgentEvent $smlEVENT_AFTER_AGENT_CREATED AgentCreatedCallback ""]
set agentCallbackId1 [$kernel RegisterForAgentEvent $smlEVENT_BEFORE_AGENT_REINITIALIZED AgentReinitializedCallback ""]
set agentCallbackId2 [$kernel RegisterForAgentEvent $smlEVENT_BEFORE_AGENT_DESTROYED AgentDestroyedCallback ""]
set systemCallbackId [$kernel RegisterForSystemEvent $smlEVENT_BEFORE_SHUTDOWN SystemShutdownCallback ""]
set rhsCallbackId [$kernel AddRhsFunction RhsFunctionTest ""]
set updateCallbackId [$kernel RegisterForUpdateEvent $smlEVENT_AFTER_ALL_OUTPUT_PHASES UpdateEventCallback ""]
set messageCallbackId [$kernel RegisterForClientMessageEvent "TestMessage" UserMessageCallback ""]
# create an agent named Soar1
set agent [$kernel CreateAgent Soar1]

set printCallbackId [$agent RegisterForPrintEvent $smlEVENT_PRINT PrintCallback ""]
set productionCallbackId [$agent RegisterForProductionEvent $smlEVENT_BEFORE_PRODUCTION_REMOVED ProductionExcisedCallback ""]
set productionCallbackId [$agent RegisterForProductionEvent $smlEVENT_AFTER_PRODUCTION_FIRED ProductionFiredCallback ""]
set runCallbackId [$agent RegisterForRunEvent $smlEVENT_AFTER_PHASE_EXECUTED PhaseExecutedCallback ""]
set structuredCallbackId [$agent RegisterForXMLEvent $smlEVENT_XML_TRACE_OUTPUT StructuredTraceCallback ""]

# load the TOH productions
set result [$agent LoadProductions $towersOfHanoiFile]

if { $result == 0 } {
	puts "\nFAILED to load $towersOfHanoiFile"
    exit 1
}

# loads a function to test the user-defined RHS function stuff
set result [$agent LoadProductions $tohTestFile]

if { $result == 0 } {
	puts "\nFAILED to load $tohTestFile"
    exit 1
}

$kernel SendClientMessage $agent "TestMessage" "This is a \"quoted\" message"
if { $user_message_received == 0 } {
    puts "ERROR: UserMessage event not received"
    exit 1
} else {
    puts "SUCCESS: UserMessage event received"
}

$kernel UnregisterForClientMessageEvent $messageCallbackId

$agent RunSelf 2 $sml_ELABORATION

$agent UnregisterForProductionEvent $productionCallbackId
$agent UnregisterForRunEvent $runCallbackId

$kernel RunAllAgents 3

$kernel UnregisterForUpdateEvent $updateCallbackId

puts ""

# set the watch level to 0
set result [$kernel ExecuteCommandLine "watch 0" Soar1]
# excise the monitor production
set result [$kernel ExecuteCommandLine "excise towers-of-hanoi*monitor*operator-execution*move-disk" Soar1]

# run TOH the rest of the way and time it using Tcl's built-in timer
set speed [time {set result [$agent RunSelfForever]}]
puts "\n$speed"

# the output of "print s1" should contain "^rhstest success"
if { [string first "^rhstest success" [$kernel ExecuteCommandLine "print s1" Soar1]] == -1 } {
	puts "\nRHS test FAILED"
} else {
	puts "\nRHS test SUCCEEDED"
}

set result [$kernel ExecuteCommandLine "init-soar" Soar1]

set input_link [$agent GetInputLink]
if {[catch {$input_link GetParameterValue "non-existent"} msg]} {
   puts "✅Correctly caught error:\n$errorInfo\n"
} else {
    puts "❌ Catching error FAILED"
}

$kernel DestroyAgent $agent

# remove all the remaining kernel callback functions (not required, just to test)
puts "removing callbacks"
$kernel UnregisterForAgentEvent $agentCallbackId0
$kernel UnregisterForAgentEvent $agentCallbackId1
$kernel UnregisterForAgentEvent $agentCallbackId2
$kernel UnregisterForSystemEvent $systemCallbackId
$kernel RemoveRhsFunction $rhsCallbackId

# shutdown the kernel; this makes sure agents are deleted and events fire correctly
$kernel Shutdown

# delete kernel object
set result [$kernel -delete]
# don't leave bad pointers around
unset agent
unset kernel

# Here's an example for those who want to execute commands in a slave interpreter.
# This is important for some environments (i.e. Tcl Eaters created each agent in a separate slave interpreter)
# This also demonstrates adding something to the input-link (which we don't do above)

# create slave
interp create red

# load package in slave
red eval lappend auto_path .
red eval lappend auto_path [pwd]
red eval package require tcl_sml_clientinterface

# create kernel and agent
set kernel [red eval Kernel_CreateKernelInNewThread]
set agent [red eval $kernel CreateAgent Soar1]

# add wme to input-link
set input_link [red eval $agent GetInputLink]
set wme [red eval $agent CreateIdWME $input_link test]

# commit the changes to working memory
red eval $agent Commit

# print the timetag
# note that the timetag will be negative since it's a client-side timetag
# client-side timetags don't match kernel-side timetags for performance reasons
set timetag [red eval $wme GetTimeTag]
puts "timetag = $timetag"

# shutdown the kernel; this makes sure agents are deleted and events fire correctly
red eval $kernel Shutdown
# delete the kernel
red eval $kernel -delete

# delete the interp
interp delete red

# don't leave bad pointers around
unset agent
unset kernel

