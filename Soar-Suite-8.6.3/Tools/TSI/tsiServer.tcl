
# SocketIO Procedures added 5/98		S.Wallace
proc tsiInitServerControlPanel {{port -1} {topFrame {}}} {
   global socketio_library sio_listeningPort
   
   # SocketIO added 5/98 by S.Wallace
   set sio_listeningPort $port
   
   # add the SocketIO interface
   if {$port != 0} {
      uplevel #0 [list source [file join $socketio_library socketioUtil.tcl] ] 
      uplevel #0 [list source [file join $socketio_library socket.tcl] ]
      

      # add the stuff to the control panel for changing the listening port
      frame $topFrame.socketio -relief ridge -borderwidth 5

      
      label $topFrame.socketio.label1 -text {Remote Agents Port:} 
      label $topFrame.socketio.listenPort -textvariable sio_listeningPort
      button $topFrame.socketio.changePort -text {Change Port} -command \
	 SIO_ChangePort
    
      pack $topFrame.socketio.label1 $topFrame.socketio.listenPort -side left
      pack $topFrame.socketio.changePort -side right
      

      pack $topFrame.socketio -side bottom -fill x      
      SIO_CreateListeningSocket  
      
      

   } else {
      error "Port equals 0, socketio not initialized."
   }
}


proc createNewRemoteAgent {name socket} {
   global auto_path tsi_library soar_library soar_doc_dir \
       tsiConfig remoteAgents sio_agentStatus

   if { $tsiConfig(sioDebug) > 3 } {puts "createNewRemoteAgent -- (begin)"}
    if [catch "interp create $name"] {
      error "Cannot create agent named '$name'"
   }

    
    set remoteAgents($name) $socket
    

   load {} Tk $name
   $name eval [list set auto_path $auto_path]
 
   $name eval [list set tsi_library $tsi_library]
   if [info exists soar_doc_dir] {
      $name eval [list set soar_doc_dir $soar_doc_dir]
   }
   ### Configure the agent for the TSI
   set id [array startsearch tsiConfig]
   while {[array anymore tsiConfig $id]} {
      set elm [array nextelement tsiConfig $id]
      $name eval [list set tsiConfig($elm) $tsiConfig($elm)]
   }

   array donesearch tsiConfig $id
    $name eval [list set interp_name $name]
    $name eval [list set interp_agentSocket $socket]
  
    # Share socket w/ slave, THIS May be dangerous.
   # interp share {} $socket $name

   $name alias registerWithController registerAgent $name
   $name alias tsiListAgents tsiListAgents
    $name eval tsiInitRemoteAgent
   $name alias SIO_SendSocketCommand SIO_SendSocketCommand $socket
    $name alias SIO_SendTupleToAgent SIO_SendTupleToAgent $socket

   tsiSetupAgentVars $name

   set sio_agentStatus($name,online) 0
   set sio_agentStatus($name,inputMode) initialize

    if { $tsiConfig(sioDebug) > 3 } {
	puts "CreateNewRemoteAgent -- (end)"
    }

}


proc remoteAgentRemoveWME { agent timeTag } {
   global sio_agentToSocket sio_timeTagToTuple

   eval "SIO_SendTupleToAgent $sio_agentToSocket($agent) \
      $sio_timeTagToTuple($timeTag) 2"

}



# remoteAgentAddWME :
#
# This procedure is addFN corresponding to all remote agents. TCL 
# environments built for use with sremote agents, or for use with TSI3.0
# and higher should no longer use the standard Soar commands:
# "add-wme" and "remove-wme"  instead, the corresponding add and remove
# functions are defined based on the particular agent type.  Thus,
# occurances of "add-wme" should be replaced by the agent type specific
# add function.  This function can be found using the array value:
# tsiAgentInfo($tsiAgentInfo($agent_name),addFN)
# Occurances of "remove-wme" should be replaced similiarly, using 'rmFN'
# instead of 'addFN'
#
proc remoteAgentAddWME { agent objectID attribute value } {
   global sio_agentToSocket sio_IDs tsiConfig sio_timeTagToTuple

    if { ![string match {SIO_IDENT:*} $objectID] } {
	
	puts "WARNING: Improperly formatted Object ID: '$objectID'"
    } else {
	set objectID [lindex [split $objectID :] 1]

    }

   if { $tsiConfig(sioDebug) } {
	puts "Remote Agent ADD ($objectID $attribute $value)" 
   }
   if { ![info exists sio_IDs] } {
      set sio_IDs(input) 1
      set sio_IDs(output) -1
      
      # First two timeTags are reserved for input & output links.
      set sio_IDs(timeTag) 2
   }
   
   if { ![string compare $attribute ^input-link] } {
      if { $tsiConfig(sioDebug) > 5 } {
	 puts "Requested addition of input-link..."
      }
      
      return "1: na ^input-link SIO_IDENT:1"
   } 
   if { ![string compare $attribute ^output-link] } {
      if { $tsiConfig(sioDebug) > 5 } {
	 puts "Requested addition of output-link..."
      }
      
      return "2: na ^output-link SIO_IDENT:-1"
   }
   
   # now take the ^ off of the attribute name.
   set attribute [string range $attribute 1 end]
   
   if { ![string compare $value *] } {
      # We are requesting that a new Object ID be constructed...
      
      
      
      if { $objectID > 0 } {
	 incr sio_IDs(input)
	 incr sio_IDs(timeTag)
	 
	 # We have created a new ObjectID and Timetag, now associate them.
	 set sio_timeTagToTuple($sio_IDs(timeTag)) \
	    [list $objectID $attribute 1 $sio_IDs(input)]
	 
	 # The tuple : ( ID, attribute, value-type, value, action )
	 SIO_SendTupleToAgent $sio_agentToSocket($agent) $objectID \
	    $attribute 1 $sio_IDs(input) 1
	 
	 if { $tsiConfig(sioDebug) > 5 } {
	    puts "$sio_IDs(timeTag): $objectID ^$attribute SIO_IDENT:$sio_IDs(input)"
	 }
	 return "$sio_IDs(timeTag): $objectID ^$attribute SIO_IDENT:$sio_IDs(input)"
	 
      } else {

	 puts "*** Warning! : adding to output link of remote agent."
	 incr sio_IDs(output) -1
	 incr sio_IDs(timeTag) 
	 
	 
	 set sio_timeTagToTuple($sio_IDs(timeTag)) \
	    [list $objectID $attribute 1 $sio_IDs(output)]
	 
	 SIO_SendTupleToAgent $sio_agentToSocket($agent) $objectID \
	    $attribute 1 $sio_attributeObjectID(output) 1
	 
	 if { $tsiConfig(sioDebug) > 5 } { 
	    puts "$sio_IDs(timeTag): $objectID ^$attribute SIO_IDENT:$sio_IDs(output)"
	 }
	 return "$sio_IDs(timeTag): $objectID ^$attribute SIO_IDENT:$sio_IDs(output)"
	 
      }
  } elseif { [string match {SIO_IDENT:*} $value] } {
      # A reference to a previously added object is being made, 
      # Unfortunately, this method (which is necessary) means that
      # a user cannot create a string which matches this pattern w/o
      # encountering serious problems.

      SIO_SendTupleToAgent $sio_agentToSocket($agent) $objectID \
	  $attribute 1 [lindex [split $value :] 1] 1

      if { $tsiConfig(sioDebug) > 5 } { 
	  puts "$sio_IDs(timeTag): $objectID ^$attribute SIO_IDENT:$sio_IDs(output)"
      }

      return "$sio_IDs(timeTag): $objectID ^$attribute SIO_IDENT:$sio_IDs(output)"
      


  } else {
      # We are adding a String value (not object) to the attribute
      
      if { $objectID < 0 } {
	 puts "*** Warning! : adding to output link of remote agent."
      }
      
      incr sio_IDs(timeTag)


      set sio_timeTagToTuple($sio_IDs(timeTag)) \
	 [list $objectID $attribute 2 $value]

      SIO_SendTupleToAgent $sio_agentToSocket($agent) $objectID \
	 $attribute 2 $value 1
      
      if { $tsiConfig(sioDebug) > 5 } {
	 puts "$sio_IDs(timeTag): $objectID ^$attribute $value"
      }
      return "$sio_IDs(timeTag): $objectID ^$attribute $value"
      
      
   }
}


proc tsiOnEnvironmentStop {} {
    global remoteAgents tsiSimulationState localAgents

    
    puts " ******** tsiOnEnvironmentStop ********* $tsiSimulationState(running)"

    if { $tsiSimulationState(running) } {
	foreach {agent socket} [array get remoteAgents] {
	    
	    #for now, all agents receive simulation stop (even off line ones.)
	    SIO_SendSocketCommand $socket stopping-simulation
	}

	set tsiSimulationState(running) 0
	return 1
    }

    return 0
}

    
proc tsiOnEnvironmentRun {} {
   global remoteAgents sio_agentStatus sio_agentToSocket \
    tsiSimulationState


    puts "************ SERVER RUN **************** $tsiSimulationState(running) "
    if { !$tsiSimulationState(running) } {
	foreach {agent socket} [array get remoteAgents] {
	    
	    if { $sio_agentStatus($agent,online) } {
		SIO_SendSocketCommand $socket starting-simulation
	    }
	}
	set tsiSimulationState(running) 1
	return 1
    }

    return 0
}


proc tsiOnEnvironmentStep {} {
    global tsiSimulationState remoteAgents sio_agentStatus
    
    puts "tsiOnEnvironmentStep -- $tsiSimulationState(running)"
    if { !$tsiSimulationState(running) } {
	puts "NOT  RUNNING ------ STEP"

	foreach {agent socket} [array get remoteAgents] {
	    
puts "Agent $agent -- online? $sio_agentStatus($agent,online)"
	    if { $sio_agentStatus($agent,online) } {
		SIO_SendSocketCommand $socket starting-simulation
	    }
	}
	set tsiSimulationState(running) 1
	return 1
    }
    return 0
}


proc SIO_ChangePort {} {
}

