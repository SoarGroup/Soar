# This allows us to have Soar commands that are intercepted
# and handled as we wish.
proc print {arg} {
	puts "Called internal print"
	puts $arg
	# This line fails because $_kernel is in the global namespace of the parent
	# interpreter and we're executing this in the child.
	#$_kernel ExecuteCommandLine print $arg
}

# We can use this technique to intercept "puts" and send the output somewhere else
# of our choosing.
rename puts myPuts
proc puts {arg} {
	myPuts "Called special puts"
	myPuts $arg
}
