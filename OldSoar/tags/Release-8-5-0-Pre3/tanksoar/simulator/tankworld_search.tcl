global mapdim

set neighbors(north) [list north west east]
set neighbors(south) [list south west east]
set neighbors(west) [list north south west]
set neighbors(east) [list north east south]
set i 0
while {$i < $mapdim} {
    set j 0
    while {$j < $mapdim} {
	set visited($i,$j) 0
	incr j 1
    }
    incr i 1
}

proc search {x y l self} {
    global visited mapdim sounddist
    
    set dlimit $sounddist
    
    set i [expr $x - $dlimit]
    if {$i < 0} {
	set i 0
    }
    set maxi [expr $x + $dlimit + 2]
    if {$maxi > $mapdim} {
	set maxi $mapdim
    }
    set minj [expr $y - $dlimit]
    if {$minj < 0} {
	set minj 0
    }
    set maxj [expr $y + $dlimit + 2]
    if {$maxj > $mapdim} {
	set maxj $mapdim
    }
    while {$i < $maxi} {
	set j $minj
	while {$j < $maxj} {
	    set visited($i,$j) 0
	    incr j 1
	}
	incr i 1
    }
    return [BFsearch $x $y $l]
}

proc BFsearch {x y l} {
    global neighbors sounddist map mapdim visited
    
    set queue [list]
    set visited($x,$y) 1

    foreach i $l {
	if {$i == "west"} {
	    set newx [expr $x - 1]
	} elseif {$i == "east"} {
	    set newx [expr $x + 1]
	} else {
	    set newx $x
	}
	if {$i == "north"} {
	    set newy [expr $y - 1]
	} elseif {$i == "south"} {
	    set newy [expr $y + 1]
	} else {
	    set newy $y
	}
	if {($map($newx,$newy) != 1) && ($visited($newx,$newy) != 1)} {
	    if {$map($newx,$newy) != 0} {
		return $i
	    } else {
		set visited($newx,$newy) 1		
		lappend queue [list $newx $newy $neighbors($i) $i 1]
	    }
	}
    }
    
    while {[llength $queue] > 0} {
	set i [lindex $queue 0]
	set queue [lrange $queue 1 end]
	set oldx [lindex $i 0]
	set oldy [lindex $i 1]
	foreach j [lindex $i 2] {
	    if {$j == "west"} {
		set newx [expr $oldx - 1]
	    } elseif {$j == "east"} {
		set newx [expr $oldx + 1]
	    } else {
		set newx $oldx
	    }
	    if {$j == "north"} {
		set newy [expr $oldy - 1]
	    } elseif {$j == "south"} {
		set newy [expr $oldy + 1]
	    } else {
		set newy $oldy
	    }
	    if {($map($newx,$newy) != 1) && ($visited($newx,$newy) != 1)} {
		if {$map($newx,$newy) != 0} {
		    return [lindex $i 3]
		} else {
		    set visited($newx,$newy) 1		
		    if {[expr [lindex $i 4] + 1] < $sounddist} {
			lappend queue [list $newx $newy $neighbors($j) [lindex $i 3] \
				[expr [lindex $i 4] + 1]]
		    }
		}
	    }
	}
    }
    return 0
}
