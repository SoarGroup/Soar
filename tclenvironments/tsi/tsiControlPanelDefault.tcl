##########################################################################
# File         : tsiControlPanel.tcl
# Author       : Gordon D. Baxter
# Date	       : 01/07/96
# Description  : 
#
#
# This version of the control panel was primarily written by Randy Jones.
# Socket support was added by Scott Wallace at version 3.0.
# Configuration was generalized to support external environments
#    by Mazin Assanie at version 2.6.
#
##########################################################################

###
###	I.	Make the TSI Control Panel
###

source [file join $tsi_library tsiControlPanelCommon.tcl]

proc makeTSIDefaultControlPanel { {hide 0}  {x -20} {y 1} } {
  global tsi_library tsiConfig tcl_platform agentCount

    set tsiConfig(ControlPanel) TSIDefaultControlPanel
    set agentCount 0
 
  . configure -relief ridge -borderwidth 5
  wm title . {Agent Control Panel}
  if {$x >= 0} { set x +$x }
  if {$y >= 0} { set y +$y }
  wm geometry . $x$y

  # create the menu bar
  frame .menu -relief raised -border 2
  pack .menu -side top -fill x
  menubutton .menu.file -text {File} -menu .menu.file.m
  menu .menu.file.m 
  .menu.file.m add command -label Exit -command quitSoar
  pack .menu.file -side left -padx 5
  menubutton .menu.agents -text {Agents} -menu .menu.agents.m
  menu .menu.agents.m
  .menu.agents.m add checkbutton -label {Stop After Decision Phase} \
           -variable tsiConfig(afterDecision) \
           -command "if \$tsiConfig(afterDecision) { \
                        stopAfterDecision ; \
                        set tsiConfig(runTilOutputGen) 0; \
                        set soarTimeUnit d; \
                     } else { \
                        dontStopAfterDecision \
                     };"
  .menu.agents.m add checkbutton -label {Run til Output Generated} \
           -variable tsiConfig(runTilOutputGen) \
           -command "if \$tsiConfig(runTilOutputGen) { \
                        set tsiConfig(afterDecision) 0 ; \
                        dontStopAfterDecision ; \
                        set soarTimeUnit out ; \
                     } else { \
                        set soarTimeUnit d \
                     };"
  pack .menu.agents -side left -padx 5
  

  # create a frame for the general buttons

  frame .run -borderwidth 2
  if {![info exists $tsiConfig(hasEnvironment)] \
	    || $tsiConfig(hasEnvironment) == 0} {
	button .run.step -text Step -command tsiStep-soar
	button .run.run -text Run -command tsiRun-soar
	button .run.stop -text Stop -command tsiStop-soar
	button .run.quit -text Quit -command quitSoar
  } else {
	button .run.step -text Step -command environmentStep
	button .run.run -text Run -command environmentRun 
	button .run.stop -text Stop -command environmentStop
	button .run.quit -text Quit -command quitSoar
  }

  # pack the frame, and its components
  pack .run.step .run.run .run.stop .run.quit -side top -anchor n -fill both
  pack .run -side right -fill y


  # make a frame for creating new agents
  frame .agents
  frame .agents.create  -borderwidth 2
  label .agents.create.name -text {Name:}

  entry .agents.create.agentNameEntry -width 20 \
       -relief sunken -textvariable newAgentName
  button .agents.create.createAgentButton \
       -text {Create New Agent} -relief raised \
       -command {createNewAgent $newAgentName; set newAgentName {} }
   
  pack .agents.create.name .agents.create.agentNameEntry \
        .agents.create.createAgentButton -side left
  pack .agents.create -side top -fill x

  ## no click to focus
  bind . <Enter>  { focus .agents.create.agentNameEntry }
  bind . <Return> { .agents.create.createAgentButton invoke }


  # The rest of the window will be filled with agent frames as they
  # are registered with the control panel.
  # There will be a permanent frame at the top for commands to be
  # issued to all agents at the same time

  frame .agents.agents -relief ridge -borderwidth 5
  pack .agents.agents -side top -fill x

  label .agents.agents.label -text {Agent Menus:}
  menubutton .agents.agents.all -text {All Agents} -relief raised \
             -menu .agents.agents.all.m

  menu .agents.agents.all.m
  .agents.agents.all.m add command -label {Open All Agent Windows} \
       -command {foreach a [tsiListAgents] { \
                    set showAgent($a) 1 ;\
                    tsiSendAgent $a {wm deiconify .tsw} \
                 }}
  .agents.agents.all.m add command -label {Close All Agent Windows} \
       -command {foreach a [tsiListAgents] { \
                    set showAgent($a) 0 ;\
                    tsiSendAgent $a {wm withdraw .tsw} \
                 }}
  .agents.agents.all.m add command -label {Watch 0} \
       -command {foreach a [tsiListAgents] { \
                    tsiSendAgent $a {tsiDisplayAndSendCommand {watch 0}} \
                 }}
  .agents.agents.all.m add separator
  .agents.agents.all.m add command -label {Clear Working Memory} \
         -command {sendAllAgents init-soar}
  .agents.agents.all.m add command -label {Excise All} \
         -command {sendAllAgents {excise -all}}
  .agents.agents.all.m add command -label {Excise Chunks} \
         -command {sendAllAgents {excise -chunks}}
  pack .agents.agents.label .agents.agents.all -side left -fill y
  pack .agents -side top -padx 10 -pady 10

  if $hide {
    #createNewAgent soar
    wm withdraw .
    if {$tcl_platform(platform) == {macintosh}} {console hide}
  } else {
    wm deiconify .
  }
  
  #createNewAgent ""

}   ;### end of proc makeTSIDefaultControlPanel


proc registerAgent {name} {
  global showAgent tsiConfig sio_agentStatus tsiAgentInfo

   ##puts "registerAgent   (begin) -- name: $name"
   
  menubutton .agents.agents.agent$name -text $name -relief raised \
              -menu .agents.agents.agent$name.m

  menu .agents.agents.agent$name.m

  # if this is a ClientTSI interface, we add an additional checkbutton
  # to each agent menu.
    if { ![string compare $tsiConfig(mode) client] } {
	.agents.agents.agent$name.m add checkbutton -label {Online} \
	   -variable sio_agentStatus($name,online) \
	   -command "agentToggleCommand $name \$sio_agentStatus($name,online) \
                       {SetClientOnline 1} {SetClientOnline 0}"
    }
	
  .agents.agents.agent$name.m add checkbutton -label {Show Agent Window} \
       -variable showAgent($name) \
       -command "agentToggleCommand $name \$showAgent($name) \
                   {wm deiconify .tsw} {wm withdraw .tsw}"
  set showAgent($name) 1
  .agents.agents.agent$name.m add command -label {Raise Agent Window} \
       -command "tsiSendAgent $name {raise .tsw}"
  .agents.agents.agent$name.m add command -label {watch 0} \
      -command "tsiSendAgent $name {tsiDisplayAndSendCommand {watch 0}}"
  .agents.agents.agent$name.m add separator
  .agents.agents.agent$name.m add command -label {Clear Working Memory} \
         -command "tsiSendAgent $name {tsiDisplayAndSendCommand init-soar}"
  .agents.agents.agent$name.m add command -label {Excise All} \
         -command "tsiSendAgent $name {tsiDisplayAndSendCommand {excise -all}}"
  .agents.agents.agent$name.m add command -label {Excise Chunks} \
         -command "tsiSendAgent $name {tsiDisplayAndSendCommand {excise -chunks}}"
  .agents.agents.agent$name.m add separator
  .agents.agents.agent$name.m add command -label {Destroy Agent} \
      -command "destroyAgent $name"

  pack .agents.agents.agent$name -side left -fill y

   set tsiAgentInfo($name) local
   set tsiAgentInfo($name,addFN) localAgentAddWME
   set tsiAgentInfo($name,rmFN) localAgentRemoveWME
}

proc destroyAgent {name} {
  global localAgentPtrs

   if {[lsearch [tsiListAgents] $name] >= 0} {
      #bind .tsw <Destroy> ""
      #destroy .tsw
      if [catch "destroy-interpreter $name"] {
         if [catch "interp delete $name"] {
            error "Cannot delete agent '$name'!"
         }
      }
       
      ##puts "this is the proc that destroys agents"

      ## this code isn't invoked by Eaters, so is untested
      ## but something like this is required for SML...
      if (0) {
      #delete the agent object 
      set result [$name eval [list soar_kernel DestroyAgent $localAgentPtrs($name)]];
      # Give Tcl object ownership of underlying C++ object so when we 
      # delete the Tcl object, they both get deleted
      set result [$name eval [list soar_agent -acquire]]
      set result [$name eval [list soar_agent -delete]]
      #don't leave bad pointers around
      if [$name eval [list info exists soar_agent]]  {
	  [$name eval [list unset soar_agent]]
      }
      unset localAgentPtrs($name)
      }
      
  }
   if [winfo exists .agents.agents.agent$name] {
      destroy .agents.agents.agent$name
   }
}
