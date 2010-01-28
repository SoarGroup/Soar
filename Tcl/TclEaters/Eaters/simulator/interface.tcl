#
#
# $Id$
# $Log$
# Revision 1.1  2005/06/01 19:14:13  rmarinie
# initial commit of sml eaters
#
# Revision 1.2  2003/10/21 18:26:42  snason
#  hopefully eliminated error message after init-soar
#
# Revision 1.1.1.1  2003/06/16 13:48:36  swallace
# eaters initial cvs version (3.0.5)
#
# Revision 8.7  1998/10/25 14:52:30  swallace
# Synced w/ Mazin
#
# Revision 8.6  1998/10/08 13:54:45  swallace
# Released to John
#
# Revision 8.5  1998/10/07 14:24:12  swallace
# Updates an enemy eater's sensor entry properly
#
# Revision 8.4  1998/10/07 13:12:12  swallace
# 8.1 Safe
#
# Revision 8.3  1998/10/05 12:55:48  swallace
# Release 2.0a (w/ new tsi30alpha) and new control panel etc.
#
# Revision 8.2  1998/10/02 13:49:18  swallace
# *** empty log message ***
#
# Revision 8.1  1998/09/18 16:44:35  swallace
# Works fine, but code is dirty.  About to clean up.
#
# Revision 6.4  1998/08/19 15:44:12  swallace
# proprioceptic feedback added - movecount remains
#
# Revision 6.3  1998/08/19 13:03:54  swallace
#  All Fixes :) except for propreoceptic feedback
#
# Revision 6.2  1998/07/22 13:47:15  swallace
# removed an overlooked $addFN and replaced w/ tsiAgentInfo(agent,addFN)
#
# Revision 6.1  1998/06/02 19:02:32  swallace
#  allows concurrent remote/local agents.
#
# Revision 5.1  1998/06/02 16:51:09  swallace
# 2 side step & run
#
# Revision 4.1  1998/06/01 22:14:54  swallace
# works using step on both sides.
#
# Revision 1.2  1998/05/28 20:20:52  swallace
# ready to work on WME wrappers
#
#

proc getSimulatorInfo {} {
   
   set info(name) Eaters
   set info(version) 1.3
   set info(autoAddOutputLnk) t
   set info(outputCycle) elaboration

   return info
}


proc eatersAgentAddWME {agent objectID attribute value type} {
   # this proc is new for the SML.  For old non-SML versions
   # tsiAgentInfo($agent,addFN) should be set to localAgentAddWME
   # as defined in tsiInit.tcl  

##   global tsiConfig, ioID, inputLinkID
   global tsiConfig 

   if { $tsiConfig(sioDebug) > 4 } {
     puts "Adding WME to local agent: ($agent $objectID $attribute $value $type)"
   }

   # strip off any ^ in the attribute
   regsub {^\^} $attribute "" attribute

   switch -exact $type {
     wme_int    {
	 return [$agent eval [list soar_agent CreateIntWME $objectID $attribute $value ]]}
     wme_string {
	 return [$agent eval [list soar_agent CreateStringWME $objectID $attribute $value ]]}
     wme_float  {
	 return [$agent eval [list soar_agent CreateFloatWME $objectID $attribute $value ]]}
     wme_id     {
	 return [$agent eval [list soar_agent CreateIdWME $objectID $attribute]] }
     wme_sharedID {
	 return [$agent eval [list soar_agent CreateSharedIdWME $objectID $attribute $value ]]}
   
       default {

   ###KJC
   puts "\ninterface.tcl(eatersAgentAddWME): shouldn't get here..."
   return [$agent eval [list soar_agent ExecuteCommandLine "add-wme $objectID $attribute $value"]]
}
   }
}

proc eatersAgentRemoveWME { agent pWME } {
   # this proc is new for the SML.  For old non-SML versions
   # tsiAgentInfo($agent,rmFN) should be set to localAgentRemoveWME
   # as defined in tsiInit.tcl  

 ##   global tsiConfig, ioID, inputLinkID
   global tsiConfig

   if { $tsiConfig(sioDebug) > 4 } {
     puts "Removing WME from local agent: ($pWME)"
   }
   $agent alias tt_object $pWME
   $agent eval [list soar_agent DestroyWME $pWME ]
   $agent alias tt_object {}

   ###KJC   return [$agent eval [list soar_agent ExecuteCommandLine "remove-wme $pWME "]
}

proc updateOutputLink {agent} {
   global outputLinkID interp_name tsiAgentInfo

   #  Eaters output commands are:
   #      move ^direction <direction> 
   #      jump ^direction <direction> 
   #      

   # new output processing code for SML
    set numCmds [$agent eval [list soar_agent GetNumberCommands]]
    #puts "\n updateOutputLink: numCmds = $numCmds "
    if {$numCmds == "" || $numCmds == 0 } { return }
    for {set i 0} {$i < $numCmds} {incr i} {
	set cmd [$agent eval [list soar_agent GetCommand $i]]
        $agent alias cmd_id $cmd
        set cmdName [$agent eval [list cmd_id GetCommandName]]
        set walkdir [$agent eval [list cmd_id GetParameterValue "direction"]]
        set isComplete [$agent eval [list cmd_id GetParameterValue "status"]]
	if {$cmdName == "move"} {
	    #puts "set [$agent eval [list set interp_name]] move $walkdir"
	    if {$walkdir == "" || $isComplete == "complete" } {
#		 puts "CANNOT FIND WALK DIRECTION OR ALREADY APPLIED 1!!!"
	     } else {
		 ##setWalkDir $walkdir [$agent eval [list set cmd_id]]
		 $agent eval [list setWalkDir $walkdir $cmd]
#		 puts "WALK DIRECTION is $walkdir"
	     }
	} else {
	    #puts "set [$agent eval [list set interp_name]] jump $walkdir"
	    if {$cmdName == "jump"} {
		if {$walkdir == "" || $isComplete == "complete" } {
		    # puts "CANNOT FIND WALK DIRECTION OR ALREADY APPLIED 1!!!"
		} else {
		    $agent eval [list setJumpDir $walkdir $cmd]
		    # puts "JUMP DIRECTION is $walkdir"
		}
	    }
	}
        $agent alias cmd_id {}
    }
}

proc setUpInputLink {agent ioID inID outID} {
   ## we don't need this for SML, doesn't do much real work anymore
   ## used to be done whenever "top-state-just-created"
   global inputLinkWME inputLinkID tsiConfig tsiAgentInfo
   if { $tsiConfig(sioDebug) > 3 } { 
      puts "setUpInputLink -- (begin) agent: $agent" 
   }
   if { $tsiConfig(sioDebug) > 4 } { 
      puts "AddWME Function is: $tsiAgentInfo($agent,addFN)" 
   }
   set inputLinkID($agent) [$agent eval [list soar_agent GetInputLink]]
   set outputLinkID($agent) [$agent eval [list soar_agent GetOutputLink]]
   
   #SML can't do next line since setUpInputLink called at agent creation...
   #updateInputLink $agent
   if { $tsiConfig(sioDebug) > 3 } {
      puts "setUpInputLink -- (end) agent: $agent"   
   }
}

proc updateInputLink {agent} {
   global inputSensorRange sensorRange eaterSelfWME eaterObject deleteList \
          currentBumpList entityList eaterX eaterY eaterDir eaterScore \
          agentMoved numberOfMoves inputLinkID tsiAgentInfo \
       actionID tsiConfig

 if { $tsiConfig(sioDebug) > 3 } { 
         puts "updateInputLink -- (begin) agent: $agent" 
   }
 
    ### See if there are any objects we need to delete WME's for
   if [info exists deleteList($agent)] {
      foreach obj $deleteList($agent) {
         ### Currently we don't see other eaters, so this won't happen
         if {[lsearch [.wGlobal.c itemcget $obj -tags] eater] >= 0} {
#            puts "************ REMOVE EATER WME 1 ***************"
	     removeEaterWMEs $obj $agent 
         } else {
            removeSimpleObjectWMEs $obj $agent 
         }
      }
      set deleteList($agent) ""
   }

   ### If any bumps popped up, add them to the input link
   if [info exists currentBumpList] {
      foreach i $currentBumpList {
         set obj [lindex $i 0]
         if ![info exists entityList($obj,$agent)] {
            addBumpWMEs $obj [lindex $i 1] $agent
         }
      }
   }

   ### For now, we only create an eater WME for ourselves
   if ![info exists eaterSelfWME($eaterObject($agent),$agent)] { 
       addEaterWME $eaterObject($agent) $agent $agent 
   }
   ### Now update my eater's information, creating new WMEs if needed
   changeEaterXWME $eaterObject($agent) $agent \
                   [convertToLogical $eaterX($agent)]

   ### convertToLogical::  return [expr int($n/$gridSize)]

   changeEaterYWME $eaterObject($agent) $agent \
                   [convertToLogical $eaterY($agent)]
   changeEaterDirWME $eaterObject($agent) $agent $eaterDir($agent)
   changeEaterScoreWME $eaterObject($agent) $agent $eaterScore($agent)

   ### Set up the my-location WME if not already there...

   ### Check whether we have built the sensor grid on the input link yet
   if ![info exists inputSensorRange($agent)] {
      set inputSensorRange($agent) -1
   }


   ### Now check whether the sensor range has changed and build or
   ### delete the necessary layers
   ### This will work because a side-effect of addSensorLayer and
   ### deleteSensorLayer is to update the value of inputSensorRange
   while {$inputSensorRange($agent) < $sensorRange} {
      addSensorLayer $agent
   }
   while {$inputSensorRange($agent) > $sensorRange} {
      deleteSensorLayer $agent
   }

   ### Finally, scan the sensors and update WME's appropriately
   updateSensorWMEs $agent
 
   if {[info exists agentMoved($agent)] && $agentMoved($agent)} {
       
       unset agentMoved($agent)
       if { ![info exists actionID($agent)] } {
	   if { $numberOfMoves($agent) != 0 } {
	       error "No Corresponding ActionID for agent $agent"
	   }
       } else {
           $agent alias move_ID $actionID($agent)
           $agent eval [list move_ID AddStatusComplete]
	  ## $tsiAgentInfo($agent,addFN) $agent $actionID($agent) \
	       "^status" "complete" wme_string
           $agent alias move_ID {}
	   unset actionID($agent)
       } 
   }
   if { $tsiConfig(sioDebug) > 3 } { 
       puts "\nCommitting changes to inputLink..."
   }
    $agent eval [list soar_agent Commit]
}  ; # end proc updateInputLink 

proc removeInputLink {agent} {
   global inputLinkWME inputLinkID deleteList currentBumpList entityList \
          eaterSelfWME eaterNameWME eaterSelfID \
          eaterXWME eaterXVal eaterYWME eaterYVal eaterDirWME eaterNWME \
          eaterDirVal eaterNVal  eaterScoreWME eaterScoreVal \
          sensorRange sensorID northWME southWME westWME eastWME \
          typeOfObj natureWME otherEaterColorWME otherEaterScoreWME \
          sensorEater agentMoved inputSensorRange \
          eaterObject tsiConfig moveDir jumpDir
   
   if { $tsiConfig(sioDebug) > 3 } { 
         puts "removeInputLink -- (begin) agent: $agent" 
   }
   catch "unset inputLinkWME($agent)"
   catch "unset inputLinkID($agent)"
   catch "unset deleteList($agent)"
   set agentMoved($agent) 0
   catch "unset inputSensorRange($agent)"
   if [info exists moveDir($agent)] {
        catch "unset moveDir($agent)"
        }
   if [info exists jumpDir($agent)] {
        catch "unset jumpDir($agent)"
        }
   if [info exists currentBumpList] {
      foreach i $currentBumpList {
         set obj [lindex $i 0]
         catch "unset entityList($obj,$agent)"
      }
   }
   ### For now we are the only eater on the input link
   set obj $eaterObject($agent)
   catch "unset eaterSelfWME($obj,$agent)"
   catch "unset eaterNameWME($obj,$agent)"
   catch "unset eaterSelfID($obj,$agent)"
   catch "unset eaterXWME($obj,$agent)"
   catch "unset eaterXVal($obj,$agent)"
   catch "unset eaterYWME($obj,$agent)"
   catch "unset eaterYVal($obj,$agent)"
   catch "unset eaterNWME($obj,$agent)"
   catch "unset eaterNVal($obj,$agent)"
   catch "unset eaterDirWME($obj,$agent)"
   catch "unset eaterDirVal($obj,$agent)"
   catch "unset eaterScoreWME($obj,$agent)"
   catch "unset eaterScoreVal($obj,$agent)"

   ### Now do all the possible stuff in sensor locations
   for {set i -$sensorRange} {$i<= $sensorRange} {incr i} {
      for {set j -$sensorRange} {$j<= $sensorRange} {incr j} {
         catch "unset sensorID($i,$j,$agent)"
         catch "unset northWME($i,$j,$agent)"
         catch "unset southWME($i,$j,$agent)"
         catch "unset westWME($i,$j,$agent)"
         catch "unset eastWME($i,$j,$agent)"
         catch "unset typeOfObj($i,$j,$agent)"
         catch "unset natureWME($i,$j,$agent)"
         catch "unset otherEaterColorWME($i,$j,$agent)"
         catch "unset otherEaterScoreWME($i,$j,$agent)"
         catch "unset sensorEater($i,$j,$agent)"
      }
   }
}

proc addBumpWMEs {obj names agent } {
   global inputLinkID entityList tsiAgentInfo

    ## CreateIdWME returns a pID
   set wme [$tsiAgentInfo($agent,addFN) $agent $inputLinkID($agent) bump * wme_id]
   $agent alias id_object $wme
   # get the TimeTag 
   # set ttag [$agent eval [list id_object GetTimeTag]]
   # set entityList($obj,$agent) $ttag  ; #it's a negative number, but it's ok
    
   # SML wants us to save a pWME, not TimeTag
   set entityList($obj,$agent) $wme
   foreach name $names {
      $tsiAgentInfo($agent,addFN) $agent $wme ^eater $name wme_string
      ####KJC  should that be a wme_sharedID???  check 8.5.2 links
   }
    $agent alias id_object {}
}

proc removeSimpleObjectWMEs {obj agent } {
   global entityList tsiAgentInfo
   
   if [info exists entityList($obj,$agent)] {
      $tsiAgentInfo($agent,rmFN) $agent $entityList($obj,$agent)
      unset entityList($obj,$agent)
   }
}

proc addEaterWME {obj name agent } {
   global eaterSelfWME eaterSelfID inputLinkID tsiAgentInfo

    ## CreateIdWME returns a pID
    set wme [$tsiAgentInfo($agent,addFN) $agent $inputLinkID($agent) ^eater * wme_id]
    #$agent alias id_object $wme
    set eaterSelfID($obj,$agent) $wme

    # get the TimeTag 
    #set ttag [$agent eval [list id_object GetTimeTag]]
    #set eaterSelfWME($obj,$agent) $ttag;  #it's a negative number, but it's ok
    
    # SML wants us to store the pWME, not TimeTag
    set eaterSelfWME($obj,$agent) $wme

    $tsiAgentInfo($agent,addFN) $agent $wme ^name $name wme_string
  
    #$agent alias id_object  {}
}

proc removeEaterWMEs {obj name agent } {
   global eaterSelfWME eaterSelfID eaterNameWME eaterXWME eaterYWME \
          eaterDirWME eaterScoreWME tsiAgentInfo

   if [info exists eaterSelfWME($obj,$agent)] {
      $tsiAgentInfo($agent,rmFN) $agent $eaterSelfWME($obj,$agent)
      unset eaterSelfWME($obj,$agent) 
      catch "unset eaterSelfID($obj,$agent)"
   }
   catch "unset eaterNameWME($obj,$agent)"
   catch "unset eaterXWME($obj,$agent)"
   catch "unset eaterYWME($obj,$agent)"
   catch "unset eaterDirWME($obj,$agent)"
   catch "unset eaterScoreWME($obj,$agent)"
}


proc changeEaterXWME {obj agent {val ""}} {
   global eaterXWME eaterSelfID eaterXVal tsiAgentInfo

   ### could change to use SML Update method if keep ptr,
   ### but this routine already nicely deletes or adds or updates

   # if nothing changed, just return
   if {[info exists eaterXVal($obj,$agent)] && \
       ($eaterXVal($obj,$agent) == $val)} {
      return
   }
   # either need to delete, add, or update, so first remove WME
   if [info exists eaterXWME($obj,$agent)] {
      $tsiAgentInfo($agent,rmFN) $agent $eaterXWME($obj,$agent)
      unset eaterXWME($obj,$agent)
      catch "unset eaterXVal($obj,$agent)"
   }
   # value exists, so add the new value
   if {($val != "") && ([info exists eaterSelfID($obj,$agent)])} {
       set wme [$tsiAgentInfo($agent,addFN) $agent \
		  $eaterSelfID($obj,$agent) ^x $val wme_int ]
       # save the pWME (not TimeTag) and value
       #$agent alias id_object $wme
#       set eaterXWME($obj,$agent)  [$agent eval [list id_object GetTimeTag]]
       set eaterXWME($obj,$agent) $wme
       set eaterXVal($obj,$agent) $val

       #$agent alias id_object {} 
   }
}

proc changeEaterYWME {obj agent {val ""}} {
   global eaterYWME eaterSelfID eaterYVal tsiAgentInfo

   # if nothing changed, just return
   if {[info exists eaterYVal($obj,$agent)] && \
       ($eaterYVal($obj,$agent) == $val)} {
      return
   }
#   puts "changeEaterYWME: obj =$obj   agent = $agent   val = $val" 
   # either need to delete, add, or update, so first remove WME
   if [info exists eaterYWME($obj,$agent)] {
      $tsiAgentInfo($agent,rmFN) $agent $eaterYWME($obj,$agent)
      unset eaterYWME($obj,$agent)
      catch "unset eaterYVal($obj,$agent)"
   }
   if {($val != "") && ([info exists eaterSelfID($obj,$agent)])} {
      set wme [$tsiAgentInfo($agent,addFN) $agent \
		  $eaterSelfID($obj,$agent) ^y $val wme_int]
      # store the pWME and value
      #$agent alias id_object $wme
#      set eaterYWME($obj,$agent)  [$agent eval [list id_object GetTimeTag]]
#      set eaterYWME($obj,$agent) [$agent eval [list set id_object]]
      set eaterYWME($obj,$agent) $wme
      set eaterYVal($obj,$agent) $val

       #$agent alias id_object {}
   }
}

proc changeEaterDirWME {obj agent {val ""}} {
   global eaterDirWME eaterSelfID eaterDirVal tsiAgentInfo

   # if nothing changed, just return
   if {[info exists eaterDirVal($obj,$agent)] && \
       ($eaterDirVal($obj,$agent) == $val)} {
        return
   }
   # either need to delete, add, or update, so first remove WME
   if [info exists eaterDirWME($obj,$agent)] {
      $tsiAgentInfo($agent,rmFN) $agent $eaterDirWME($obj,$agent)
      unset eaterDirWME($obj,$agent)
      catch "unset eaterDirVal($obj,$agent)"
   }
   if {($val != "") && ([info exists eaterSelfID($obj,$agent)])} {
      set wme [$tsiAgentInfo($agent,addFN) $agent \
		  $eaterSelfID($obj,$agent) ^direction $val wme_string]
      # store the TimeTag and value
      #$agent alias id_object $wme
      # set eaterDirWME($obj,$agent) [$agent eval [list id_object GetTimeTag]]

      # SML wants us to store the pWME and value
       set eaterDirWME($obj,$agent) $wme
       set eaterDirVal($obj,$agent) $val
      # $agent alias id_object {}
   }
}

proc changeEaterScoreWME {obj agent {val ""}} {
   global eaterScoreWME eaterSelfID eaterScoreVal tsiAgentInfo eaterScore

   # if nothing changed, just return
   if {[info exists eaterScoreVal($obj,$agent)] && \
       ($eaterScoreVal($obj,$agent) == $val)} {
      return
   }
   # either need to delete, add, or update, so first remove WME
   if [info exists eaterScoreWME($obj,$agent)] {
      $tsiAgentInfo($agent,rmFN) $agent $eaterScoreWME($obj,$agent)
      unset eaterScoreWME($obj,$agent)
      catch "unset eaterScoreVal($obj,$agent)"
   }
   if {($val != "") && ([info exists eaterSelfID($obj,$agent)])} {
      set wme [$tsiAgentInfo($agent,addFN) $agent \
		  $eaterSelfID($obj,$agent) ^score $val wme_int]
      # store the TimeTag and value
      #$agent alias id_object $wme
      #set eaterScoreWME($obj,$agent) [$agent eval [list id_object GetTimeTag]]

       # SML wants the pWME not TimeTag
      set eaterScoreWME($obj,$agent) $wme
      set eaterScoreVal($obj,$agent) $val
      set eaterScore($agent) $val   
      # $agent alias id_object {}
   }
}

proc addSensorLayer {agent} {
   global inputSensorRange inputLinkID myLocationID sensorID \
          northWME southWME westWME eastWME tsiAgentInfo

   incr inputSensorRange($agent)
   if !$inputSensorRange($agent) {
      if ![info exists sensorID(0,0,$agent)] {
         set wme [$tsiAgentInfo($agent,addFN) $agent \
		     $inputLinkID($agent) ^my-location * wme_id]
	  # store the TimeTag and ptr to ID
	  #$agent alias id_object $wme
	  #set myLocationWME($agent) [$agent eval [list id_object GetTimeTag]]
	  set myLocationWME($agent) $wme
	  set sensorID(0,0,$agent) $wme
	  #$agent alias id_object {}
      }
   } elseif {$inputSensorRange($agent) > 0} {
      ### Add top edge
      set j -$inputSensorRange($agent)
      for {set i [expr 1-$inputSensorRange($agent)]} \
          {$i <= [expr $inputSensorRange($agent)-1]} \
          {incr i} {
	  set wme [$tsiAgentInfo($agent,addFN) $agent \
		   $sensorID($i,[expr $j+1],$agent) ^north * wme_id]
	  # store the TimeTag and ptr to ID
	  #$agent alias id_north $wme
	  #set northWME($agent) [$agent eval [list id_north GetTimeTag]]
          
          # SML wants us to use pWMEs
	  set northWME($agent) $wme
	  set sensorID($i,$j,$agent) $wme
	      #$agent alias id_north {}

         ### Don't need to save these, because they should get garbage
         ### collected when we delete by layers.
         $tsiAgentInfo($agent,addFN) $agent $sensorID($i,$j,$agent) ^south \
                       $sensorID($i,[expr $j+1],$agent) wme_sharedID
         ### Link to previous neighbor on this layer
         if {$i > [expr 1-$inputSensorRange($agent)]} {
            $tsiAgentInfo($agent,addFN) $agent $sensorID($i,$j,$agent) ^west \
                          $sensorID([expr $i-1],$j,$agent) wme_sharedID
            $tsiAgentInfo($agent,addFN) $agent \
	                  $sensorID([expr $i-1],$j,$agent) ^east \
                          $sensorID($i,$j,$agent) wme_sharedID
         }
      }
      ### Add bottom edge
      set j $inputSensorRange($agent)
      for {set i [expr 1-$inputSensorRange($agent)]} \
          {$i <= [expr $inputSensorRange($agent)-1]} \
          {incr i} {
	 set wme [$tsiAgentInfo($agent,addFN) $agent \
		     $sensorID($i,[expr $j-1],$agent) ^south * wme_id]
         # store the TimeTag and ptr to ID
         #$agent alias id_south $wme
	 #set southWME($agent) [$agent eval [list id_south GetTimeTag]]

	 # SML wants us to use pWMEs, not TimeTags
	 set southWME($agent) $wme
         set sensorID($i,$j,$agent) $wme
	      #$agent alias id_south  {}
    
         ### Don't need to save these, because they should get garbage
         ### collected when we delete by layers.
         $tsiAgentInfo($agent,addFN) $agent $sensorID($i,$j,$agent) ^north \
	               $sensorID($i,[expr $j-1],$agent) wme_sharedID
         ### Link to previous neighbor on this layer
         if {$i > [expr 1-$inputSensorRange($agent)]} {
            $tsiAgentInfo($agent,addFN) $agent $sensorID($i,$j,$agent) ^west \
	                  $sensorID([expr $i-1],$j,$agent) wme_sharedID
            $tsiAgentInfo($agent,addFN) $agent \
	                  $sensorID([expr $i-1],$j,$agent) ^east \
                          $sensorID($i,$j,$agent) wme_sharedID
         }
      }
      ### Add left edge
      set i -$inputSensorRange($agent)
      ### Don't need to decrement by 1 because now the top and bottom exist
      for {set j -$inputSensorRange($agent)} \
          {$j <= $inputSensorRange($agent)} \
          {incr j} {
         set wme [$tsiAgentInfo($agent,addFN) $agent \
		      $sensorID([expr $i+1],$j,$agent) ^west * wme_id]
         #$agent alias id_west $wme
         if {($j > -$inputSensorRange($agent)) && \
             ($j < $inputSensorRange($agent))} {
	      set westWME([expr $i+1],$j,$agent) $wme
	      #set westWME([expr $i+1],$j,$agent) \
		 [$agent eval [list id_west GetTimeTag]]
         }
	      #$agent alias id_west {}

         set sensorID($i,$j,$agent) $wme
         ### Don't need to save these, because they should get garbage
         ### collected when we delete by layers.
         $tsiAgentInfo($agent,addFN) $agent $sensorID($i,$j,$agent) ^east \
                       $sensorID([expr $i+1],$j,$agent) wme_sharedID
         ### Link to previous neighbor on this layer
         if {$j > -$inputSensorRange($agent)} {
            $tsiAgentInfo($agent,addFN) $agent $sensorID($i,$j,$agent) ^north \
                          $sensorID($i,[expr $j-1],$agent) wme_sharedID
            $tsiAgentInfo($agent,addFN) $agent \
		          $sensorID($i,[expr $j-1],$agent) ^south \
		          $sensorID($i,$j,$agent) wme_sharedID
         }
      }
      ### Add right edge
      set i $inputSensorRange($agent)
      ### Don't need to decrement by 1 because now the top and bottom exist
      for {set j -$inputSensorRange($agent)} \
          {$j <= $inputSensorRange($agent)} \
          {incr j} {
         set wme [$tsiAgentInfo($agent,addFN) $agent \
		     $sensorID([expr $i-1],$j,$agent) ^east * wme_id]
         #$agent alias id_east $wme
         if {($j > -$inputSensorRange($agent)) && \
             ($j < $inputSensorRange($agent))} {
	     set eastWME([expr $i-1],$j,$agent) $wme
	     #set eastWME([expr $i-1],$j,$agent) \
		 [$agent eval [list id_east GetTimeTag]]
         }
	      #$agent alias id_east {}
         set sensorID($i,$j,$agent) $wme
         ### Don't need to save these, because they should get garbage
         ### collected when we delete by layers.
         $tsiAgentInfo($agent,addFN) $agent $sensorID($i,$j,$agent) ^west \
                       $sensorID([expr $i-1],$j,$agent) wme_sharedID
         ### Link to previous neighbor on this layer
         if {$j > -$inputSensorRange($agent)} {
            $tsiAgentInfo($agent,addFN) $agent $sensorID($i,$j,$agent) ^north \
                          $sensorID($i,[expr $j-1],$agent) wme_sharedID
            $tsiAgentInfo($agent,addFN) $agent \
	                  $sensorID($i,[expr $j-1],$agent) ^south \
	                  $sensorID($i,$j,$agent) wme_sharedID
         }
      }
   }
}

proc deleteSensorLayer {agent} {
   global inputSensorRange myLocationWME sensorID \
          northWME southWME westWME eastWME tsiAgentInfo

   incr inputSensorRange($agent) -1
   if {$inputSensorRange($agent) == -1} {
      $tsiAgentInfo($agent,rmFN) $agent $myLocationWME($agent)
      unset myLocationWME($agent)
      unset sensorID(0,0,$agent)
   } elseif {$inputSensorRange($agent) >= 0} {
      ### Delete top and bottom edges
      for {set i -$inputSensorRange($agent)} \
          {$i <= $inputSensorRange($agent)} \
          {incr i} {
         $tsiAgentInfo($agent,rmFN) $agent \
                $northWME($i,-$inputSensorRange($agent),$agent)
         $tsiAgentInfo($agent,rmFN) $agent \
                $southWME($i,$inputSensorRange($agent),$agent)
         unset northWME($i,-$inputSensorRange($agent),$agent)
         unset southWME($i,$inputSensorRange($agent),$agent)
         unset sensorID($i,[expr -$inputSensorRange($agent)-1],$agent)
         unset sensorID($i,[expr $inputSensorRange($agent)+1],$agent)
      }
      ### Delete left and right edges
      for {set j -$inputSensorRange($agent)} \
          {$j <= $inputSensorRange($agent)} \
          {incr j} {
         $tsiAgentInfo($agent,rmFN) $agent \
                $westWME(-$inputSensorRange($agent),$j,$agent)
         $tsiAgentInfo($agent,rmFN) $agent \
                $eastWME($inputSensorRange($agent),$j,$agent)
         unset westWME(-$inputSensorRange($agent),$j,$agent)
         unset eastWME($inputSensorRange($agent),$j,$agent)
         catch "unset sensorID([expr -$inputSensorRange($agent)-1],$j,$agent)"
         catch "unset sensorID([expr $inputSensorRange($agent)+1],$j,$agent)"
      }
      ### ...and garbage collection should do the rest!
   }
}

proc updateSensorWMEs {agent} {
   global sensorRange eaterX eaterY gridSize typeOfObj natureWME \
       otherEaterColorWME otherEaterScoreWME otherEaterScore sensorEater \
       sensorID eaterColor eaterScore boardX boardY tsiAgentInfo \
       eaterObjectName agentMoved

   for {set i -$sensorRange} {$i<= $sensorRange} {incr i} {
      for {set j -$sensorRange} {$j<= $sensorRange} {incr j} {
         ### Just in case something goes wrong...
         if ![info exists sensorID($i,$j,$agent)] {
            break
         }
         set x [expr $i + [convertToLogical $eaterX($agent)]]
         set y [expr $j + [convertToLogical $eaterY($agent)]]
         ### Everything beyond the boundaries is assumed to be a wall.
         if {($x < 0) || ($x > [expr $boardX+1]) || \
             ($y < 0) || ($y > [expr $boardY+1])} {
            set nature wall
         } else {
            set contents [.wGlobal.c find enclosed \
                              [expr $x*$gridSize-1] [expr $y*$gridSize-1] \
                              [expr ($x+1)*$gridSize] [expr ($y+1)*$gridSize]]
            ### We are going to assume that there can only be one of these
            ### things on a square at a time.
            set nature empty
            foreach c $contents {
               if {[lsearch [.wGlobal.c itemcget $c -tags] eater] >= 0} {
                  set nature eater
                  set eaterObj $c
		  set eaterName $eaterObjectName($eaterObj)
               }
               if {[lsearch [.wGlobal.c itemcget $c -tags] normalfood] >= 0} {
                  set nature normalfood
               }
               if {[lsearch [.wGlobal.c itemcget $c -tags] bonusfood] >= 0} {
                  set nature bonusfood
               }
               if {[lsearch [.wGlobal.c itemcget $c -tags] wall] >= 0} {
                  set nature wall
               }
            }
         }
         ### If the nature of the object has changed, remove the WME's
         ### describing the old object
         if {[info exists typeOfObj($i,$j,$agent)] && \
             ($typeOfObj($i,$j,$agent) != $nature)} {
	     $tsiAgentInfo($agent,rmFN) $agent $natureWME($i,$j,$agent)
	     unset natureWME($i,$j,$agent)
	     if {$typeOfObj($i,$j,$agent) == {eater}} {
		 $tsiAgentInfo($agent,rmFN) $agent \
		     $otherEaterColorWME($i,$j,$agent)
		 $tsiAgentInfo($agent,rmFN) $agent \
		     $otherEaterScoreWME($i,$j,$agent)
		 unset otherEaterColorWME($i,$j,$agent)
		 unset otherEaterScoreWME($i,$j,$agent)
		 unset otherEaterScore($i,$j,$agent)
		 unset sensorEater($i,$j,$agent)
	     }
	     unset typeOfObj($i,$j,$agent)
         } elseif { [info exists typeOfObj($i,$j,$agent)] && \
			[info exists agentMoved($agent)] && \
			$agentMoved($agent) } {
	     # If the agent moved, or turned, then we want new timetags
	     # on the contents of each square...that is, the eater should 
	     # be able to distinguish between the piece of food that was
	     # on its left and the new piece of food that is currently
	     # on its left.
	     
	     # this might not be what we want for the special case
	     # of eaters moving alongside each other...
	     $tsiAgentInfo($agent,rmFN) $agent $natureWME($i,$j,$agent)
	     unset natureWME($i,$j,$agent)
	     unset typeOfObj($i,$j,$agent)
	 }
	     
         ### If this is a new object, add a WME describing it
         if ![info exists typeOfObj($i,$j,$agent)] {
            set wme [$tsiAgentInfo($agent,addFN) $agent \
			$sensorID($i,$j,$agent) ^content $nature wme_string]
	    set natureWME($i,$j,$agent) $wme
            set typeOfObj($i,$j,$agent) $nature
	    #$agent alias nature_id $wme
	    #set natureWME($i,$j,$agent) [$agent eval [list nature_id GetTimeTag]]
	     #$agent alias nature_id {}
         }
         ### Remove old eater WMES, because these guys are dynamic --
	 ### that is, their score changes...
         if {($nature == {eater}) && \
		 [info exists sensorEater($i,$j,$agent)] } {

	     if { $sensorEater($i,$j,$agent) == $eaterObj } {
		 if {$eaterScore($eaterName) != $otherEaterScore($i,$j,$agent)} {
		     $tsiAgentInfo($agent,rmFN) $agent \
			 $otherEaterScoreWME($i,$j,$agent)
		     set wme [$tsiAgentInfo($agent,addFN) $agent \
				  $sensorID($i,$j,$agent) ^eater-score \
				  $eaterScore($eaterName) wme_int]
		     set otherEaterScoreWME($i,$j,$agent) $wme
                     #$agent alias score_id $wme
		     #set otherEaterScoreWME($i,$j,$agent) \
			 [$agent eval [list score_id GetTimeTag]]
		     set otherEaterScore($i,$j,$agent) $eaterScore($eaterName)
		 } 
	     } else {
		 $tsiAgentInfo($agent,rmFN) $agent \
		     $otherEaterColorWME($i,$j,$agent)
		 $tsiAgentInfo($agent,rmFN) $agent \
		     $otherEaterScoreWME($i,$j,$agent)
		 unset sensorEater($i,$j,$agent)
		 unset otherEaterColorWME($i,$j,$agent)
		 unset otherEaterScoreWME($i,$j,$agent)
		 unset otherEaterScore($i,$j,$agent)
	     }
         }
         ### If the content is a new eater, add special eater WME's
         if {($nature == {eater}) && \
             ![info exists sensorEater($i,$j,$agent)]} {
            set wme [$tsiAgentInfo($agent,addFN) $agent \
			$sensorID($i,$j,$agent) ^eater-color \
			$eaterColor($eaterName) wme_string]
	    set otherEaterColorWME($i,$j,$agent) $wme

            #$agent alias id_eater $wme
	    #set otherEaterColorWME($i,$j,$agent) \
		[$agent eval [list id_eater GetTimeTag]]

            set wme [$tsiAgentInfo($agent,addFN) $agent \
			$sensorID($i,$j,$agent) ^eater-score \
			$eaterScore($eaterName) wme_int]
            set otherEaterScoreWME($i,$j,$agent) $wme

            #$agent alias id_escore $wme
            #set otherEaterScoreWME($i,$j,$agent) \
		[$agent eval [list id_escore GetTimeTag]]

	    set otherEaterScore($i,$j,$agent) $eaterScore($eaterName)
            set sensorEater($i,$j,$agent) $eaterObj
	     #$agent alias id_eater {}
	     #$agent alias id_escore {}
         }
      }
   }

   updateSensorDisplay $agent
}
