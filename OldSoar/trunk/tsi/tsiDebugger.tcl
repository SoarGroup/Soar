namespace eval Debugger {
   variable _wfont

   variable notebook
   variable mainframe
   variable status
   variable font
   variable font_name
   variable toolbar1  1
   variable toolbar2  1
   variable activeUpdate
   variable learnSettings
   variable indifferentSetting
   variable elabSetting
   variable soarVersion
   variable cycleIcons
   variable cycleCanvas
   variable lastPhase
   variable paneWin
}


proc Debugger::create { } {
   global   tcl_platform
   global   tk_patchLevel
   variable _wfont
   variable notebook
   variable mainframe
   variable font
   variable activeUpdate
   variable learnSettings
   variable indifferentSetting
   variable elabSetting
   variable soarVersion
   variable cycleIcons
   variable cycleCanvas
   variable lastPhase
   variable paneWin
   global   tsi_library tsiConfig auto_path tsi_library soar_library interp_name

   set activeUpdate 1
   set lastPhase "Last Phase: NONE"
   update

   set cycleIcons(blank) [image create photo -file [file join $tsi_library gifs c_blank.gif]]
   set cycleIcons(ip) [image create photo -file [file join $tsi_library gifs c_ip.gif]]
   set cycleIcons(p) [image create photo -file [file join $tsi_library gifs c_p.gif]]
   set cycleIcons(da) [image create photo -file [file join $tsi_library gifs c_da.gif]]
   set cycleIcons(a) [image create photo -file [file join $tsi_library gifs c_a.gif]]
   set cycleIcons(oi) [image create photo -file [file join $tsi_library gifs c_oi.gif]]


   SelectFont::loadfont

   # Menu description
   set descmenu {
     "&File" all file 0 {
         {command "E&xit" {} "Close Agent Debugger" {} -command exit}
      }
      "&Options" all options 0 {
         {checkbutton "Toolbar &1" {all option} "Show/hide Toolbar" {}
            -variable Debugger::toolbar1
            -command  {$Debugger::mainframe showtoolbar 0 $Debugger::toolbar1}
         }
         {command "&Preferences" {} "Modify TSI Preferences" {} -command makeTSIPrefsWindow}
      }
   }

   set mainframe [MainFrame .tsw.mainframe \
                  -menu       $descmenu \
                  -textvariable Debugger::status ]

   # toolbar creation
   set tb1  [$mainframe addtoolbar]
   set bbox [ButtonBox $tb1.bbox1 -spacing 0 -padx 1 -pady 1]
   $bbox add -image [Bitmap::get back] \
      -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
      -helptext "Previous Snapshot" -state disabled
   $bbox add -image [Bitmap::get snapshot] \
      -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
      -helptext "Take Snapshot"  -state disabled
   $bbox add -image [Bitmap::get forward] \
      -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
      -helptext "Next Snapshot" -state disabled
   pack $bbox -side left -anchor w

   set sep [Separator $tb1.sep -orient vertical]
   pack $sep -side left -fill y -padx 4 -anchor w

   set bbox [ButtonBox $tb1.bbox2 -spacing 0 -padx 1 -pady 1]
   $bbox add -image [Bitmap::get refresh] \
      -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
      -helptext "Refresh WME Listing" \
      -command Debugger::updateNotebooks
   pack $bbox -side left -anchor w

   set sep2 [Separator $tb1.sep2 -orient vertical]
   pack $sep2 -side left -fill y -padx 4 -anchor w

   set bbox [ButtonBox $tb1.bbox3 -spacing 0 -padx 1 -pady 1]
   $bbox add -image [Bitmap::get cut] \
      -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
      -helptext "Cut selection" -state disabled
   $bbox add -image [Bitmap::get copy] \
      -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
      -helptext "Copy selection" -state disabled
   $bbox add -image [Bitmap::get paste] \
      -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
      -helptext "Paste selection" -state disabled
   pack $bbox -side left -anchor w


   #Indicator Setup
   UpdateStats
   $mainframe addindicator -textvariable Debugger::lastPhase -padx 10 
   $mainframe addindicator -textvariable Debugger::learnSettings -padx 10
   $mainframe addindicator -textvariable Debugger::soarVersion -padx 6 
   $mainframe addindicator -text "tsi $tsiConfig(ControlPanelVersion)" -padx 6 
   # laird said get rid of this
   #$mainframe addindicator -text "Debugger v1.0b" -padx 6

   # Paned frame for Notebook on top and console on bottom
   set paneWin   [PanedWindow [$mainframe getframe].pw2 -side left]
   set pane1  [$paneWin add]
   set pane2  [$paneWin add]

   # NoteBook creation
   #set frame   [frame $pane1.nbframe]
#   set notebook [NoteBook $frame.nb]
   set notebook [NoteBook $pane1.nb]

   set f0 [Console::create $notebook]
   set f1 [WMETree::create $notebook]
   set f2 [StateTree::create $notebook]
   set f3 [MatchTree::create $notebook]
   set f4 [pacNotebook::create $notebook]
   set f5 [justificationsNotebook::create $notebook]

   #$notebook compute_size
   pack $notebook -fill both -expand yes -padx 4 -pady 4
   $notebook raise [$notebook page 0]
   $notebook itemconfigure consoleNotebook -raisecmd  {Debugger::updateRaisedNotebook}

   # Soar console widget

   set topframe [frame $pane2.topframe]
   set tsiConfig(tswFrame) $topframe

   if [catch registerWithController msg] {
      error "Failed to register new agent with controller: $msg"
   }

   frame $topframe.menubar -relief raised -bd 2
   pack $topframe.menubar -side top -fill x

   addTSIFileMenu $topframe
   addTSIShowMenu $topframe
   addTSIMemoryMenu $topframe
   addTSIProductionMenu $topframe
   addTSIWatchMenu $topframe
   addTSIViewMenu $topframe
   addTSICommandsMenu $topframe
   addTSIHelpMenu $topframe
   addTSIDemoMenu $topframe

   if $tsiConfig(debug) {puts "\n>>> starting makeSoar interaction window-term"}

   setUpSoarTerm $topframe.t
   $topframe.t configure -font $tsiConfig(normalFont)
   $topframe.t configure -yscrollcommand "$topframe.s set"
   scrollbar $topframe.s -relief flat -command "$topframe.t yview"

   pack $topframe.s -side right -fill both
   pack $topframe.t -side top -expand 1 -fill both

   output-strings-destination -push -procedure tsiOutputToWidget

   # We add a monitor to make sure that long running commands like
   # "go" do not stop X events from being processed.
   monitor -add after-decision-cycle update

   eval $topframe.t tag configure prompt $tsiConfig(promptConfigure)
   eval $topframe.t tag configure oldUserText $tsiConfig(userTextConfigure)
   eval $topframe.t tag configure searchFind $tsiConfig(searchTextConfigure)

   # Soar execution button creation

   set bottomframe  [frame $mainframe.bottomframe -height 40]
   set titf1 [TitleFrame $bottomframe.titf1 -text "Environment"]
   set titf2 [TitleFrame $bottomframe.titf2 -text "Agent Only"]
   set titf3 [TitleFrame $bottomframe.titf3 -text "Options"]

   set bframe [$titf1 getframe]
   set buttonframe [frame $bframe.butfr]
   set but1   [Button $buttonframe.but1 -text "Step" \
               -repeatdelay 300 \
               -command  {Debugger::SoarButton 0 {step}} \
               -helptext "Step Agent/Environment 1 Decision/Output Cycle"]
   set but2   [Button $buttonframe.but2 -text "Run" \
               -repeatdelay 300 \
               -command  {Debugger::SoarButton 0 {run}} \
               -helptext "Run Agent/Environment"]
   if { ![info exists tsiConfig(hasEnvironment)] || $tsiConfig(hasEnvironment) == 0 } {
     set but3   [Button $buttonframe.but3 -text "Stop" \
               -repeatdelay 300 \
               -command  {Debugger::SoarButton 0 {stop-soar}} \
               -helptext "Halt Agent/Environment"]

   } else {     
     set but3   [Button $buttonframe.but3 -text "Stop" \
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
   set but4   [Button $buttonframe2.but4 -text "Phase" \
               -repeatdelay 300 \
               -command  {Debugger::SoarButton 1 {run 1 p -self}} \
               -helptext "Run next phase for this agent.  No environment update."]
   set but5   [Button $buttonframe2.but6 -text "Full Cycle" \
               -repeatdelay 300 \
               -command  {Debugger::SoarButton 0 {run 1 d -self}} \
               -helptext "Run one decision cycle for this agent.  No environment update."]

   set bframe [$titf3 getframe]

   set chk [checkbutton $bframe.chk -text "Active Update" \
             -variable Debugger::activeUpdate \
             -command  "Debugger::ChangeUpdate"]
   pack $chk -anchor w

   canvas $bottomframe.cycle -width 158 -height 55 -relief sunken -bd 3
   $bottomframe.cycle create image 0 7 \
      -image $cycleIcons(blank) -anchor nw -tag cycleTag

   set cycleCanvas $bottomframe.cycle

   pack $but1 $but2 $but3 -side left -padx 4
#   pack $but1 $but2 $but3 $but31 -side left -padx 4
   pack $buttonframe
   pack $but4 $but5 -side left -padx 4
   pack $buttonframe2

   pack $titf1 $titf2 $titf3 -side left -fill both -padx 8 -expand no
   pack $bottomframe.cycle -side right

   pack $topframe -fill both -expand 1
   pack $bottomframe -side bottom -pady 2 -fill y -before $topframe

   pack $paneWin -fill both -expand yes

   pack $mainframe -fill both -expand yes
   pack propagate $mainframe 0
   update idletasks
   initSashPosition
}

proc Debugger::initSashPosition {} {
   variable sashPosition

   set sashPosition(consoleNotebook) 7
   set sashPosition(MatchTree) 100
   set sashPosition(wmeTree) 55
   set sashPosition(pacNotebook) 100
   set sashPosition(justificationsNotebook) 100
   set sashPosition(StateTree) 40

}

proc Debugger::SoarButton {isPhase stepCommand} {
   variable cycleCanvas
   variable lastPhase
   variable cycleIcons
   variable activeUpdate

   if {$isPhase} {
      if {$lastPhase == "Last Phase: Input"} {
         $cycleCanvas delete all
         $cycleCanvas create image 0 7 \
            -image $cycleIcons(p) -anchor nw -tag cycleTag
         set lastPhase "Last Phase: Propose"
      } elseif {$lastPhase == "Last Phase: Decision"} {
         $cycleCanvas delete all
         $cycleCanvas create image 0 7 \
            -image $cycleIcons(a) -anchor nw -tag cycleTag
         set lastPhase "Last Phase: Apply"
      }
   }

   tsiDisplayAndSendCommand $stepCommand
   if {($stepCommand != "run") && $activeUpdate} {
      updateNotebooks
   }
}

proc Debugger::update_font { newfont } {
    variable _wfont
    variable notebook
    variable font
    variable font_name

    .tsw configure -cursor watch
    if { $font != $newfont } {
        $_wfont configure -font $newfont
        $notebook configure -font $newfont
        set font $newfont
    }
    .tsw configure -cursor ""
}

proc Debugger::updateNotebooks {} {
   variable notebook
   variable paneWin
   variable sashPosition

   set currentPage [$notebook raise]
   switch -exact $currentPage {
       wmeTree {WMETree::Update}
       pacNotebook {pacNotebook::Update}
       justificationsNotebook {justificationsNotebook::Update}
       StateTree {StateTree::Update}
   }
   
   PanedWindow::setPercentage $paneWin $sashPosition($currentPage)
   UpdateStats
}

proc Debugger::updateRaisedNotebook {} {
   variable notebook
   variable activeUpdate
   variable paneWin
   variable sashPosition

   set currentPage [$notebook raise]
   if {$activeUpdate} {
      switch -exact $currentPage {
         wmeTree {WMETree::Update}
         pacNotebook {pacNotebook::Update}
         justificationsNotebook {justificationsNotebook::Update}
         StateTree {StateTree::Update}
      }
   }

   PanedWindow::setPercentage $paneWin $sashPosition($currentPage)
   UpdateStats
}

proc Debugger::ChangeCycleIcon {updateNum} {
    variable cycleCanvas
    variable lastPhase
    variable cycleIcons

    $cycleCanvas delete all
    switch -exact $updateNum {
       1 {
           $cycleCanvas create image 0 7 \
             -image $cycleIcons(ip) -anchor nw -tag cycleTag
           set lastPhase "Last Phase: Input"
         }

       2 {
           $cycleCanvas create image 0 7 \
             -image $cycleIcons(da) -anchor nw -tag cycleTag
           set lastPhase "Last Phase: Decision"
         }
       3 {
           $cycleCanvas create image 0 7 \
             -image $cycleIcons(oi) -anchor nw -tag cycleTag
           set lastPhase "Last Phase: Output"
         }
     }
}

proc Debugger::ChangeUpdate {} {
   variable activeUpdate

   if {$activeUpdate} {
       monitor -add before-input-phase {Debugger::ChangeCycleIcon 1} debugUpdate
       monitor -add before-decision-phase-cycle {Debugger::ChangeCycleIcon 2} debugUpdate
       monitor -add before-output-phase {Debugger::ChangeCycleIcon 3} debugUpdate
       updateNotebooks
       set Debugger::toolbar1 0
   } else {
      monitor -delete before-input-phase debugUpdate
      monitor -delete before-decision-phase-cycle debugUpdate
      monitor -delete before-output-phase debugUpdate
      set Debugger::toolbar1 1
   }
   $Debugger::mainframe showtoolbar 0 $Debugger::toolbar1
}

proc Debugger::UpdateStats {} {

   variable learnSettings
   variable indifferentSetting
   variable elabSetting
   variable soarVersion

   output-strings-destination -push -append-to-result

   set ls [split [tsiInternalLearn] "\n"]
   set ls2 [lrange $ls 1 [llength $ls]]
   set learnSettings "Learn:"
   foreach s $ls2 {
      set learnSettings "$learnSettings [removeFirstChar [string trim $s]] "
   }
   
   set soarVersion "Soar v[version]"

   output-strings-destination -pop
}

proc isId {symbol} {
   return [regexp "^\[A-Z\]\[0-9\]+" $symbol]
}

proc isPref {symbol} {
   return [regexp "\\++\\)" $symbol]
}

proc isClosedParen {symbol} {
   return [regexp "\\)" $symbol]
}

proc removeFirstChar {string} {
   return [string range $string 1 [string length $string]]
}

proc removeLastChar {string} {
   return [string range $string 0 [expr ([string length $string] - 2)]]
}

proc ldelete {theList itemToDelete} {
		set itemIndex [lsearch -exact $theList $itemToDelete]
		if {$itemIndex >= 0} {
		  return [lreplace $theList $itemIndex $itemIndex]
		} else {
		  return $theList
		}
}

proc lUniqueAppend {theList theItem} {

   if {[lsearch $theList $theItem] == -1} {
      return [concat $theList $theItem]
   } else {
      return $theList
   }
}

namespace eval pacNotebook {

   variable textItem
}

proc pacNotebook::create { nb } {

    set frame [$nb insert end pacNotebook -text "Chunks"]

    set topf  [frame $frame.topf]
    set titf1 [TitleFrame $frame.titf3 -text "All Chunks This Agent Has Compiled"]

    _mainframe [$titf1 getframe]

    pack $titf1 -pady 2 -padx 4 -fill both -expand yes

    $nb itemconfigure pacNotebook -raisecmd  {Debugger::updateNotebooks}
    $nb itemconfigure pacNotebook -leavecmd {set Debugger::sashPosition(pacNotebook) $PanedWindow::panePercentage}
    #puts "set Debugger::sashPosition(pacNotebook) $PanedWindow::panePercentage"

    return $frame
}


proc pacNotebook::_mainframe { parent } {
   variable textItem

    set pw1   [PanedWindow $parent.pw -side top]
    set pane  [$pw1 add]
    set sw [ScrolledWindow $pane.sw -relief sunken -borderwidth 2 -auto both]
    set tx [text $sw.text -setgrid 1 -bd 2 -font {courier 10} ]
    $sw setwidget $tx

    pack $tx -fill both -expand yes -pady 4
    pack $sw $pw1 -fill both -expand yes


    set textItem $sw.text

    Update

}

proc pacNotebook::Update {} {
   variable textItem

   $textItem configure -state normal
   $textItem delete 1.0 end

    output-strings-destination -push -append-to-result
    set userProds [print -chunk]

	 foreach p $userProds {
		 $textItem insert end [print $p]
    }

   $textItem see 1.0
   output-strings-destination -pop
   update

}

namespace eval justificationsNotebook {

   variable textItem
}

proc justificationsNotebook::create { nb } {

    set frame [$nb insert end justificationsNotebook -text "Justifications"]

    set topf  [frame $frame.topf]
    set titf1 [TitleFrame $frame.titf3 -text "All Current Justifications"]

    _mainframe [$titf1 getframe]

    pack $titf1 -pady 2 -padx 4 -fill both -expand yes

    $nb itemconfigure justificationsNotebook -raisecmd  {Debugger::updateNotebooks}
    $nb itemconfigure justificationsNotebook -leavecmd {set Debugger::sashPosition(justificationsNotebook) $PanedWindow::panePercentage}

    return $frame
}


proc justificationsNotebook::_mainframe { parent } {
   variable textItem

    set pw1   [PanedWindow $parent.pw -side top]
    set pane  [$pw1 add]
    set sw [ScrolledWindow $pane.sw -relief sunken -borderwidth 2 -auto both]
    set tx [text $sw.text -setgrid 1 -bd 2 -font {courier 10} ]
    $sw setwidget $tx

    pack $tx -fill both -expand yes -pady 4
    pack $sw $pw1 -fill both -expand yes


    set textItem $sw.text

    Update

}

proc justificationsNotebook::Update {} {
   variable textItem

   $textItem configure -state normal
   $textItem delete 1.0 end

    output-strings-destination -push -append-to-result
    set userProds [print -justifications]

	 foreach p $userProds {
		 $textItem insert end [print $p]
    }

   $textItem see 1.0
   output-strings-destination -pop
   update

}

namespace eval Console {

   variable textItem

}

proc Console::create {nb} {
   global tsiConfig auto_path tsi_library
   global soar_library interp_name

   set frame [$nb insert end consoleNotebook -text "TSI Console"]

   set topf  [frame $frame.topf]
   set tsiConfig(tswFrame) $topf

   return $frame

}

proc makeTSIPrefsWindow {} {
   if {![winfo exist prefsWindow]} {

      toplevel .prefsWindow
      set topf [frame .prefsWindow.topf]

      set sw [ScrolledWindow $topf.sw -relief sunken -borderwidth 2]
      set sf [ScrollableFrame $sw.f  -width 524 -height 600 -constrainedwidth 1]
      $sw setwidget $sf
      set subf [$sf getframe]

      fillTSIPrefsWindow $subf
    

      pack $sw -fill both -expand yes
      pack $topf -fill both -expand yes -pady 2
      wm deiconify .prefsWindow
      wm title .prefsWindow "Tcl Soar Interface Preferences"
      bind .prefsWindow <Destroy> {catch "destroyTSIPrefsWindow"}
   }
}         

proc destroyTSIPrefsWindow {} {
   global tsiConfig tsiLibrary
   
   set confirmed [MessageDlg .msgdlg -message "Save TSI configuration preferences?" -title "Please confirm" -type yesno]
   if {$confirmed == 0} {
      writeTSIPrefs
   }
   bind .prefsWindow <Destroy> ""
   destroy .prefsWindow
}

proc fillTSIPrefsWindow {base} {
   global tsiConfig tsiConfigHelp
   
   set i 0

   foreach field [array names tsiConfig] {
      if {[string match "*Font*" $field]} {
         lappend tsiConfigListFonts $field
      } elseif {[string match "*dir*" $field]} {
         lappend tsiConfigListDirs $field
      } elseif {[string match "*sio*" $field] || [string match "*socket*" $field] || [string match "*Remote*" $field]} {
         lappend tsiConfigListSIO $field
      } else {
         lappend tsiConfigListOthers $field
      }
   }
   set tsiConfigList [concat [lsort $tsiConfigListFonts] [lsort $tsiConfigListDirs] [lsort $tsiConfigListOthers] [lsort $tsiConfigListSIO]]
   foreach field $tsiConfigList {
      incr i
      frame $base.f$i
      set fieldEntry [LabelEntry $base.f$i.e$i -textvariable tsiConfig($field) -label $field -width 55 -labeljustify left -labelwidth 26 -side left -padx 4 -pady 2 -borderwidth 1]
      if {[info exists tsiConfigHelp($field)]} {
         $fieldEntry configure -helptext $tsiConfigHelp($field)
      }
      pack $fieldEntry -side left
      pack $base.f$i -side top -fill x -pady 4
   }
   return
}

proc tsiDebugger {} {

    global tcl_platform interp_name
    global auto_path tsiConfig
    #lappend auto_path ..
    package require -exact BWidget 1.2.1

    option add *TitleFrame.font {helvetica 10 bold italic}

    #wm withdraw .
    toplevel .tsw
    wm iconify .tsw
    wm withdraw .tsw
    wm title .tsw "$interp_name Agent"

    Debugger::create
    BWidget::place .tsw 0 0 center
    focus -force .tsw


    wm deiconify .tsw
    wm geom .tsw [wm geom .tsw]
    raise .tsw
    #wm geom .tsw 88x60+2+409
    #wm geom .tsw 18x24+2+409
    wm geom .tsw 90x62+2+40
    if {$tcl_platform(platform) == {unix}} {
      wm geom .tsw 110x76+2+40
    } else {
      wm geom .tsw 90x62+2+40
    }
    Debugger::ChangeUpdate
}

