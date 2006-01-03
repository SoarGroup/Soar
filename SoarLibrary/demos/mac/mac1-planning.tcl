#
#  Missionaries and Cannibals Demonstration
#
#-----------------------------------------------------------------------
#
# Tcl/Tk implementation of the Missionaries and Cannibals Demonstration.
#
#	last update: 6.9.00 Acar Altinsel
#

package require Soar

set boatsent 0

proc releaseboat {} {
	global boatsent
	set boatsent 0
}

proc sendboat {bank passenger num numtypes ml cl mr cr} {
	global boatsent
	if {$boatsent == 0} {
		if {$numtypes == 2} {
			if {$bank == "L1"} {
				drawstate [expr $ml - 1] [expr $cl - 1] $mr $cr 0
				animate 1 1 l

			}
			if {$bank == "R1"} {
				drawstate $ml $cl [expr $mr - 1] [expr $cr - 1] 0
				animate 1 1 r
			}
		}
		if {$numtypes == 1} {
			if {$passenger == "missionaries"} {
				if {$num == 1} {
					if {$bank == "L1"} {
						drawstate [expr $ml - 1] $cl $mr $cr 0
						animate 1 0 l
					}
					if {$bank == "R1"} {
						drawstate $ml $cl [expr $mr - 1] $cr 0
						animate 1 0 r
					}
				}						
			 	if {$num == 2} {
					if {$bank == "L1"} {
						drawstate [expr $ml - 2] $cl $mr $cr 0
						animate 2 0 l
					}
					if {$bank == "R1"} {
						drawstate $ml $cl [expr $mr - 2] $cr 0
						animate 2 0 r
					}
				}
			}

			if {$passenger == "cannibals"} {
				if {$num == 1} {
					if {$bank == "L1"} {
						drawstate $ml [expr $cl - 1] $mr $cr 0
						animate 0 1 l
					}
					if {$bank == "R1"} {
						drawstate $ml $cl $mr [expr $cr - 1] 0
						animate 0 1 r
					}
				}				
			 	if {$num == 2} {
					if {$bank == "L1"} {
						drawstate $ml [expr $cl - 2] $mr $cr 0
						animate 0 2 l
					}
					if {$bank == "R1"} {
						drawstate $ml $cl $mr [expr $cr - 2] 0
						animate 0 2 r
					}
				}
			}
		}
	}
	set boatsent 1
}

proc animate {m c b} {

	set i 0
	if {$b == "l"} {
		if {$m == 2} {
			.mac.frame.can create image 200 370 -anchor se -image missionary -tag onboat
			.mac.frame.can create image 230 370 -anchor se -image missionary -tag onboat
			.mac.frame.can create image 200 365 -image boat -tag onboat			
			while {$i < 220} {
				set i [expr $i + 1]	
				set j 0
				after 1 set j 1
				vwait j
				.mac.frame.can move onboat 1 0 
			}
		}
		if {$c == 2} {
			.mac.frame.can create image 203 370 -anchor se -image cannibal -tag onboat
			.mac.frame.can create image 243 370 -anchor se -image cannibal -tag onboat
			.mac.frame.can create image 200 365 -image boat -tag onboat			
			while {$i < 220} {
				set i [expr $i + 1]	
				set j 0
				after 1 set j 1
				vwait j
				.mac.frame.can move onboat 1 0 
			}
		}
		if {$m == 1} {
			if {$c == 0} {
			
				.mac.frame.can create image 216 370 -anchor se -image missionary -tag onboat
				.mac.frame.can create image 200 365 -image boat -tag onboat			
				while {$i < 220} {
					set i [expr $i + 1]	
					set j 0
					after 1 set j 1
					vwait j
					.mac.frame.can move onboat 1 0 
				}
			}
			if {$c == 1} {
				.mac.frame.can create image 202 370 -anchor se -image missionary -tag onboat
				.mac.frame.can create image 242 370 -anchor se -image cannibal -tag onboat
				.mac.frame.can create image 200 365 -image boat -tag onboat			
				while {$i < 220} {
					set i [expr $i + 1]	
					set j 0
					after 1 set j 1
					vwait j
					.mac.frame.can move onboat 1 0 
				}				

			}
		}
		if {$c == 1} {
			if {$m == 0} {
				.mac.frame.can create image 224 370 -anchor se -image cannibal -tag onboat
				.mac.frame.can create image 200 365 -image boat -tag onboat			
				while {$i < 220} {
					set i [expr $i + 1]	
					set j 0
					after 1 set j 1
					vwait j
					.mac.frame.can move onboat 1 0 
				}				
			}
		}
	}
	if {$b == "r"} {
		if {$m == 2} {
			.mac.frame.can create image 420 370 -anchor se -image missionary -tag onboat
			.mac.frame.can create image 450 370 -anchor se -image missionary -tag onboat
			.mac.frame.can create image 420 365 -image boat -tag onboat			
			while {$i < 220} {
				set i [expr $i + 1]	
				set j 0
				after 1 set j 1
				vwait j
				.mac.frame.can move onboat -1 0 
			}
		}
		if {$c == 2} {
			.mac.frame.can create image 423 370 -anchor se -image cannibal -tag onboat
			.mac.frame.can create image 463 370 -anchor se -image cannibal -tag onboat
			.mac.frame.can create image 420 365 -image boat -tag onboat			
			while {$i < 220} {
				set i [expr $i + 1]	
				set j 0
				after 1 set j 1
				vwait j
				.mac.frame.can move onboat -1 0 
			}
		}
		if {$m == 1} {
			if {$c == 0} {
			
				.mac.frame.can create image 436 370 -anchor se -image missionary -tag onboat
				.mac.frame.can create image 420 365 -image boat -tag onboat			
				while {$i < 220} {
					set i [expr $i + 1]	
					set j 0
					after 1 set j 1
					vwait j
					.mac.frame.can move onboat -1 0 
				}
			}
			if {$c == 1} {
				.mac.frame.can create image 422 370 -anchor se -image missionary -tag onboat
				.mac.frame.can create image 462 370 -anchor se -image cannibal -tag onboat
				.mac.frame.can create image 420 365 -image boat -tag onboat			
				while {$i < 220} {
					set i [expr $i + 1]	
					set j 0
					after 1 set j 1
					vwait j
					.mac.frame.can move onboat -1 0 
				}				

			}
		}
		if {$c == 1} {
			if {$m == 0} {
				.mac.frame.can create image 443 370 -anchor se -image cannibal -tag onboat
				.mac.frame.can create image 420 365 -image boat -tag onboat			
				while {$i < 220} {
					set i [expr $i + 1]	
					set j 0
					after 1 set j 1
					vwait j
					.mac.frame.can move onboat -1 0 
				}				
			}
		}
	}
}



proc drawscreen {w} {
	catch {destroy $w}
	toplevel $w
	
	wm title $w "Missionaries and Cannibals Demonstration"
	wm iconname $w "Missionaries and Cannibals"
	message $w.msg -font -Adobe-times-medium-r-normal--*-180* \
            	-aspect 400 -text "Missionaries and Cannibals Demonstration"
	frame $w.frame -width 480 -height 360 -borderwidth 4 \
	    	-relief ridge -bg Grey70

	button $w.solve -text Solve -command "run"
	button $w.stop -text "Stop" -command "stop-soar"
	button $w.init -text "Init Soar" -command "init-soar; drawstate 3 3 0 0 l"
	button $w.exch -text "Excise Chunks" -command "excise -chunks"
	button $w.ok -text Close -command "dismiss $w"
       
	pack $w.msg -side top
	pack $w.frame -side top -padx 20
	pack $w.solve $w.stop $w.init $w.exch -side left -padx 1m -pady 2m
	pack $w.ok -side right -padx 1m -pady 2m 	

	drawstate 3 3 0 0 l		
}

proc dismiss {w} {
    excise -all
    init-soar
    destroy $w
}


#-----------------------------------------------------------------------
# The test.tcl stuff...
#

proc drawstate {ml cl mr cr b} {
    global soar_library

catch {destroy .mac.frame.can}
canvas .mac.frame.can -width 640 -height 480 -background white
pack .mac.frame.can

image create photo land -file "[file join $soar_library .. demos mac land.gif]"
.mac.frame.can create image 0 0 -image land -anchor nw

image create photo cannibal -file "[file join $soar_library .. demos mac cannibal.gif]"
image create photo missionary -file "[file join $soar_library .. demos mac missionary.gif]"
image create photo boat -file "[file join $soar_library .. demos mac boat.gif]"


if {$ml >= 1} {
	.mac.frame.can create image 90 375 -image missionary -tag move_m1
}
if {$ml >= 2} {
	.mac.frame.can create image 60 375 -image missionary -tag move_m2
}
if {$ml >= 3} {
	.mac.frame.can create image 30 375 -image missionary -tag move_m3
}

if {$cl >= 1} {
	.mac.frame.can create image 170 310 -image cannibal -tag move_c1
}
if {$cl >= 2} {
	.mac.frame.can create image 130 310 -image cannibal -tag move_c2
}
if {$cl >= 3} {
	.mac.frame.can create image 90 310 -image cannibal -tag move_c3
}

if {$b == "l"} {
	.mac.frame.can create image 200 365 -image boat -tag move_b
}


if {$mr >= 1} {
	.mac.frame.can create image 540 375 -image missionary -tag move_m1
}
if {$mr >= 2} {
	.mac.frame.can create image 570 375 -image missionary -tag move_m2
}
if {$mr >= 3} {
	.mac.frame.can create image 600 375 -image missionary -tag move_m3
}

if {$cr >= 1} {
	.mac.frame.can create image 490 310 -image cannibal -tag move_c1
}
if {$cr >= 2} {
	.mac.frame.can create image 530 310 -image cannibal -tag move_c2
}
if {$cr >= 3} {
	.mac.frame.can create image 570 310 -image cannibal -tag move_c3
}

if {$b == "r"} {
	.mac.frame.can create image 420 365 -image boat -tag move_b
}

.mac.frame.can bind move_c1 <Button-1> {
	global c1x c1y
	
	set c1x %x
	set c1y %y
}

.mac.frame.can bind move_c1 <B1-Motion> {
	global c1x c1y
	
	set newx %x
	set newy %y

	set distx [expr $newx - $c1x]
	set disty [expr $newy - $c1y]

	.mac.frame.can move move_c1 $distx $disty

	set c1x $newx
	set c1y $newy
}

}

#-----------------------------------------------------------------------
#
# Initialization and GUI creation

# Load rules defining missionaries and cannibals solution method

global soar_library
global interp_type

set ml 3
set cl 3
set bl 1

set mr 0
set cr 0
set br 0

excise -all
source mac1-planning.soar

drawscreen .mac


###
### MOVE-MAC-BOAT MONITOR OPERATOR AND STATE
###

sp {monitor*move-mac-boat
   (state <s> ^operator <o>
	      ^left-bank <l>
	      ^right-bank <r>)
   (<l> ^missionaries <ml>
        ^cannibals <cl>)
   (<r> ^missionaries <mr>
        ^cannibals <cr>)
   (<o> ^name move-mac-boat
        ^{ << cannibals missionaries >>  <type> } <number>
	^types <x>
	^bank <bank>)
   -->
   (tcl | sendboat | <bank> | | <type> | | <number> | | <x> | | <ml> | | <cl> | | <mr> | | <cr>)
   (write (crlf) | Move | <number> | | <type>)}

sp {monitor*state*left
   (state <s> ^name mac
              ^left-bank <l>
              ^right-bank <r>)
   (<l> ^missionaries <ml>
        ^cannibals <cl>
        ^boat 1)
   (<r> ^missionaries <mr>
        ^cannibals <cr>
        ^boat 0)
   -->
   (tcl | drawstate | <ml> | | <cl> | | <mr> | | <cr> | l |)
   (tcl | releaseboat |)
   (write (crlf) | M: | <ml> |, C: | <cl> | B ~~~   | 
                 | M: | <mr> |, C: | <cr> |  |)}

sp {monitor*state*right
   (state <s> ^name mac
              ^left-bank <l>
              ^right-bank <r>)
   (<l> ^missionaries <ml>
        ^cannibals <cl>
        ^boat 0)
   (<r> ^missionaries <mr>
        ^cannibals <cr>
        ^boat 1)
   -->
   (tcl | drawstate | <ml> | | <cl> | | <mr> | | <cr> | r |)
   (tcl | releaseboat |)
   (write (crlf) | M: | <ml> |, C: | <cl> |   ~~~ B | 
                 | M: | <mr> |, C: | <cr> |  |)}

###
