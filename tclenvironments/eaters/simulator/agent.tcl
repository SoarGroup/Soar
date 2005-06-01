global interp_name 
puts "Loading interface code for $interp_name"

### default setup

soar_agent ExecuteCommandLine "learn --off" 


##indifferent-selection -random

if {0} {
 ###KJC delete this stuff.  Need to support SML.

foreach i [io -list -input] {
   io -delete -input $i
}

## register the input function
io -add -input manageInputLink

foreach i [io -list -output] {
   io -delete -output $i
}

## register the output function
io -add -output manageOutputLink output-link

}

set ioID "dummy"
set inID [soar_agent GetInputLink]
set outID [soar_agent GetOutputLink]

setUpInputLink  $ioID $inID $outID

proc BeforeDecisionCycleCallback {id userData agent message} {
    updateInputLink
}
proc AfterDecisionCycleCallback {id userData agent message} {
    updateOutputLink
}

proc PrintCallback {id userData agent message} {
    puts "agent printCallback:$message"
    tsiOutputToWigdet $message
}

## rest of file used for Soar 8.5.2 and earlier
## SML uses updateInputLink and updateOutputLink def'd in interface.tcl

if {0} {
puts "defining manageInputLink"

proc manageInputLink {mode} {
   global ioID interp_name

#    puts "manageInputLink $mode		(begin)"
   switch $mode {


      top-state-just-created {
	  output-strings-destination -push -append-to-result
	  set topStateID [lindex [wmes {(* ^superstate nil)}] 1]
	  set ioID [lindex [wmes "($topStateID ^io *)"] 3]
	  ## remove the trailing parenthesis
	  set ioID [string trimright $ioID ")"]
          set inID  [lindex [wmes "($ioID ^input-link *)"] 3]
          set inID [string trimright $inID ")"]

          set outID [lindex [wmes "($ioID ^output-link *)"] 3]
          set outID [string trimright $outID ")"]
	  output-strings-destination -pop
	  setUpInputLink $ioID $inID $outID

      }
      normal-input-cycle {


	 updateInputLink
	 

      }
      top-state-just-removed {

         removeInputLink


      }
   }
#    puts "manageInputLink $mode		(end)"
}


proc manageOutputLink {mode outputs} {
   global ioID outputLinkID 
    
#    puts "manageOutputLink $mode     (begin)"
  
   switch $mode {
      added-output-command {
         set outputLinkID [getOutputValue $outputs $ioID "output-link"]
	 set walkID [getOutputValue $outputs $outputLinkID "move"]
         set jumpID [getOutputValue $outputs $outputLinkID "jump"]
         
         ## SB May 02 1997
         ## walkdir is actually used to walk the eater one step; movement is
         ## for one step and then stops.
          if {$walkID != ""} {
	     set walkdir [getOutputValue $outputs $walkID "direction"]
	      set isComplete [getOutputValue $outputs $walkID "status"]
 
	     if {$walkdir == "" || $isComplete == "complete" } {
		 
#		 puts "CANNOT FIND WALK DIRECTION OR ALREADY APPLIED 1!!!"
	     } else {
		 setWalkDir $walkdir $walkID
#		 puts "WALK DIRECTION is $walkdir"
	     }
            # send $controlName "set eaterDir($interp_name) $walkdir"
         } else {
           if {$jumpID != ""} {
	       set jumpdir [getOutputValue $outputs $jumpID "direction"]
	       set isComplete [getOutputValue $outputs $jumpID "status"]

	       if { $jumpdir == "" || $isComplete == "complete" } {
#		   puts "CANNOT FIND JUMP DIRECTION OR ALREADY APPLIED 1!!!"
	       } else {
		   setJumpDir $jumpdir $jumpID
	       }
	      # send $controlName "set eaterDir($interp_name) $jumpdir"
            } 
   
         } 
      } 
      modified-output-command {
         set walkID [getOutputValue $outputs $outputLinkID "move"]
         set jumpID [getOutputValue $outputs $outputLinkID "jump"]
         ## SB May 02 1997
         ## walkdir is actually used to move the eater; movement is
         ## for one step and then stops.
         if {$walkID != ""} {
	     set walkdir [getOutputValue $outputs $walkID "direction"]
	     set isComplete [getOutputValue $outputs $walkID "status"]

	     if {$walkdir == "" || $isComplete == "complete"} {
#		 puts "CANNOT FIND WALK DIRECTION OR ALREADY APPLIED 2!!!"
#		 puts "WALK DIRECTION Is $walkdir"
	     } else {
		 setWalkDir $walkdir $walkID
#		 puts "WALK DIRECTION is $walkdir"
	     }
            # send $controlName "set eaterDir($interp_name) $walkdir"
         } else {
#	     puts "NO WALK DIR--------- Checking for JUMP"
           if {$jumpID != ""} {
	       set jumpdir [getOutputValue $outputs $jumpID "direction"]
	       set isComplete [getOutputValue $outputs $jumpID "status"]

	       if { $jumpdir == "" || $isComplete == "complete" } {
#		   puts "CANNOT FIND JUMP DIRECTION OR ALREADY APPLIED 2!!!"
	       } else {
		   setJumpDir $jumpdir $jumpID
	       }
	      # send $controlName "set eaterDir($interp_name) $jumpdir"
            } 
	 }
      }
   }



#    puts "manageOutputLink $mode     (end)"
}

proc getOutputValue {outputs id attr} {
   foreach wme $outputs {
      if {(($id == "") || [string match $id [lindex $wme 0]]) && \
          (($attr == "") || [string match $attr [lindex $wme 1]])} {
         return [lindex $wme 2]
      }
   }
   return ""
}
}

