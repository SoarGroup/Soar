# tsiInitClientControlPanel
#
# This function is called by "makeTSIControlPanel" when a ClientTSI Interface
# is to be constructed.
#
#
proc tsiInitClientControlPanel {host port topFrame} {
    global socketio_library sio_envHostname sio_envHostPort tsiConfig \
	tsiClicksPerSecond



    # SocketIO added 5/98 by S.Wallace
 
    # add the SocketIO interface, if port was 0, this was presumable called 
    # by mistake.
    if {$port != 0} {
    
	uplevel #0 [list source [file join $socketio_library socketioUtil.tcl] ]
	uplevel #0 [list source [file join $socketio_library sio-client.tcl] ]
	uplevel #0 [list source [file join $socketio_library socket.tcl] ] 
	uplevel #0 [list source [file join $socketio_library run-client.tcl] ]
      
  
    } else {
	error "Port equals 0, socketio not initialized."
    }

    if { ![string compare $host ""] } { set host $tsiConfig(socketioHost) }
    if { $port == -1 } { set port $tsiConfig(socketioPort) }
    set sio_envHostname $host
    set sio_envHostPort $port


    # modify the necessary buttons
   #    .run.quit configure -command tsiClientQuitSoar
   .run.run configure -command clientRequestRun
   .run.stop configure -command clientRequestStop
    .run.step configure -command clientRequestStep

   .agents.create.createAgentButton configure -command \
	{SIO_CreateClientAgent seeOtherProc}     

# { SIO_CreateClientAgent $newAgentName; set newAgentName {}}



    # add the stuff to the control panel for changing the listening port
    frame $topFrame.socketio -relief ridge -borderwidth 5
    pack $topFrame.socketio -side top -fill x

    frame $topFrame.socketio.connect_labels 
    pack $topFrame.socketio.connect_labels -side top -pady 3 -fill x

    label $topFrame.socketio.connect_labels.l1 -text {Connect to: }
    label $topFrame.socketio.connect_labels.host -textvariable sio_envHostname
    label $topFrame.socketio.connect_labels.l2 -text { Port: }
    label $topFrame.socketio.connect_labels.port -textvariable sio_envHostPort

    pack $topFrame.socketio.connect_labels.l1 \
	$topFrame.socketio.connect_labels.host \
	$topFrame.socketio.connect_labels.l2 \
	$topFrame.socketio.connect_labels.port -side left

    frame $topFrame.socketio.controls
    pack $topFrame.socketio.controls -side top 

    button $topFrame.socketio.controls.changeConnection -text {Change Host} \
	 -command SIO_ChangeHost
    pack $topFrame.socketio.controls.changeConnection -side left

    # For DEbugging Purposes
    set tsiConfig(dCycles) 0
    entry $topFrame.socketio.controls.dCycles -textvariable tsiConfig(dCycles)
    pack $topFrame.socketio.controls.dCycles -side bottom

}


proc SetClientOnline { name socket t } {
    global sio_agentStatus

    if $t {
	
	SIO_SendSocketCommand $socket "status online"
	set sio_agentStatus($name,online) 1
    } else {
	SIO_SendSocketCommand $socket "status offline"
        set sio_agentStatus($name,online) 0
	
    }
}
	
	

proc clientRequestRun {} {
   global sio_envSocketList tsiConfig

   if { $tsiConfig(sioDebug) > 3 } {
      puts "clientsRun :: sio_envSocketList = $sio_envSocketList"
   }

   foreach socket $sio_envSocketList {
      
      SIO_SendSocketCommand $socket "simulator-go"
   }
}

proc clientRequestStop {} {
   global sio_envSocketList tsiConfig sio_runningSimulation

   foreach socket $sio_envSocketList {
      SIO_SendSocketCommand $socket "simulator-stop"
   }
    
    #set sio_runningSimulation 0

    
}

proc clientRequestStep {} {
       global sio_envSocketList tsiConfig sio_runningSimulation

   foreach socket $sio_envSocketList {
      SIO_SendSocketCommand $socket "simulator-go 1"
   }
}
   
