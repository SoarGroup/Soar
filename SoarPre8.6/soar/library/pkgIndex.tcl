# pkgIndex.tcl
#
# This file is sourced either when an application starts up or
# by a "package unknown" script.  It invokes the
# "package ifneeded" command to set up package-related
# information so that packages will be loaded automatically
# in response to "package require" commands.  When this
# script is sourced, the variable $dir must contain the
# full path name of this file's directory.

puts "dir = $dir"
package ifneeded Soar 8.5.2 [list
	# First, check to see if Soar is already loaded into the
	# current interpreter. If so, then don't load it again.
	foreach pkg [info loaded {}] {
		set name [lindex $pkg 1]
		
		if {"$name" == "Soar"} { return }
			# The Soar package is already loaded here.
	}
	
	# Next, check to see if Soar is loaded in *any* interpreter.
	# If so, load it from the same binary. Note that this will
	# work if Soar is statically linked, too.
	foreach pkg [info loaded] {
		set file [lindex $pkg 0]
		set name [lindex $pkg 1]
		
		if {"$name" == "Soar"} {
			load $file $name
			return
		}
	}
	
	# Otherwise, issue the platform-specific "load" command
	set lib "libsoar8.5.2[info sharedlibext]"
	load [file join $dir $lib]
]

