#-----------------------------------------------------------------------------
# tclx.tcl -- Extended Tcl initialization.
#-----------------------------------------------------------------------------
# $Id: tclx.tcl,v 1.4 2004/11/23 05:54:16 hobbs Exp $
#-----------------------------------------------------------------------------

namespace eval ::tclx {
    global auto_path auto_index tclx_library
    if {[info exists tclx_library] && [string compare {} $tclx_library]} {
	set auto_index(buildpackageindex) \
		{source [file join $tclx_library buildidx.tcl]}
	if {![info exists auto_path] || \
		![lcontain $auto_path $tclx_library]} {
	    lappend auto_path $tclx_library
	}
    }

    array set libfiles {
	arrayprocs.tcl	1
	autoload.tcl	0
	buildhelp.tcl	0
	buildidx.tcl	0
	compat.tcl	1
	convlib.tcl	1
	edprocs.tcl	1
	events.tcl	1
	fmath.tcl	1
	forfile.tcl	1
	globrecur.tcl	1
	help.tcl	1
	profrep.tcl	1
	pushd.tcl	1
	setfuncs.tcl	1
	showproc.tcl	1
	stringfile.tcl	1
	tcllib.tcl	0
	tclx.tcl	0
    }
    set dir [file dirname [info script]]
    foreach file [array names libfiles] {
	if {$libfiles($file)} {
	    uplevel #0 [list source [file join $dir $file]]
	}
    }
}; # end namespace tclx

# == Put any code you want all Tcl programs to include here. ==
