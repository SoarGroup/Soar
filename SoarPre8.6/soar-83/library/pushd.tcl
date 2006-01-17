#
# pushd.tcl --
#
# C-shell style directory stack procs.
#
#------------------------------------------------------------------------------
# Copyright 1992-1994 Karl Lehenbauer and Mark Diekhans.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies.  Karl Lehenbauer and
# Mark Diekhans make no representations about the suitability of this
# software for any purpose.  It is provided "as is" without express or
# implied warranty.
#------------------------------------------------------------------------------
# $Id$
#------------------------------------------------------------------------------
#
# Modified by KBS, 2 Feb. 95, to use standard Tcl list ops. instead
# of TclX lvarpop command.  Also changed "dirs" to return list rather
# than printing it.
#
# Modified by KBS, 8 Mar. 95, to store cwd explicitly in dir stack
# as opposed in implicitly via pwd.  This works better in a multi-
# interpreter environment where different interpreters are doing
# cd's.

#@package: TclX-directory_stack pushd popd dirs

global TCLXENV(dirPushList)

set TCLXENV(dirPushList) "\"[pwd]\""

rename cd tcl-cd
proc cd {{dir $env(HOME)}} {
    global env TCLXENV
    if [catch "tcl-cd \"$dir\"" msg] {
	return $msg
    } else {
	set fulldir [pwd]
	set TCLXENV(dirPushList) \
            [linsert [lrange $TCLXENV(dirPushList) 1 end] \
                     0 "$fulldir"]
	return $fulldir	
    }
}

proc pushd {dir} {
    global TCLXENV

    if [catch "tcl-cd \"$dir\"" msg] {
	return $msg
    } else {
	set dir [pwd]
	set TCLXENV(dirPushList) [linsert $TCLXENV(dirPushList) 0 "$dir"]
	return $dir
    }
}

proc popd {} {
    global TCLXENV

    if {[llength $TCLXENV(dirPushList)] > 1} {
	set prev_dir [lindex $TCLXENV(dirPushList) 1]
	set TCLXENV(dirPushList) [lrange $TCLXENV(dirPushList) 1 end]
	if [catch "tcl-cd \"$prev_dir\"" msg] {
	    return $msg
	} else {
	    return $prev_dir
	}
    } else {
        error "directory stack cannot be empty"
    }
}

proc topd {} {
    global TCLXENV
    return [lindex $TCLXENV(dirPushList) 0]
}

proc dirs {} { 
    global TCLXENV
    return $TCLXENV(dirPushList)
}

