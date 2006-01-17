#  This file contains an example of defining Soar I/O functions entirely
#  using Tcl.  This example computes the Fibonnaci sequence. Karl Schwamb
#  (schwamb@isi.edu) wrote the original version and 
#  Robert Wray (robert.wray@umich.edu) modified the example to include
#  Soar operators and deliberate output-link clean up.
#
### This example is consistent with the way I (robert.wray@umich.edu) think 
### Soar-6-syle IO was generally used.  It is based on the following 
### observations:
###
###
### In Soar 6:
###    1) The top state (identifiable by the ^superstate nil) and the
###       ^io augmentation on the top state are created by the architecture
###       at decision cycle 0.
###    2) The ^input-link augmentation on ^io is created by the IO subsystem.
###       In Soar 6, this was normally a C function, in Soar 7, it will 
###       (probably) most often be done with tcl scripts.
###    3) The ^output-link augmentation on ^io is created by production firings
###       from the Soar agent.
###    4) Once created, the input-link and output-link identifiers are not deleted
###       or changed unless there is an init-soar issued.  
###
###
###  The total program (Soar + IO routines) computes the Fibonnaci sequence.  This
###  occurs through the action of the add-two-inputs Soar operator which simply
###  takes  the values of two numbers on its input-link, adds them, and directs
###  the output routine to print the added number by issuing an output command.
###  Also notice that the removal of output commands is a deliberate process in
###  this example (ie, done by an operator).  That is the way this is done in most
###  Soar implementations.
###
###  The second of the Fibonnaci computations occurs on the IO process side.  The
###  print-add-result command not only prints the result but also updates the
###  variables fib_n and fib_n_1.  These two variables are then updated on the Soar
###  input-link by the input-routine, resulting in a new proposal of 
###  add-two-numbers.  So the result
###  is the addition of a new number to the Fibonnaci sequence every decision.

###  To run:
###  source this file in a Soar 7 window         ==> source soar-io-using-tcl.tcl
###  type step by decisions to see the behavior  ==> go 1 d 
###  Note: Since the Fibonnaci values increase quickly, you will see errors
###  as soon as 40 decisions, depending on the size of integers on your machine.
###  (Fib(40) = 165580141)

package require Soar

proc my-input-procedure {mode} {
    global io_header input_header counter past_count number_1_wme \
           number_2_wme number_1_id number_2_id fib_n fib_n_1 
    # Show inputs: prints out this message each time the input procedure is called
    echo "my-input-procedure: <$mode>"

    switch $mode {
	top-state-just-created {
	    
	    set counter 2             ;# Initialize counter
            set past_count $counter   ;# Past Count is used to decide when to update input
            set fib_n   1             ;# Bootstrap the Fibonnaci sequence for this example
            set fib_n_1 1
	    # Find the identifier for the root of I/O activity.  On the 
            # top-state this identifier is the value of the "io" attribute.
            # We use the command "output-strings-destination" to return the
            # results of the wmes command rather than printing the information
            # to the screen.

	    # First, find the top state id:

	    output-strings-destination -push -append-to-result
	    scan [print -internal {(* ^superstate nil)}] "(%d: %s" timetag top_state_id
	    set top_wmes [print -internal $top_state_id]
	    output-strings-destination -pop

	    # Now find the io header id within the top state wmes.  We
            # cannot use "wmes" here because although the addition of the 
            # "io" link has just been made in the agent, the WME additions 
            # are buffered until all the input functions have been processed.

	    set io_header_part [lindex $top_wmes \
		                       [expr [lsearch $top_wmes "^io"] \
                                             + 1]]
	    set io_header [string trimright $io_header_part ")"]

	    output-strings-destination -push -append-to-result
	    set io_wmes [print -internal $io_header]
  	    output-strings-destination -pop
	    set input_header [lindex $io_wmes \
		                       [expr [lsearch $io_wmes "^input-link"] \
                                             + 1]]
	    set link_id [string trimright $input_header ")"]


	    # Now add the three wmes defining the input structure.  We save the
	    # timetag of the last number wmes so we can update during normal input 
            # cycles.

	    #set link_id  [add-wme-and-get-id $io_header input-link]
	    set number_1_id [add-wme-and-get-id $link_id number-1]
	    set number_2_id [add-wme-and-get-id $link_id number-2]
	    scan [add-wme $number_1_id value $fib_n]   "%d" number_1_wme
	    scan [add-wme $number_2_id value $fib_n_1] "%d" number_2_wme

	}
	normal-input-cycle     {
 
	    # For a normal input, check to see if the counter (which is
            # incremented for valid output) is different from the
            # past count.  If so, then remove the wme's for the
            # previous values of number-1 and number-2 and then
            # place the new values (fib_n and fib_n_1) on the input-link.

            if { $past_count != $counter } { 
	       remove-wme $number_1_wme
	       remove-wme $number_2_wme
	       scan [add-wme $number_1_id value $fib_n]   "%d" number_1_wme
	       scan [add-wme $number_2_id value $fib_n_1] "%d" number_2_wme
               set past_count $counter
            }
	}
	top-state-just-removed {
	    # The lack of non-commented code here implies that no special processing 
	    # is necessary when the top state is just removed (ie, an init-soar has just
            # been issued).  This may not always be the case. 
        }
    }
}

# The following procedure adds a wme with a specified object and
# attribute.  The value is an identifier which is returned after 
# being generated automatically by Soar.

proc add-wme-and-get-id {obj attr} {
    scan [add-wme $obj $attr *] "%d: %s %s %s" \
	    timetag object attribute id
    return $id
}

# The following output procedure examines the example output link
# structure and prints the added result data, if present.  Note
# that io_header is computed in my_input_procedure before this
# procedure is called.

proc my-output-procedure {mode outputs} {
    global io_header output_link_id counter fib_n fib_n_1 

# output_link_id is global because one need only compute it once, when
# the output-link augmentation is created.  In general, this should only
# happen once between init-soar's (mode: added-output-command).
# Then, the id is used in the changed-output-command calls to do the
# processing directed by the Soar agent.

    # Show inputs

    echo "my-output-procedure: <$mode>"
    foreach out $outputs {
    	echo "==> $out"
    }

    switch $mode {
	added-output-command    {


	    output-strings-destination -push -append-to-result
	    set io_wmes [print -internal $io_header]
  	    output-strings-destination -pop
	    set output_header [lindex $io_wmes \
		                       [expr [lsearch $io_wmes "^output-link"] \
                                             + 1]]
	    set output_link_id [string trimright $output_header ")"]

	    echo "output-link id = $output_link_id"

	    #set output_link_id \
            #       [get-output-value $outputs $io_header "^output-link"]

	    if [string match $output_link_id ""] {
			echo "wrong output_link_id: $output_link_id"
			return}
	    set command_name \
                    [get-output-value $outputs $output_link_id command]
            echo "called:  get-output-value $outputs $output_link_id command"
            echo "command_name = $command_name"
	    if [string match $command_name ""] {return}
	    if [string match $command_name print-add-result] {
		set value \
			[get-output-value $outputs $output_link_id \
                                          add-result]
		if [string match $value ""] {return}
		echo "IO: New Value Fib($counter) = $value = $fib_n + $fib_n_1"
                incr counter 
                set fib_n_1 $fib_n
                set fib_n $value
            }

        }
	modified-output-command {
	    if [string match $output_link_id ""] {return}
	    set command_name \
                    [get-output-value $outputs $output_link_id command]
	    if [string match $command_name ""] {return}
	    if [string match $command_name print-add-result] {
		set value \
			[get-output-value $outputs $output_link_id \
                                          add-result]
		if [string match $value ""] {return}
		echo "IO: New Value Fib($counter) = $value = $fib_n + $fib_n_1"
                incr counter 
                set fib_n_1 $fib_n
                set fib_n $value
	    } else {

# In this case, there is only a single command.  In other systems, there
# will be a number of elseif's here (or perhaps a switch on $command_name)
# to parse the command from the Soar agent.

		echo "I don't know the command: $command_name"
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
if {[info exists added_monitors]==0} {
    io -add -input  my-input-procedure  input-test
    io -add -output my-output-procedure output-link
    set added_monitors 1
}



######################### Soar Productions #######################################

# this production creates the output-link augmentation under ^io

#sp { top-state*elaborate*output-link
#   (state <s> ^io <io>)
#   -->
#   (write (crlf) |Soar: Creating the output-link augmentation| (crlf) )
#   (<io> ^output-link <ol>)}


# if there are two numbers on the input-link, then propose this operator to add them.

sp { propose*operator*add-two-numbers
   (state <s> ^io <io>)
   (<io> ^input-link <il>)
   (<il> ^number-1 <n1> ^number-2 <n2>)
   (<n1> ^value <v1>)
   (<n2> ^value <v2>)
   -->
   (<s> ^operator <o> +)
   (<o> ^name add-two-numbers
        ^number-1 <v1> 
        ^number-2 <v2>)}

# Once the operator has been selected, this application production does the addition
# and sends the result as a command to the output function through the output-link.

sp { add-two-numbers*apply*add-and-output
        (state <s> ^operator <o> ^io <io> )
        (<io> ^output-link <ol>)
        (<o> ^name add-two-numbers
             ^number-1 <n1> 
             ^number-2 <n2>)
        -(<ol> ^command print-add-result)
        --> 
	(write (crlf) |Soar: | <n1>| + | <n2> | = | (+ <n1> <n2>) (crlf))
        (<ol> ^command print-add-result
              ^add-result (+ <n1> <n2>))
   }                                           

# The test of the output-link guarantees that the
# there has been an output phase between the application/elaboration
# that placed the output command on the link and the firing of
# this one.  
sp { add-two-numbers*apply*remove-output-command
        (state <s> ^operator <o> ^io <io> )
        (<o> ^name add-two-numbers 
             ^number-1 <n1>) 
        (<io> ^output-link <ol>)
        (<ol> ^command <command> ^add-result <n1>)
        --> 
    (write (crlf) |Soar: Removing TCL command | <command> | add-result  | <n1> (crlf) )
    (<ol> ^command <command> -
         ^add-result <n1> -)}

