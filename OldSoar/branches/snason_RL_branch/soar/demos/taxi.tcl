# hierarchical test for RL-Soar ; taken from dietterich ; also source taxi.soar

package require Soar

learn -off

proc random { range } {
    return [expr {int(rand()*$range)}]
}


proc go_north {} {
  global vertical changes
  if { $vertical != 4 } { incr vertical ; set changes(vertical) 1}
  }

proc go_south {} {
  global vertical changes
  if { $vertical != 0 } { set vertical [expr $vertical - 1] ; set changes(vertical) 1}
  }

proc go_east {} {
  global vertical horizontal changes
  switch $horizontal {
     0 { if { $vertical > 1 } { incr horizontal ; set changes(horizontal) 1} }
     1 { if { $vertical < 3 } { incr horizontal ; set changes(horizontal) 1} }
     2 { if { $vertical > 1 } { incr horizontal ; set changes(horizontal) 1} }
     3 { incr horizontal ; set changes(horizontal) 1}
     4 {}
     }
}

proc go_west {} {
  global vertical horizontal changes
  switch $horizontal {
     1 { if { $vertical > 1 } { incr horizontal -1 ; set changes(horizontal) 1} }
     2 { if { $vertical < 3 } { incr horizontal -1 ; set changes(horizontal) 1} }
     3 { if { $vertical > 1 } { incr horizontal -1 ; set changes(horizontal) 1} }
     4 { incr horizontal -1 ; set changes(horizontal) 1}
     0 {}
     }
}

proc do_putdown {} {
   global reward pass_loc dest_loc horizontal vertical changes
   
   set reward -1
   if { $pass_loc == 4 } {
      if { $dest_loc == 0 && $horizontal == 0 && $vertical == 0  ||  
           $dest_loc == 1 && $horizontal == 0 && $vertical == 4  || 
           $dest_loc == 2 && $horizontal == 3 && $vertical == 0  || 
           $dest_loc == 3 && $horizontal == 4 && $vertical == 4 }  { 
           set reward [expr $reward + 20]
           set pass_loc $dest_loc
           set changes(pass) 1
        } else { set reward [expr $reward - 10] }
   }
}

proc do_pickup {} {
   global reward pass_loc horizontal vertical changes		
	set reward -1
	
	if { $pass_loc != 4 } {
	 if { $pass_loc == 3 && $horizontal == 4 && $vertical == 4 || 
	      $pass_loc == 0 && $horizontal == 0 && $vertical == 0 || 
	      $pass_loc == 1 && $horizontal == 0 && $vertical == 4 || 
    	      $pass_loc == 2 && $horizontal == 3 && $vertical == 0 } { 
          set pass_loc 4
          set changes(pass) 1
         } else {  
         set reward [expr $reward - 10]
         }
      }
    }
           



proc compute-stand { h v } {
   
	if { $h == 0 && $v == 0 } {
		return 0
	} elseif { $h == 0 && $v == 4 } {
	        return 1
	} elseif { $h == 3 && $v == 0 } {
	        return 2
	} elseif { $h == 4 && $v == 4 } {
	        return 3
	} else { return 4 }
}









proc my-input-procedure {mode} {
    global io_header input_header reward_header hor_wme horizontal ver_wme vertical \
              pass_wme pass_loc dest_wme dest_loc reward_wme reward stand stand_wme changes
     
           

    # Show inputs: prints out this message each time the input procedure is called
   # puts "my-input-procedure: <$mode>"

    switch $mode {
	top-state-just-created {

	  
	    # Find the identifier for the root of I/O activity.  On the 
            # top-state this identifier is the value of the "io" attribute.
            # We use the command "output-strings-destination" to return the
            # results of the wmes command rather than printing the information
            # to the screen.

	    # First, find the top state id:

	    output-strings-destination -push -append-to-result
	    scan [wmes {(* ^io *)}] "(%d: %s %s %s" \
	           timetag object wme temp
	    set io_header [string trimright $temp ")"]       
	    scan [wmes {(* ^input-link *)}] "(%d: %s %s %s" \
	           timetag object wme temp
	    set input_header [string trimright $temp ")"]       
	    scan [wmes {(* ^reward *)}] "(%d: %s %s %s" \
	           timetag object wme temp
	    set reward_header [string trimright $temp ")"]       
	    output-strings-destination -pop
	    
	    set stand [compute-stand $horizontal $vertical]
	    
 	    # Now add the three wmes defining the input structure.  We save the
	    # timetag of the last number wmes so we can update during normal input 
            # cycles.

	    set hor_wme [add-wme-and-get-timetag $input_header horizontal $horizontal]
	    set ver_wme [add-wme-and-get-timetag $input_header vertical $vertical]
	    set pass_wme [add-wme-and-get-timetag $input_header pass_loc $pass_loc]
	    set dest_wme [add-wme-and-get-timetag $input_header dest_loc $dest_loc]
	    set reward_wme [add-wme-and-get-timetag $reward_header value $reward]
	    set stand_wme [add-wme-and-get-timetag $input_header stand $stand]
	    }
	normal-input-cycle     {

	    # For a normal input, check to see if the counter (which is
            # incremented for valid output) is different from the
            # past count.  If so, then remove the wme's for the
            # previous values of number-1 and number-2 and then
            # place the new values (fib_n and fib_n_1) on the input-link.

	       set temp [compute-stand $horizontal $vertical]
	       if { $temp != $stand } {
	            set changes(stand) 1
	            set stand $temp
	            }

               if {[info exists changes(horizontal)] && $changes(horizontal)} {
       	  		unset changes(horizontal)
               		remove-wme $hor_wme
               		set hor_wme [add-wme-and-get-timetag $input_header horizontal $horizontal]
               		}
                if {[info exists changes(vertical)] && $changes(vertical)} {
       	  	       unset changes(vertical)
               	       remove-wme $ver_wme
               	       set ver_wme [add-wme-and-get-timetag $input_header vertical $vertical]
	       	       }
	         if {[info exists changes(pass)] && $changes(pass)} {
       	  		unset changes(pass)
	       	       remove-wme $pass_wme
	       	       set pass_wme [add-wme-and-get-timetag $input_header pass_loc $pass_loc]
	       	       }
	       	       
	       remove-wme $reward_wme
	       set reward_wme [add-wme-and-get-timetag $reward_header value $reward]
	    	       	       
	        if {[info exists changes(stand)] && $changes(stand)} {
       	  		unset changes(stand)
	               remove-wme $stand_wme
	               set stand_wme [add-wme-and-get-timetag $input_header stand $stand]
	               }
	       
	      
	       
	       	      
               
            }
            
	top-state-just-removed {
	    # The lack of non-commented code here implies that no special processing 
	    # is necessary when the top state is just removed (ie, an init-soar has just
            # been issued).  This may not always be the case. 
            
             set horizontal [random 4] 
	     set vertical [random 4]
	     set pass_loc [random 4]
	     #set horizontal 0
	     #set vertical 3
	     #set pass_loc 1
	     set dest_loc [random 4]
  	     set reward 0
        }
    }
}

 
# The following procedure adds a wme with a specified object and
# attribute.  The value is an identifier which is returned after 
# being generated automatically by Soar.

proc add-wme-and-get-timetag {obj attr value} {
    scan [add-wme $obj $attr $value] "%d: %s %s %s" \
	    timetag object attribute id
    return $timetag
}

proc get-commands {outputs ids} {
	foreach id $ids {
		foreach wme $outputs {
			if { [string match $id [lindex $wme 0]] } {lappend commands $wme}
					}
			}
	return $commands
}

proc get-output-values {outputs id attr} {
	set outlist [list]
	foreach wme $outputs {
		if { [string match $id [lindex $wme 0]] && [string match $attr [lindex $wme 1]] } {
			lappend outlist [lindex $wme 2]
		}
	}
	return $outlist
}


# The following output procedure examines the example output link
# structure and prints the added result data, if present.  Note
# that io_header is computed in my_input_procedure before this
# procedure is called.

proc my-output-procedure {mode outputs} {
    global io_header olink_id reward 
    
# output_link_id is global because one need only compute it once, when
# the output-link augmentation is created.  In general, this should only
# happen once between init-soar's (mode: added-output-command).
# Then, the id is used in the changed-output-command calls to do the
# processing directed by the Soar agent.

    # Show inputs

    puts "my-output-procedure: <$mode> <$outputs>"
    foreach out $outputs {
	puts "==> $out"
    }

    switch $mode {
	added-output-command    {

	    set olink_id [get-output-value $outputs $io_header "output-link"]
	     set command_ids [get-output-values $outputs $olink_id "command"]
	       if {[llength $command_ids] == 0} {return}
	       set commands [get-commands $outputs $command_ids]
	       foreach command $commands {
	       		set attr [lindex $command 1]
	       		set value [lindex $command 2]
	       		switch $attr {
	    	   		move {
	    	   		  switch $value {
	    	   		        north { go_north ; set reward -1  }
	    		       		south { go_south ; set reward -1 }
	    		       		east { go_east ; set reward -1 }
	           			west { go_west ; set reward -1 }
	    	   		      	default { echo "I don't know the command: $command_name" }
	    	   		      }
	       		     }
	    	   		do {
	    	   		  switch $value {
	    	   		         pickup { do_pickup }
	    	       			 putdown { do_putdown }
	    	       			 default { echo "I don't know the command: $command_name"}
	           		   }
	           		}
	           		default { echo "I don't know the command: $command_name" }
	                    }
	       		add-wme [lindex $command 0] status complete 			
	    }  
        }
	modified-output-command {
	 set command_ids [get-output-values $outputs $olink_id "command"]
	    if {[llength $command_ids] == 0} {return}
	    set commands [get-commands $outputs $command_ids]
	    foreach command $commands {
	    		set attr [lindex $command 1]
	    		set value [lindex $command 2]
	    		switch $attr {
	    			move {
	    			    switch $value {
	    			        north { go_north ; set reward -1 }
				       	south { go_south ; set reward -1 }
				       	east { go_east ; set reward -1 }
	        			west { go_west ; set reward -1 }
	    			      	default { echo "I don't know the command: $command_name" }
	    			      }
	    			     }
	    			do {
	    			    switch $value {
	    			         pickup { do_pickup }
	        			 putdown { do_putdown }
	        			 default { echo "I don't know the command: $command_name"}
	        			 }
	        		  }
	        		default { echo "I don't know the command: $command_name" }
                             }
	    		add-wme [lindex $command 0] status complete 			
	     	}    
	    }
	removed-output-command  {
# The same comment applies here as it did in the input routine.
        }
    }
}

# The utility get-output-value is defined as follows:

# Given an id or an attribute, return the value of that WME. 
proc get-output-value {outputs id attr} {
    foreach wme $outputs {
#        the 0'th element in a wme list is the id
	if {   [string match $id ""] 
	|| [string match $id [lindex $wme 0]]} {
#        and the second (1'th) element is the attribute
	    if {   [string match $attr ""] 
	    || [string match $attr [lindex $wme 1]]} {
#        and the third (2'th) element is the value
		return [lindex $wme 2]
	    }
	}
    }
}

# Now tell the agent about which I/O procedures to use


# You can't add the same input-function twice without first deleting it.  so, to
# avoid this problem (when re-source-ing, for example) the io functions are added
# only when the added_montors variable has not yet been created.
#if {[info exists added_monitors]==0} {
#    io -add -input  my-input-procedure  input-test
#    io -add -output my-output-procedure output-link
#    set added_monitors 1
#}


scan [io -list -input] "%s" old_input
scan [io -list -output] "%s" old_output
if {[info exists old_input]==1} {
	io -delete -input $old_input
}
if {[info exists old_output]==1} {
      io -delete -output $old_output
 }
io -add -input my-input-procedure input-test
io -add -output my-output-procedure output-link

set horizontal [random 5] 
set vertical [random 5]
set pass_loc [random 4]
#set horizontal 0
#set vertical 3
#set pass_loc 1
  set dest_loc [random 4]
  set reward 0
 