set t1_right [image create photo -file ./images/tank_right.gif]
set t1_left [image create photo -file ./images/tank_left.gif]
set t1_up [image create photo -file ./images/tank_up.gif]
set t1_down [image create photo -file ./images/tank_down.gif]

set i_ground1 [image create photo -file ./images/ground1.gif]
set i_ground2 [image create photo -file ./images/ground1.gif]
set i_ground3 [image create photo -file ./images/ground3.gif]

set i_obstruct1 [image create photo -file ./images/tree1.gif]
set i_obstruct2 [image create photo -file ./images/tree2.gif]
set i_obstruct3 [image create photo -file ./images/tree3.gif]
set i_obstruct4 $i_obstruct1
set i_obstruct5 $i_obstruct2
set i_obstruct6 $i_obstruct3
set i_obstruct7 $i_obstruct1
set i_obstruct8 $i_obstruct3
set i_obstruct9 $i_obstruct3
set i_obstruct10 $i_obstruct2

set i_mountain1 [image create photo -file ./images/rock1.gif]
set i_mountain2 [image create photo -file ./images/rock2.gif]
set i_mountain3 [image create photo -file ./images/rock3.gif]

set i_oasisb [image create photo -file ./images/oasisb.gif]
set i_oasish [image create photo -file ./images/oasish.gif]
set i_rechargeh [image create photo -file ./images/recharge.gif]
set i_rechargeb [image create photo -file ./images/battrecharge.gif]
set i_explosion [image create photo -file ./images/explosion.gif]
set i_fireball [image create photo -file ./images/fire1.gif]
set i_question [image create photo -file ./images/question.gif]
set i_battery [image create photo -file ./images/battery.gif]
set i_health [image create photo -file ./images/health.gif]
set i_missile [image create photo -file ./images/missile.gif]
set i_mine [image create photo -file ./images/mine.gif]
set i_buried_mine [image create photo -file ./images/ground_mined.gif]

set i_tank_small [image create photo -file ./images/small/tank.gif]
set i_ground_small [image create photo -file ./images/small/ground.gif]
set i_obstruct_small [image create photo -file ./images/small/tree.gif]
set i_oasisb_small [image create photo -file ./images/small/oasisb.gif]
set i_oasish_small [image create photo -file ./images/small/oasish.gif]
set i_battery_small [image create photo -file ./images/small/battery.gif]
set i_missile_small [image create photo -file ./images/small/missile.gif]
set i_mine_small [image create photo -file ./images/small/mine.gif]
set i_health_small [image create photo -file ./images/small/health.gif]
set i_buried_mine_small [image create photo -file ./images/small/ground_mined.gif]

#foreach p {t1_right t1_left t1_up t1_down i_ground1 i_ground2 i_ground3 \
#			i_obstruct1 i_obstruct2 i_obstruct3 i_mountain1 i_mountain2 i_mountain3 \
#			i_rechargeh i_rechargeb i_explosion i_fireball i_battery i_health \
#			i_mine i_buried_mine} {
#		eval [subst "$$p configure -width 20 -height 20"]
#}
#set gridSize 20
