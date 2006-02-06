namespace eval MiscPanes {
   variable soarConsoleText
}

proc MiscPanes::CreateStateStack { thePW} {

 # Set up Soar monitors
   set stateTextWidget [CreateInfoPane $thePW "State Stack"]
   set stackFrame [winfo parent $stateTextWidget]
   monitor -add after-decision-cycle \
                   [list ShowInMonitor $stackFrame \
                         {output-strings-destination -push -append-to-result
                          set trace [print -stack]
                          output-strings-destination -pop
                          format "%s" $trace}] debuggerPane
   monitor -add after-init-soar \
                   [list ShowInMonitor $stackFrame \
                         {output-strings-destination -push -append-to-result
                          set trace [print -stack]
                          output-strings-destination -pop
                          format "%s" $trace}] debuggerPane



   bind $stateTextWidget <Double-Button-2> [list printObject .printObjectWindow {Print Object}]
   
   ShowInMonitor $stackFrame {echo "Welcome to Soar [version]!\n\nHit Step/Run to breathe life into your agent."}
   update idletasks
   return $stateTextWidget
}
proc MiscPanes::CreateOPrefStack { thePW} {

 # Set up Soar monitors
   set OPrefsTextWidget [CreateInfoPane $thePW "Operator Preferences"]
   set stackFrame [winfo parent $OPrefsTextWidget]
   monitor -add after-decision-cycle \
                   [list ShowInMonitor $stackFrame \
                         {output-strings-destination -push -append-to-result
                          set trace [preferences]
                          output-strings-destination -pop
                          format "%s" $trace}] debuggerPane
   monitor -add after-init-soar \
                   [list ShowInMonitor $stackFrame \
                         {output-strings-destination -push -append-to-result
                          set trace [preferences]
                          output-strings-destination -pop
                          format "%s" $trace}] debuggerPane
   monitor -add after-output-phase \
                   [list ShowInMonitor $stackFrame \
                         {output-strings-destination -push -append-to-result
                          set trace [preferences]
                          output-strings-destination -pop
                          format "%s" $trace}] debuggerPane



   bind $OPrefsTextWidget <Double-Button-2> [list printObject .printObjectWindow {Print Object}]
   
   update idletasks
   return $OPrefsTextWidget
}

proc MiscPanes::CreateMatchesPane { thePW} {

 # Set up Soar monitors
   set OPrefsTextWidget [CreateInfoPane $thePW "Matches"]
   set stackFrame [winfo parent $OPrefsTextWidget]
   monitor -add after-output-phase \
                   [list ShowInMonitor $stackFrame \
                         {output-strings-destination -push -append-to-result
                          set trace [matches]
                          output-strings-destination -pop
                          format "%s" $trace}] debuggerPane
   monitor -add after-init-soar \
                   [list ShowInMonitor $stackFrame \
                         {output-strings-destination -push -append-to-result
                          set trace [matches]
                          output-strings-destination -pop
                          format "%s" $trace}] debuggerPane
   monitor -add wm-changes \
                   [list ShowInMonitor $stackFrame \
                         {output-strings-destination -push -append-to-result
                          set trace [matches]
                          output-strings-destination -pop
                          format "%s" $trace}] debuggerPane
   monitor -add after-decision-cycle \
                   [list ShowInMonitor $stackFrame \
                         {output-strings-destination -push -append-to-result
                          set trace [matches]
                          output-strings-destination -pop
                          format "%s" $trace}] debuggerPane
   monitor -add production-just-added \
                   [list ShowInMonitor $stackFrame \
                         {output-strings-destination -push -append-to-result
                          set trace [matches]
                          output-strings-destination -pop
                          format "%s" $trace}] debuggerPane



   bind $OPrefsTextWidget <Double-Button-2> [list printObject .printObjectWindow {Print Object}]
   
   update idletasks
   return $OPrefsTextWidget
}

proc MiscPanes::DeleteMonitors {} {
   #return
   monitor -delete after-output-phase debuggerPane
   monitor -delete after-init-soar debuggerPane
   monitor -delete wm-changes debuggerPane
   monitor -delete after-decision-cycle debuggerPane
   monitor -delete production-just-added debuggerPane
}

proc MiscPanes::SetupSoarConsole { pane3 {reset 0}} {
   global tsiConfig

   set topframe [frame $pane3.topframe]
   set tsiConfig(tswFrame) $topframe

   if {!$reset} {
      if {[catch registerWithController msg]} {
         error "Failed to register new agent with controller: $msg"
      }
   }

   set soarConsoleText $topframe.t
   setUpSoarTerm $topframe.t
   
   # Finish setting up Soar console
   $topframe.t configure -font $tsiConfig(normalFont)
   $topframe.t configure -yscrollcommand "$topframe.s set"
   scrollbar $topframe.s -relief flat -command "$topframe.t yview"

   pack $topframe.s -side right -fill both
   pack $topframe.t -side top -expand 1 -fill both

   ###KJC output-strings-destination -push -procedure tsiOutputToWidget

   # We add a monitor to make sure that long running commands like
   # "go" do not stop X events from being processed.
   ###KJC monitor -add after-decision-cycle update idleUpdate

   eval $topframe.t tag configure prompt $tsiConfig(promptConfigure)
   eval $topframe.t tag configure oldUserText $tsiConfig(userTextConfigure)
   eval $topframe.t tag configure searchFind $tsiConfig(searchTextConfigure)
   pack $topframe -fill both -expand 1

   return $soarConsoleText
}

proc MiscPanes::CreateExecutionFrame { bottomframe } {
global tsiConfig
   # Set up bottom Soar execution buttons
   $bottomframe configure -height 500
   
   #set bottomframe  [frame $pane6.bottomframe -height 40]
   set titf1 [TitleFrame $bottomframe.titf1 -text "Environment"]
   set titf2 [TitleFrame $bottomframe.titf2 -text "Agent Only"]

   set bframe [$titf1 getframe]
   set buttonframe [frame $bframe.butfr -height 500]
   set but1   [Button $buttonframe.but1 -image [Bitmap::get b_step] -bd 0\
               -repeatdelay 300 \
               -command  {Debugger::SoarButton 0 {step}} \
               -helptext "Step Agent/Environment 1 Decision/Output Cycle"]
   set but2   [Button $buttonframe.but2 -image [Bitmap::get b_run] -bd 0\
               -repeatdelay 300 \
               -command  {Debugger::SoarButton 0 {run}} \
               -helptext "Run Agent/Environment"]
   if { ![info exists tsiConfig(hasEnvironment)] || $tsiConfig(hasEnvironment) == 0 } {
     set but3   [Button $buttonframe.but3 -image [Bitmap::get b_stop] -bd 0 \
               -repeatdelay 300 \
               -command  {Debugger::SoarButton 0 {stop-soar}} \
               -helptext "Halt Agent/Environment"]

   } else {     
     set but3   [Button $buttonframe.but3 -image [Bitmap::get b_stop] -bd 0 \
               -repeatdelay 300 \
               -command  {Debugger::SoarButton 0 {stop}} \
               -helptext "Halt Agent/Environment"]
   }
   
# probably not a good idea to have this right next to the other buttons...
#   set but31   [Button $buttonframe.but4 -text "Init-Soar" \
#               -repeatdelay 300 \
#               -command  {Debugger::SoarButton 0 {init-soar}} \
#               -helptext "Reset Agent/Environment"]

   set bframe [$titf2 getframe]
   set buttonframe2 [frame $bframe.butfr2]
   set but4   [Button $buttonframe2.but4 -image [Bitmap::get b_phase] -bd 0 \
               -repeatdelay 300 \
               -command  {Debugger::SoarButton 1 {run 1 p -self}} \
               -helptext "Run next phase for this agent.  No environment update."]
   set but5   [Button $buttonframe2.but6 -image [Bitmap::get b_cycle] -bd 0 \
               -repeatdelay 300 \
               -command  {Debugger::SoarButton 0 {run 1 d -self}} \
               -helptext "Run one decision cycle for this agent.  No environment update."]

   canvas $bottomframe.cycle -width 158 -height 55 -relief sunken -bd 3
   $bottomframe.cycle create image 0 7 \
      -image $Debugger::cycleIcons(blank) -anchor nw -tag cycleTag


   pack $but1 $but2 $but3 -side left -padx 4
   pack $buttonframe
   pack $but4 $but5 -side left -padx 4
   pack $buttonframe2

   pack $titf1 $titf2  -side left -fill both -padx 8 -expand no
   pack $bottomframe.cycle -side right
   
   return $bottomframe.cycle

}
