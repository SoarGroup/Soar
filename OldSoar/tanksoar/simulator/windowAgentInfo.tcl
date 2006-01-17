canvas .wAgentInfo.statsensors -height $wSensors_h -width $wtag_sensor_width -relief raised \
	-background $backgroundColor
pack .wAgentInfo.statsensors -side left

canvas .wAgentInfo.statsensors.hstat -height $wtag_stat_height -width $wtag_sensor_width -relief raised \
	-background $backgroundColor
canvas .wAgentInfo.statsensors.lstat -height $wtag_stat_height -width $wtag_sensor_width -relief raised \
	-background $backgroundColor
canvas .wAgentInfo.statsensors.bstat -height $wtag_stat_height -width $wtag_sensor_width -relief raised \
	-background $backgroundColor
pack .wAgentInfo.statsensors.hstat -side top
pack .wAgentInfo.statsensors.lstat -side top
pack .wAgentInfo.statsensors.bstat -side top

# Three Status Bars

drawbar .wAgentInfo.statsensors.hstat "Health" 8 healthlabel healthbar 100 100
drawbar .wAgentInfo.statsensors.bstat "Radar Setting" 8 radarlabel radarbar 100 100
drawbar .wAgentInfo.statsensors.lstat "Energy" 8 energylabel energybar 100 100

# Sensor Boxes

canvas .wAgentInfo.statsensors.sense -height $wtag_sensor_height -width $wtag_sensor_width \
	-relief raised -background $backgroundColor
pack .wAgentInfo.statsensors.sense -side top

canvas .wAgentInfo.statsensors.sense.c1 -height $sensor_box -width $wtag_sensor_width \
	-relief raised -background $backgroundColor
pack .wAgentInfo.statsensors.sense.c1 -side top
canvas .wAgentInfo.statsensors.sense.c1.infrared -height $sensor_box -width $sensor_box \
	-relief raised -background $backgroundColor
canvas .wAgentInfo.statsensors.sense.c1.radar -height $sensor_box -width $sensor_box \
	-relief raised -background $backgroundColor
pack .wAgentInfo.statsensors.sense.c1.infrared .wAgentInfo.statsensors.sense.c1.radar -side left

drawsensor .wAgentInfo.statsensors.sense.c1.radar "RWaves" radarsensors none white "north" 0 0 0 0
drawsensor .wAgentInfo.statsensors.sense.c1.infrared "Blocked" irsensors none white "north" 0 0 0 0

canvas .wAgentInfo.statsensors.sense.c2 -height $sensor_box -width $wtag_sensor_width \
	-relief raised -background $backgroundColor
pack .wAgentInfo.statsensors.sense.c2 -side top
canvas .wAgentInfo.statsensors.sense.c2.sound -height $sensor_box -width $sensor_box \
	-relief raised -background $backgroundColor
pack .wAgentInfo.statsensors.sense.c2.sound -side left

drawsensor .wAgentInfo.statsensors.sense.c2.sound "Sound"  soundsensors none white "north" 0 0 0 0

canvas .wAgentInfo.statsensors.sense.c3 -height $sensor_box -width $wtag_sensor_width \
	-relief raised -background $backgroundColor
pack .wAgentInfo.statsensors.sense.c3 -side top
canvas .wAgentInfo.statsensors.sense.c3.incoming -height $sensor_box -width $sensor_box \
	-relief raised -background $backgroundColor
pack .wAgentInfo.statsensors.sense.c3.incoming  -side left

drawsensor .wAgentInfo.statsensors.sense.c3.incoming "Incomng" incomingsensors none white "north" 0 0 0 0

canvas .wAgentInfo.statsensors.sense.smell -height $wtag_stat_height -width $wtag_sensor_width \
	-relief raised -background $backgroundColor
pack .wAgentInfo.statsensors.sense.smell -side top
drawbar .wAgentInfo.statsensors.sense.smell "Smell" 8 smelllabel smellbar 5 5

# radar Sensor Grid

canvas .wAgentInfo.radar -height $wSensors_h -width $wtag_radar_width -relief raised \
	-background $backgroundColor

pack .wAgentInfo.radar -side left
.wAgentInfo.radar create text [expr round($radar_box * 1.5) + 8] 10 \
	-text "radar" -tag radar

set j 0
while {$j <= [expr $mapdim - 2]} {
    set i 0
    while {$i < 3} {
		    .wAgentInfo.radar create image \
				[expr $i * $radar_box + ($radar_box / 4) + 3] \
				[expr ($mapdim - $j - 2) * $radar_box + 19 - 4] \
				-image $i_question -tags question -anchor nw
	incr i 1
   }
   incr j 1
}
