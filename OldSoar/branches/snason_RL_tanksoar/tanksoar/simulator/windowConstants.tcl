proc constWindow {} {
global widget    
if {![winfo exist constWindow]} {

		  set base .constWindow
		  toplevel $base -class Toplevel \
					 -cursor xterm 

		  #wm focusmodel $base passive
		  wm geometry $base 506x467+191+303
		  wm maxsize $base 1604 1185
		  wm minsize $base 112 1
		  #wm overrideredirect $base 0
		  wm resizable $base 1 1
		  wm deiconify $base
		  wm title $base "Tank-Soar Constants"
 
		  makeConstWindow $base
	 }
}

proc makeConstWindow {base} {
 global shieldCost weaponCost disarmCost regenerateFreq \
		  PU_HealthMax PU_EnergyMax PU_MissilesMax PU_MinesMax \
		  healthIncrease energyIncrease missilesIncrease minesIncrease \
		  decisions_per_update worldCountLimit disarmErrorRate \
		  projectileDamage terrainDamage absorbDamage Pts_Flag_Secured \
		  Pts_Flag_Bonus Pts_Kill_Bonus Pts_Death_Penalty sounddist \
		  initLives projectileSpeed


   checkbutton $base.che18 \
        -text {Use Flags} -variable useFlags 
    checkbutton $base.che19 \
        -text {Use Mines} -variable useMines 
    label $base.lab22 \
        -borderwidth 1 -relief raised -text {Initial Lives} 
    label $base.lab26 \
        -borderwidth 1 -relief raised -text {Health per Powerup} 
    label $base.lab27 \
        -borderwidth 1 -relief raised -text {Energy per Powerup} 
    label $base.lab28 \
        -borderwidth 1 -relief raised -text {Missiles per Powerup} 
    label $base.lab29 \
        -borderwidth 1 -relief raised -text {Mines per Powerup} 
    label $base.lab30 \
        -borderwidth 1 -relief raised -text {Powerup Regeneration Interval} 
    label $base.lab31 \
        -borderwidth 1 -relief raised -text {Health Powerups per Tank} 
    label $base.lab32 \
        -borderwidth 1 -relief raised -text {Energy Powerups per Tank} 
    label $base.lab33 \
        -borderwidth 1 -relief raised -text {Mine Powerups per Tank} 
    label $base.lab34 \
        -borderwidth 1 -relief raised -text {Missile PowerUps per Tank} 
    label $base.lab35 \
        -borderwidth 1 -relief raised -text {Weapon Use Cost} 
    label $base.lab36 \
        -borderwidth 1 -relief raised -text {Weapon Hit Damage} 
    label $base.lab37 \
        -borderwidth 1 -relief raised -text {Energy Tax Per Move} 
    label $base.lab38 \
        -borderwidth 1 -relief raised -text {Mine Disarm Error Rate} 
    label $base.lab39 \
        -borderwidth 1 -relief raised -text {Mine Disarming Cost} 
    label $base.lab40 \
        -borderwidth 1 -relief raised -text {Shield Energy Cost} 
    label $base.lab41 \
        -borderwidth 1 -relief raised -text {Terrain Collision Damage} 
    label $base.lab42 \
        -borderwidth 1 -relief raised -text {Shield Absorb Damage} 
    label $base.lab43 \
        -borderwidth 1 -relief raised -text {My Flag Bonus} 
    label $base.lab44 \
        -borderwidth 1 -relief raised -text {Enemy Flag Bonus} 
    label $base.lab45 \
        -borderwidth 1 -relief raised -text {Death Penalty} 
    label $base.lab46 \
        -borderwidth 1 -relief raised -text {Kill Bonus} 
    label $base.lab47 \
        -borderwidth 1 -relief raised -text {Sound Sensor Range} 
    label $base.lab48 \
        -borderwidth 1 -relief raised -text {Decisions Per Update} 
    label $base.lab49 \
        -borderwidth 1 -relief raised -text {World Count Limit} 
		  
    entry $base.ent53 \
        -textvariable shieldCost 
    entry $base.ent55 \
        -textvariable disarmCost 
    entry $base.ent56 \
        -textvariable regenerateFreq 
    entry $base.ent57 \
        -textvariable PU_HealthMax 
    entry $base.ent58 \
        -textvariable PU_EnergyMax 
    entry $base.ent59 \
        -textvariable PU_MissilesMax 
    entry $base.ent60 \
        -textvariable PU_MinesMax 
    entry $base.ent61 \
        -textvariable healthIncrease 
    entry $base.ent62 \
        -textvariable energyIncrease 
    entry $base.ent63 \
        -textvariable missilesIncrease 
    entry $base.ent64 \
        -textvariable minesIncrease 
    entry $base.ent65 \
        -textvariable decisions_per_update 
    entry $base.ent66 \
        -textvariable worldCountLimit 
    entry $base.ent67 \
        -textvariable disarmErrorRate 
    entry $base.ent68 \
        -textvariable projectileDamage 
    entry $base.ent69 \
        -textvariable terrainDamage 
    entry $base.ent70 \
        -textvariable absorbDamage 
   entry $base.ent71 \
        -textvariable Pts_Flag_Secured 
    entry $base.ent72 \
        -textvariable Pts_Flag_Bonus 
    entry $base.ent73 \
        -textvariable Pts_Kill_Bonus 
    entry $base.ent74 \
        -textvariable Pts_Death_Penalty 
    entry $base.ent75 \
        -textvariable sounddist 
    entry $base.ent76 \
        -textvariable initLives 
    label $base.lab77 \
        -borderwidth 1 -relief raised -text {Projectile Speed} 
    entry $base.ent78 \
        -textvariable projectileSpeed 

    place $base.che18 \
        -x 5 -y 5 -anchor nw -bordermode ignore 
    place $base.che19 \
        -x 5 -y 35 -anchor nw -bordermode ignore 
    place $base.lab22 \
        -x 10 -y 80 -anchor nw -bordermode ignore 
    place $base.lab26 \
        -x 10 -y 350 -anchor nw -bordermode ignore 
    place $base.lab27 \
        -x 10 -y 375 -anchor nw -bordermode ignore 
    place $base.lab28 \
        -x 10 -y 400 -anchor nw -bordermode ignore 
    place $base.lab29 \
        -x 10 -y 425 -anchor nw -bordermode ignore 
    place $base.lab30 \
        -x 10 -y 225 -anchor nw -bordermode ignore 
    place $base.lab31 \
        -x 10 -y 250 -anchor nw -bordermode ignore 
    place $base.lab32 \
        -x 10 -y 275 -anchor nw -bordermode ignore 
    place $base.lab33 \
        -x 10 -y 325 -anchor nw -bordermode ignore 
    place $base.lab34 \
        -x 10 -y 300 -anchor nw -bordermode ignore 
    place $base.lab35 \
        -x 10 -y 165 -anchor nw -bordermode ignore 
    place $base.lab36 \
        -x 285 -y 105 -anchor nw -bordermode ignore 
    place $base.lab37 \
        -x 10 -y 115 -anchor nw -bordermode ignore 
    place $base.lab38 \
        -x 285 -y 80 -anchor nw -bordermode ignore 
    place $base.lab39 \
        -x 10 -y 190 -anchor nw -bordermode ignore 
    place $base.lab40 \
        -x 10 -y 140 -anchor nw -bordermode ignore 
    place $base.lab41 \
        -x 285 -y 130 -anchor nw -bordermode ignore 
    place $base.lab42 \
        -x 285 -y 155 -anchor nw -bordermode ignore 
    place $base.lab43 \
        -x 285 -y 200 -anchor nw -bordermode ignore 
    place $base.lab44 \
        -x 285 -y 225 -anchor nw -bordermode ignore 
    place $base.lab45 \
        -x 285 -y 275 -anchor nw -bordermode ignore 
    place $base.lab46 \
        -x 285 -y 250 -anchor nw -bordermode ignore 
    place $base.lab47 \
        -x 285 -y 310 -anchor nw -bordermode ignore 
    place $base.lab48 \
        -x 285 -y 10 -anchor nw -bordermode ignore 
    place $base.lab49 \
        -x 285 -y 35 -anchor nw -bordermode ignore 
    place $base.ent53 \
        -x 225 -y 135 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent55 \
        -x 225 -y 185 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent56 \
        -x 225 -y 220 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent57 \
        -x 225 -y 245 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent58 \
        -x 225 -y 270 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent59 \
        -x 225 -y 295 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent60 \
        -x 225 -y 320 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent61 \
        -x 225 -y 345 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent62 \
        -x 225 -y 370 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent63 \
        -x 225 -y 395 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent64 \
        -x 225 -y 420 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent65 \
        -x 450 -y 5 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent66 \
        -x 450 -y 30 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent67 \
        -x 450 -y 75 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent68 \
        -x 450 -y 100 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent69 \
        -x 450 -y 125 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent70 \
        -x 450 -y 150 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent71 \
        -x 450 -y 195 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent72 \
        -x 450 -y 220 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent73 \
        -x 450 -y 245 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent74 \
        -x 450 -y 270 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent75 \
        -x 450 -y 305 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.ent76 \
        -x 225 -y 75 -width 40 -height 25 -anchor nw -bordermode ignore 
    place $base.lab77 \
        -x 285 -y 340 -anchor nw -bordermode ignore 
    place $base.ent78 \
        -x 450 -y 335 -width 40 -height 25 -anchor nw -bordermode ignore 
}
