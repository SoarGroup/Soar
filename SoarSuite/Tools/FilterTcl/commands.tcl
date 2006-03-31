# This allows us to have Soar commands that are intercepted
# and handled as we wish.

# these lines don't seem to actually be required
#lappend auto_path .
#package require tcl_sml_clientinterface

#this is from tsiSoarCmds
proc print {args} {return [soar_agent ExecuteCommandLine "print $args" 0 1]}

#this is Doug's version (modified by Bob)
#proc print {arg} {
#	global _agentName
#	puts "Called internal print"
#	puts $arg
#	# This line fails because $_kernel is in the global namespace of the parent
#	# interpreter and we're executing this in the child.
#	soar_kernel ExecuteCommandLine "print $arg" $_agentName
#}

# We can use this technique to intercept "puts" and send the output somewhere else
# of our choosing.
rename puts myPuts
proc puts {arg} {
	myPuts "Called special puts"
	myPuts $arg
}
