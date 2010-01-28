### $Id$
### $Log$
### Revision 1.1  2005/06/01 19:14:13  rmarinie
### initial commit of sml eaters
###
### Revision 1.1.1.1  2003/06/16 13:48:36  swallace
### eaters initial cvs version (3.0.5)
###
### Revision 1.4  1998/10/26 15:48:50  swallace
### Prior to Release for 494
###
### Revision 1.3  1998/10/25 14:52:30  swallace
### Synced w/ Mazin
###
### Revision 1.2  1998/10/05 12:55:48  swallace
### Release 2.0a (w/ new tsi30alpha) and new control panel etc.
###
### Revision 1.1  1998/10/02 13:49:18  swallace
### Initial revision
###
### Revision 5.3  1998/08/21 12:10:46  swallace
### seems to work with remote agent proprioceptic feedback
###
### Revision 5.2  1998/08/19 13:03:54  swallace
###  All Fixes :) except for propreoceptic feedback
###
### Revision 5.1  1998/06/02 16:51:22  swallace
### 2 side step & run
###
### Revision 4.1  1998/06/01 22:15:06  swallace
### works using step on both sides.
###
### Revision 3.1  1998/05/21 18:33:56  swallace
### Client/Server Interface allows info to be passed to client.
###
### Revision 1.2  1998/05/19 21:43:20  swallace
### slightly better, goes w/ version 2.2 of most other source
###


global currentColor
set currentColor 0

randomMap

global xplacer yplacer
set xplacer 100
set yplacer 100

###KJC added this from Eaters-3.0.8/tsi-defaults.tcl
## set tsiConfig(debug) to 1 when debugging, 0 when not
set tsiConfig(debug) 0

## if expertise is 1, it is the full interface,
## if 0, it is for very very first time users
set tsiConfig(expertise) 1


set tsiConfig(simulationName) {Eaters}
set tsiConfig(hasEnvironment) 1

set tsiConfig(ControlPanel) ETControlPanel
set tsiConfig(ControlPanelVersion) 3.2.0

set tsiSuppressMenus(demos) 1
### end KJC addition


### Change around the File menu
### delete the original Exit entry, so it can stay at the bottom
#.menu.file.m delete Exit
#.menu.file.m add command -label "Reload Eater name menu" \
#       -command {loadEaterNames .agents.create.e}
#.menu.file.m add command -label Exit -command exit
#
#### Agent Creation Menu:
#global currentAgentSourceFile
#.agents.create.createAgentButton configure -text {Create Eater} \
#    -command {startEater .wGlobal.c $tsiCurrentAgentSourceFile $currentColor}
#
#pack forget .agents.create.createAgentButton
#pack .agents.create.createAgentButton -side left


## Get rid of the normal TSI focus behavior
#bind . <Enter> {}


#frame .mapcontrol -borderwidth 2

#frame .mapcontrol.inner

#global eaterList eaterTick eaterScore
#button .mapcontrol.inner.newmap -text {New Map} \
#          -command {makeBoard $boardX $boardY}
#button .mapcontrol.inner.reset -text {Restart This Map} \
#          -command restartMap
#pack .mapcontrol.inner.newmap .mapcontrol.inner.reset -side left
#pack .mapcontrol.inner -side top
#pack .mapcontrol -side top -fill x -padx 10


global tsiConfig
if { [info exists tsiConfig(ControlPanel)] } {
    if { [string compare $tsiConfig(ControlPanel) ETControlPanel] != 0 } {
	error "The Eaters Simulator Requires the ETControlPanel."
	exit 0
    }
} else {
	error "The Eaters Simulator Requires the ETControlPanel."
	exit 0
}

# Now we make the eater specific changes to the et-controlpanel.
# In particular, we need to change all the occurances of 'Agent' with 'Eater'

.agentcreation.createAgent configure  -command \
    {startEater -1 -1 -1 $tsiCurrentAgentSourceDir $tsiCurrentAgentSourceFile $currentAgentColor ; updateAvailableAgentColors }

#.menu.agents configure -text {Eaters}
#.menu.agents.m entryconfigure {Destroy All Agents} -label {Destroy All Eaters}
#.menu.windowtoggle.m entryconfigure {Agent Info} -label {Eater Info}

#[.destroyAgent cget -menu] delete 0 end


### Now we can display the window
wm deiconify .




proc updateAvailableAgentColors {} {
   global currentAgentColor eaterList agentColorMenu possibleAgentColors

   ### Update the color choices, based on existing eater colors
   foreach color $possibleAgentColors {
      if {![info exists eaterList] || [lsearch $eaterList $color] < 0} {
         $agentColorMenu entryconfigure $color -state normal
      } else {
         $agentColorMenu entryconfigure $color -state disabled
      }
   }
   ### If the current color is disabled, find a new one.  If a new one can't
   ### be found, disable the create eater button
   if {($currentAgentColor == {}) || \
       ([$agentColorMenu entrycget $currentAgentColor -state] == {disabled})} {
      set currentAgemtColor {}
      .agentcreation.createAgent configure -state disabled
      foreach i $possibleAgentColors {
         if {[$agentColorMenu entrycget $i -state] == "normal"} {
            $agentColorMenu invoke $i
            .agentcreation.createAgent configure -state normal
            break
         }
      }
   }
}


