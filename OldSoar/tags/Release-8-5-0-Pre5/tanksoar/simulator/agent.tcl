global interp_name
#puts "Loading interface code for $interp_name"

source utilities.tcl
DebugOn

### default setup
learn -off
indifferent-selection -random

 
foreach i [io -list -input] {
   io -delete -input $i
}
io -add -input manageInputLink

foreach i [io -list -output] {
   io -delete -output $i
}
io -add -output manageOutputLink output-link

proc manageInputLink {mode} {
   global ioID

	#Debug "manageInputLink called with mode $mode"

   switch $mode {
      top-state-just-created {
          output-strings-destination -push -append-to-result
          set topStateID [lindex [wmes {(* ^superstate nil)}] 1]

          set ioID [lindex [wmes "($topStateID ^io *)"] 3]
          set ioID [string trimright $ioID ")"]

          set inID  [lindex [wmes "($ioID ^input-link *)"] 3]
          set inID [string trimright $inID ")"]

          set outID [lindex [wmes "($ioID ^output-link *)"] 3]
          set outID [string trimright $outID ")"]

          output-strings-destination -pop
         #Debug "IDs: $ioID $inID $outID"
          setUpInputLink $ioID $inID $outID
      }
      normal-input-cycle {
         updateInputLink
      }
      top-state-just-removed {
         removeInputLink
      }
   }
}

proc resetup {} {
   run-soar 1 p -self
          output-strings-destination -push -append-to-result
          set topStateID [lindex [wmes {(* ^superstate nil)}] 1]

          set ioID [lindex [wmes "($topStateID ^io *)"] 3]
          set ioID [string trimright $ioID ")"]

          set inID  [lindex [wmes "($ioID ^input-link *)"] 3]
          set inID [string trimright $inID ")"]

          set outID [lindex [wmes "($ioID ^output-link *)"] 3]
          set outID [string trimright $outID ")"]

          output-strings-destination -pop
          setUpInputLink $ioID $inID $outID
}   
proc manageOutputLink {mode outputs} {
   global ioID outputLinkID sensorList sensorInfo outputList
	
	#Debug "manageOutputLink called with:"
	#Debug "  mode $mode"
	#Debug "  outputs $outputs"
	set outputList $outputs	
	switch $mode {
		added-output-command {
			set outputLinkID [getOutputValue $ioID "output-link"]
			#Debug "- ioID is $ioID and outputLinkID is $outputLinkID"
						
			foreach currSensor $sensorList {
				#puts "Processing $currSensor"
				set sensorDone 0
				while {!$sensorDone} {
					set outInfo($currSensor,ID) [getOutputValue $outputLinkID $currSensor]
					#puts "ID for $currSensor is $outInfo($currSensor,ID)"
					if {$outInfo($currSensor,ID) != ""} {
						set outInfo($currSensor,value)	\
							[getOutputValue $outInfo($currSensor,ID) $sensorInfo($currSensor,parameterName)]
						#puts "Value for $currSensor is $outInfo($currSensor,value)"
						set isComplete [getOutputValue $outInfo($currSensor,ID) status]
						if {$outInfo($currSensor,value) == "" || $isComplete == "complete"} {
							#puts "Cannot find command for $currSensor or it is already applied.  Ignoring."
						} else {
							setActionRequest $sensorInfo($currSensor,actionName) $outInfo($currSensor,value) $outInfo($currSensor,ID)
							#puts "Setting actionrequest: $sensorInfo($currSensor,actionName) $outInfo($currSensor,value) $outInfo($currSensor,ID)"
						}
					} else {
						set sensorDone 1
					}
				}
			}
		}

		modified-output-command {
			#Debug "- ioID is $ioID and outputLinkID is $outputLinkID"
						
			foreach currSensor $sensorList {
				#puts "Processing $currSensor"
				set sensorDone 0
				while {!$sensorDone} {
					set outInfo($currSensor,ID) [getOutputValue $outputLinkID $currSensor]
					#puts "ID for $currSensor is $outInfo($currSensor,ID)"
					if {$outInfo($currSensor,ID) != ""} {
						set outInfo($currSensor,value)	\
							[getOutputValue $outInfo($currSensor,ID) $sensorInfo($currSensor,parameterName)]
						#puts "Value for $currSensor is $outInfo($currSensor,value)"
						set isComplete [getOutputValue $outInfo($currSensor,ID) status]
						if {$outInfo($currSensor,value) == "" || $isComplete == "complete"} {
							#puts "Cannot find command for $currSensor or it is already applied.  Ignoring."
						} else {
							setActionRequest $sensorInfo($currSensor,actionName) $outInfo($currSensor,value) $outInfo($currSensor,ID)
							#puts "Setting actionrequest: $sensorInfo($currSensor,actionName) $outInfo($currSensor,value) $outInfo($currSensor,ID)"
						}
					} else {
						set sensorDone 1
					}
				}
			}
		}
	}
}

proc getOutputValue {id attr} {
global outputList
	#>-=>
	# This function parses the items on the output link to determine
	# whether they match for a given command
	#>-=>
	
	foreach wme $outputList {
		if {(($id == "") || [string match $id [lindex $wme 0]]) && \
			 (($attr == "") || [string match $attr [lindex $wme 1]])} {
			 #puts "A valid WME was found in getOutputValue $wme"
			set outputList [ldelete $outputList $wme]
			return [lindex $wme 2]
		}
	}
	return ""
	
}

proc toggleAgentWindows {} {

	set windowList [winfo children .]
	foreach w $windowList {
		if {[winfo toplevel "$w"] == "$w"} {
			if {[winfo ismapped $w]} {
				wm iconify $w
			} else {
				wm deiconify $w
			}
		}
	}
}
