

proc updateAvailableAgentColors {} {
   global currentAgentColor tankList agentColorMenu possibleAgentColors

   ### Update the color choices, based on existing tank colors
   foreach color $possibleAgentColors {
      if {![info exists tankList] || [lsearch $tankList $color] < 0} {
         $agentColorMenu entryconfigure $color -state normal
      } else {
         $agentColorMenu entryconfigure $color -state disabled
      }
   }
   ### If the current color is disabled, find a new one.  If a new one can't
   ### be found, disable the create tank button
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

proc doPrintDepth {w} {
   global soarinfoname

	 	eval [list termTextSendCommand $w [list print -depth 5 $soarinfoname]]

#             eval  "termTextRewriteUserText $w; \
                       $w delete {promptEnd + 1 chars} end; \
                       [list termTextInsert $w \
                             "print -depth  $soarinfoname"]; \
                       $w mark set insert {promptEnd + 14 chars}"
}

proc soarPopUp {w {agentW 0}} {
  if {$agentW == 0} {
    set agentW $w
  }
  if [winfo exists $w] {
    bind $w <Control-ButtonPress-1> "
      setSoarStringInfo $w @%x,%y
      doPopupMenu $agentW %X %Y
      break
    "
     bind $w <Control-3> "
      setSoarStringInfo $w @%x,%y
      doPrintDepth $agentW
      break
    "
 } else {
    tk_dialog .error {Warning} \
              "Attempted to set up Soar Pop-ups in non-existent window $w" \
              warning 0 Dismiss

  }
}


button .agentcreation.human -text {Human Tank} \
          -command {createHumanTank -1 -1 $currentAgentColor; updateAvailableAgentColors}
.agentcreation.createAgent configure \
			-command {createTank -1 -1 $tsiCurrentAgentSourceDir $tsiCurrentAgentSourceFile $currentAgentColor; \
                    updateAvailableAgentColors }
#          -command {createTank .wGlobal.map $currentName $currentAgentColor; \
                    updateAvailableAgentColors}

grid .agentcreation.human -column 1 -sticky ew -padx 5

.menu.windowtoggle.m add command -label "Hide/Show Human Controls" \
   -command {toggleWindow .wControls}

#pack .agentcreation.inner -side top
#pack .agentcreation -side top -fill x -padx 10


#.menu.windowtoggle.m add command -label {Manual Controls} -command {toggleWindow .wControls}
#.menu.windowtoggle.m add command -label {Save Window Prefs} -command {saveWindowPreferences}
#.menu.windowtoggle.m add command -label {Load Window Prefs} -command {loadWindowPreferences}
#.menu.windowtoggle.m add command -label {Edit Constants Window} -command {constWindow}

#pack .menu.windowtoggle -side left -padx 5

### Now we can display the window
wm deiconify .
return
