
set c1x 0 
set c1y 0
global c1x c1y

set c2x 0 
set c2y 0
global c2x c2y

set c3x 0 
set c3y 0
global c3x c3y

set m1x 0 
set m1y 0
global m1x m1y

set m2x 0 
set m2y 0
global m2x m2y

set m3x 0 
set m3y 0
global m3x m3y

set bx 0 
set by 0
global bx 

catch {destroy .can}
canvas .can -width 640 -height 480 -background white
pack .can


image create photo land -file land.gif
.can create image 0 0 -image land -anchor nw

image create photo cannibal -file cannibal.gif
image create photo missionary -file missionary.gif

.can create image 90 310 -image cannibal -tag move_c1
.can create image 130 310 -image cannibal -tag move_c2
.can create image 170 310 -image cannibal -tag move_c3

.can create image 30 375 -image missionary -tag move_m1
.can create image 60 375 -image missionary -tag move_m2
.can create image 90 375 -image missionary -tag move_m3

image create photo boat -file boat.gif
.can create image 200 365 -image boat -tag move_b

.can bind move_c1 <Button-1> {
	global c1x c1y
	
	set c1x %x
	set c1y %y
}

.can bind move_c1 <B1-Motion> {
	global c1x c1y
	
	set newx %x
	set newy %y

	set distx [expr $newx - $c1x]
	set disty [expr $newy - $c1y]

	.can move move_c1 $distx $disty

	set c1x $newx
	set c1y $newy
}

.can bind move_c2 <Button-1> {
	global c2x c2y
	
	set c2x %x
	set c2y %y
}

.can bind move_c2 <B1-Motion> {
	global c2x c2y
	
	set newx %x
	set newy %y

	set distx [expr $newx - $c2x]
	set disty [expr $newy - $c2y]

	.can move move_c2 $distx $disty

	set c2x $newx
	set c2y $newy
}
.can bind move_c3 <Button-1> {
	global c3x c3y
	
	set c3x %x
	set c3y %y
}

.can bind move_c3 <B1-Motion> {
	global c3x c3y
	
	set newx %x
	set newy %y

	set distx [expr $newx - $c3x]
	set disty [expr $newy - $c3y]

	.can move move_c3 $distx $disty

	set c3x $newx
	set c3y $newy
}

.can bind move_m1 <Button-1> {
	global m1x m1y
	
	set m1x %x
	set m1y %y
}

.can bind move_m1 <B1-Motion> {
	global m1x m1y
	
	set newx %x
	set newy %y

	set distx [expr $newx - $m1x]
	set disty [expr $newy - $m1y]

	.can move move_m1 $distx $disty

	set m1x $newx
	set m1y $newy
}


.can bind move_m2 <Button-1> {
	global m2x m2y
	
	set m2x %x
	set m2y %y
}

.can bind move_m2 <B1-Motion> {
	global m2x m2y
	
	set newx %x
	set newy %y

	set distx [expr $newx - $m2x]
	set disty [expr $newy - $m2y]

	.can move move_m2 $distx $disty

	set m2x $newx
	set m2y $newy
}

.can bind move_m3 <Button-1> {
	global m3x m3y
	
	set m3x %x
	set m3y %y
}

.can bind move_m3 <B1-Motion> {
	global m3x m3y
	
	set newx %x
	set newy %y

	set distx [expr $newx - $m3x]
	set disty [expr $newy - $m3y]

	.can move move_m3 $distx $disty

	set m3x $newx
	set m3y $newy
}

.can bind move_b <Button-1> {
	global bx by
	
	set bx %x
	set by %y
}

.can bind move_b <B1-Motion> {
	global bx by
	
	set newx %x
	set newy %y

	set distx [expr $newx - $bx]
	set disty [expr $newy - $by]

	.can move move_b $distx $disty

	set bx $newx
	set by $newy
}

