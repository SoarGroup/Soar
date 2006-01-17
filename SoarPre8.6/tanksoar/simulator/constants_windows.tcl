set gifSize 32
set smallgifSize 20

set wMap_h [expr $mapdim * $gifSize]
set wMap_w [expr $wMap_h + 152]
set wMap_x 103
set wMap_y 20

set wControls_w 128
set wControls_h 222
set wControls_x 656
set wControls_y 347

set wSensors_w 200
set wSensors_h [expr ($smallgifSize * $mapdim) + 50]
set wSensors_x 585
set wSensors_y 83

set mapheight $wMap_h
set mapwidth $mapheight
#set gridSize [expr $mapheight/$mapdim]
set gridSize $gifSize
set maxsmell [expr (2 * $mapdim) - 6]

set wglobal_infowidth [expr $wMap_w - $mapwidth]
set wglobal_onestat_h [expr ($wMap_h - 96)/7]
set wglobal_y1 3
set wglobal_y3 [expr $wglobal_onestat_h * 2 / 3]
set wglobal_y2 [expr $wglobal_y1 + ($wglobal_y3 - $wglobal_y1)/2]
set wglobal_y4 [expr $wglobal_onestat_h - 3]

set wglobal_statusheight [expr $wMap_h - 96]

set radar_box [expr ($wSensors_h - 15) / ($mapdim - 1)]
set wtag_radar_width [expr ($radar_box * 4)]
set wtag_sensor_width [expr $wSensors_w - $wtag_radar_width]

set sensor_box [expr $wtag_sensor_width / 2]
set wtag_stat_height [expr ($wSensors_h - (3 * $sensor_box))/5]
set wtag_sensor_height [expr 2 * $sensor_box]

set btn_frame_h [expr $wControls_h / 6]

set minimizeManual 0
set minimizeAgentInfo 0
set minimizeMap 0

#>-=>
# Cache some graphics computation
#>-=>
set realWidth [expr $wglobal_infowidth - 7]
set drawstat_x1 3
set drawstat_x4 [expr $realWidth - 3]
set drawstat_x4_x1 [expr ($drawstat_x4 - $drawstat_x1)] 

set draw_sense_xmargin 4
set draw_sense_ymargin 9
set draw_sense_unit [expr $sensor_box/8]
set draw_sense_x1 [expr $draw_sense_xmargin + $draw_sense_unit]
set draw_sense_x2 [expr $draw_sense_xmargin + $draw_sense_unit*3]
set draw_sense_x3 [expr $draw_sense_xmargin + $draw_sense_unit*5]
set draw_sense_x4 [expr $draw_sense_xmargin + $draw_sense_unit*7]
set draw_sense_y1 [expr $draw_sense_ymargin + $draw_sense_unit]
set draw_sense_y2 [expr $draw_sense_ymargin + $draw_sense_unit*3]
set draw_sense_y3 [expr $draw_sense_ymargin + $draw_sense_unit*5]
set draw_sense_y4 [expr $draw_sense_ymargin + $draw_sense_unit*7]
set draw_sense_text [expr $draw_sense_xmargin + ($sensor_box / 2)]
