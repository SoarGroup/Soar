set lastSymNum 0

proc genSym {} {
	global lastSymNum
	
	incr lastSymNum
	return sym$lastSymNum
}

proc ldelete {theList itemToDelete} {
		set itemIndex [lsearch -exact $theList $itemToDelete]
		if {$itemIndex >= 0} {
		  return [lreplace $theList $itemIndex $itemIndex]
		} else {
		  return $theList
		}
}

proc CallTrace { {file stderr} } {
	puts $file "Tcl Call Trace"
	for {set x [expr [info level] -1]} {$x > 0} {incr x -1} {
		puts $file "$x: [info level $x]"
	}
}

proc Debug { string } {
	global debug
	if ![info exists debug(enabled)] {
		return
	}
	puts $debug(file) $string
}

proc DebugOn { {file {}}} {
	global debug
	set debug(enabled) 1
	if {[string length $file] == 0} {
		set debug(file) stderr
	} else {
		if [catch {open $file w} fileID] {
			puts stderr "Cannot open $file: $fileID"
			set debug(file) stderr
		} else {
			puts stderr "Debug info to $file"
			set debug(file) $fileID
		}
	}
}

proc DebugOff {} {
	global debug
	if [info exists debug(enabled)] {
		unset debug(enabled)
		flush $debug(file)
		if {$debug(file) ~= "stderr" && $debug(file) != "stdout"} {
			close $debug(file)
			unset $debug(file)
		}
	}
}

proc random {range} {
  return [expr int(rand()*$range)+1]
}

proc randList { listLength } {

	set startList [list]
	set finalList [list]
	
	for {set i 0} {$i < $listLength} {incr i 1} {
		lappend startList $i
	}
	set count 0
	for {set i [llength $startList]} {($i > 0) && ($count < 20)} {set i [llength $startList]} {
		set newItem [lindex $startList [expr [random $i] - 1]]
		lappend finalList $newItem
		set startList [ldelete $startList $newItem]
		incr count 1
	}
return $finalList
}

proc logicalX {w obj} {
	global gridSize
	return [expr int([lindex [.wGlobal.map coords $obj] 0] / $gridSize)]
}

proc logicalY {w obj} {
	global gridSize
	return [expr int([lindex [.wGlobal.map coords $obj] 1] / $gridSize)]
}

proc convertToLogical {n} {
	global gridSize
	return [expr int(ceil(double($n)/$gridSize))]
}

proc convertToLogical2 {n} {
	global gridSize
	return [expr int(floor(double($n)/$gridSize))]
}

proc onOrOff {n} {
	if {$n} {
		return on
	} else {
		return off
	}
}

proc toggleOn {n} {
	if {$n == "off"} {
		return on
	} else {
		return off
	}
}

proc yesOrNo {n} {
	if {$n} {
		return yes
	} else {
		return no
	}
}

proc dir-2-args {dir_name} {

	if {$dir_name == "forward"} {
		return {1 0 0 0}
	} elseif {$dir_name == "backward"} {
		return {0 0 1 0}
	} elseif {$dir_name == "right"} {
		return {0 1 0 0}
	} elseif {$dir_name == "left"} {
		return {0 0 0 1}
	}
	return {0 0 0 0}
}

proc args-2-dir {n s e w} {

	if {$n == 1} {
		return north
	}
	if {$s == 1} {
		return south
	}
	if {$e == 1} {
		return east
	}
	if {$w == 1} {
		return west
	}

}

proc onHealth {x y} {
	global powerUpList
	
	if {[info exists powerUpList($x,$y)] && \
		($powerUpList($x,$y) == "health")} {
		return 1
	} else { 
		return 0
	}
}

proc onEnergy {x y} {
	global powerUpList
	
	if {[info exists powerUpList($x,$y)] && \
		($powerUpList($x,$y) == "energy")} {
		return 1
	} else { 
		return 0
	}
}


