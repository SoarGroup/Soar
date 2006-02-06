namespace eval Debugger {
   variable _wfont

   variable notebook
   variable mainframe
   variable status
   variable font
   variable font_name
   variable toolbar1 1
   variable activeUpdate 1
   variable learnSettings
   variable soarVersion
   variable cycleIcons
   variable cycleCanvas
   variable lastPhase
   variable soarConsoleText
   variable prgindic
   variable prgtext
   variable view0 1
   variable view1 0
   variable view2 0
   variable watchValue
}

proc Debugger::_create_intro { } {
   global tsi_library
   variable prgindic
   variable prgtext
   
   set backimg [image create photo -file [file join $tsi_library images umichai.gif]]
   set top [toplevel .intro -relief raised -borderwidth 2]

   wm withdraw $top
   wm overrideredirect $top 1

   set ximg  [label $top.x -image $backimg \
	   -foreground grey90 -background white]
   set frame [frame $ximg.f -background white]
   set prg   [ProgressBar $frame.prg -width 500 -height 30 -background white \
	   -variable Debugger::prgindic -maximum 9]
   pack $prg
   place $frame -x 10 -y 220 -anchor nw
   pack $ximg
   BWidget::place $top 0 0 center
   wm deiconify $top
}

proc Debugger::create { } {
   global   tcl_platform tk_patchLevel tsiConfig auto_path tsi_library soar_library interp_name
   variable _wfont
   variable notebook
   variable mainframe
   variable font
   variable activeUpdate
   variable learnSettings
   variable soarVersion
   variable cycleIcons
   variable cycleCanvas
   variable lastPhase
   variable soarConsoleText
   variable prgindic
   variable prgtext

   set soarConsoleText ""
   set cycleCanvas ""
   set prgtext "Please wait while loading agent..."
   set prgindic -1
   _create_intro
   
   set activeUpdate 0
   set lastPhase "Last Phase: NONE"
   #set soarVersion "Soar v[version]"
   update
  
   set cycleIcons(blank) [image create photo -file [file join $tsi_library images c_blank.gif]]
   #set cycleIcons(blank) [Bitmap::get c_blank.gif]
   set cycleIcons(ip) [image create photo -file [file join $tsi_library images c_ip.gif]]
   set cycleIcons(p) [image create photo -file [file join $tsi_library images c_p.gif]]
   set cycleIcons(da) [image create photo -file [file join $tsi_library images c_da.gif]]
   set cycleIcons(a) [image create photo -file [file join $tsi_library images c_a.gif]]
   set cycleIcons(oi) [image create photo -file [file join $tsi_library images c_oi.gif]]
   set cycleIcons(da) [image create photo -file [file join $tsi_library images c_da.gif]]
   set cycleIcons(a) [image create photo -file [file join $tsi_library images c_a.gif]]
   set cycleIcons(oi) [image create photo -file [file join $tsi_library images c_oi.gif]]

   set prgindic 0
   ###KJC tsiDetermineWatchSettings
   SelectFont::loadfont

   # Menu description
   set descmenu [CreateDescMenu]
   set mainframe [MainFrame .tsw.mainframe \
                  -menu       $descmenu \
                  -textvariable Debugger::status ]

   # toolbar creation
   incr prgindic
   CreateButtonBar $mainframe

   # Set up indicators
   incr prgindic
   ###KJC UpdateStats
   $mainframe addindicator -textvariable Debugger::lastPhase -padx 10 
   $mainframe addindicator -textvariable Debugger::learnSettings -padx 10
   $mainframe addindicator -textvariable Debugger::soarVersion -padx 6 
   $mainframe addindicator -text "tsi $tsiConfig(ControlPanelVersion)" -padx 6 

   # Paned frame for Notebook on top and console on bottom

   incr prgindic
   # NoteBook creation
   set notebook [NoteBook [$mainframe getframe].nb -arcradius 4 -tabbevelsize 6 ]
   set f0 [$notebook insert end mainPage -text "Main Page"]
   set f1 [$notebook insert end productionPage -text "Productions"]
   set f2 [$notebook insert end infoPage -text "Info and Stats"]

   incr prgindic
   PaneManager::_init $f0
   set PaneManager::layoutInitialized 0
   if {[info exists tsiConfig(defaultLayout)]} {
      PaneManager::LoadLayout $tsiConfig(defaultLayout)
   } else {
      PaneManager::LoadLayout 1
   }
   incr prgindic
   Productions::create $f1

   incr prgindic
   InfoPage::create $f2

   incr prgindic
   pack $notebook -fill both -expand yes -padx 4 -pady 4
   pack $mainframe -fill both -expand yes
   pack propagate $mainframe 0

   incr prgindic
   $notebook raise [$notebook page 0]

   $notebook itemconfigure mainPage \
        -raisecmd  { Debugger::UpdateRaisedNotebook } \
        -leavecmd { return 1 }

   $notebook itemconfigure productionPage \
        -raisecmd  { Debugger::UpdateRaisedNotebook } \
        -leavecmd { return 1 }

   $notebook itemconfigure infoPage \
        -raisecmd  { Debugger::UpdateRaisedNotebook } \
        -leavecmd { return 1 }

   incr prgindic
   DoBindings
   update idletasks
   
   destroy .intro
}

proc Debugger::LoadLayout { whichLayout } {
   variable notebook
   variable mainframe
   
   PaneManager::Reset
   PaneManager::LoadLayout $whichLayout
   pack $notebook -fill both -expand yes -padx 4 -pady 4
   pack $mainframe -fill both -expand yes
   pack propagate $mainframe 0
   DoBindings
   #UpdateRaisedNotebook
   #update idletasks
   #focus $notebook
   $notebook raise [$notebook page 1]
   $notebook raise [$notebook page 0]
}
   
proc Debugger::DoBindings {} {
   variable notebook
   
   tsiBind <Control-Key-1> [list $notebook raise [$notebook page 0]]
   tsiBind <Control-Key-2> [list $notebook raise [$notebook page 1]]
   tsiBind <Control-Key-3> [list $notebook raise [$notebook page 2]]
   tsiBind <Control-q> quitSoar
   tsiBind <Control-s> step
}
   
proc Debugger::CreateButtonBar {mainframe} {

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
      -command Debugger::UpdateRaisedNotebook
   pack $bbox -side left -anchor w

   set sep2 [Separator $tb1.sep2 -orient vertical]
   pack $sep2 -side left -fill y -padx 4 -anchor w

   set bbox [ButtonBox $tb1.bbox3 -spacing 0 -padx 1 -pady 1]
   $bbox add -image [Bitmap::get cut] \
      -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
      -helptext "Cut selection" -state normal \
      -command {Debugger::DoEditCommand Cut}
   $bbox add -image [Bitmap::get copy] \
      -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
      -helptext "Copy selection" -state normal \
      -command {Debugger::DoEditCommand Copy}
   $bbox add -image [Bitmap::get paste] \
      -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 -padx 1 -pady 1 \
      -helptext "Paste selection" -state normal \
      -command {Debugger::DoEditCommand Paste}
   pack $bbox -side left -anchor w
}

proc Debugger::DoEditCommand {whichCommand} {
   variable soarConsoleText
   
   switch $whichCommand {
      Cut { event generate $soarConsoleText <<Cut>> }
      Copy { event generate $soarConsoleText <<Copy>> }
      Paste { event generate $soarConsoleText <<Paste>> }
   }
}

proc Debugger::tsiBind {keysym command} {
   variable soarConsoleText

   bind .tsw $keysym $command
   bind $soarConsoleText $keysym $command
}
   
proc Debugger::OpenHelpFile {whichHelp} {
   global soar_doc_dir soar_library tsiConfig
   
   if [info exists soar_doc_dir] {
      set docDir $soar_doc_dir
   } else {
      set docDir [file normalize [file join $soar_library .. doc]]
   }
   switch $whichHelp {
      Changes {
         ShowFile [file join $docDir .. CHANGES]
      }
      License {
         ShowFile [file join $docDir .. LICENSE]
      }
      ManPages {
         ShowManPage [file join $docDir cat] 0
         puts "ShowManPage [file join $docDir cat] 0"
      }
   }
}

proc Debugger::CreateDescMenu {} {

   return {
     "&File" all file 0 {
         {command "S&ource file..." {} "Source agent productions" {} -command sourceSoarFile}
         {separator}
         {command "&Load Production Memory..." {} "Load Rete-Net from disk" {} -command loadReteNet}
         {command "&Save Production Memory..." {} "Write Rete-Net to disk" {} -command saveReteNet}
         {separator}
         {command "Lo&g Output to New File..." {} "" {} -command openLogFile}
         {command "Log Output to &Existing File..." {} "" {} -command appendLogFile}
         {command "Logging O&ff" {} "" {} -command {tsiDisplayAndSendCommand {log --off}}}
         {command "Log S&tatus" {} "" {} -command {tsiDisplayAndSendCommand {log --query}}}
         {separator}
         {command "E&xit" {} "Close Soar Agent Debugger" {} -command quitSoar}
      }
      "&Edit" all edit 0 {
         {command "Cut (Ctrl-&x)" {} "" {} -command {Debugger::DoEditCommand Cut}  }
         {command "Copy (Ctrl-&c)" {} "" {} -command {Debugger::DoEditCommand Copy} }
         {command "Paste (Ctrl-&v)" {} "" {} -command {Debugger::DoEditCommand Paste} }
         {separator}
         {command "&Find in Console Text" {} "" {} -command {searchWindow $tsiConfig(tswFrame)} }
         {separator}
         {command "&Preferences" {} "Modify TSI Preferences" {} -command makeTSIPrefsWindow}
      }
      "&Commands" all commands 0 {
         {command "Re&start Agent" {} "" {} -command {Debugger::ReloadAgent} }
         {command "Re&load Productions" {} "" {} -command {tsiLoadAgentSource} }
         {separator}
         {command "&Clear Working Memory" {} "" {} -command {init-soar} }
         {cascad  "&Excise Productions"  {} export 0 {
            {command "&All" {} "" {} -command {excise --all} }
            {command "&Chunks" {} "" {} -command {excise --chunks} }
            {command "&Task Productions" {} "" {} -command {excise --task} }
            {command "&User Productions" {} "" {} -command {excise --user} }
         }}
         {separator}
         {cascad  "&Indifferent Selection Mode"  {} export 0 {
            {command "&First" {} "" {} -command {} }
            {command "&Last" {} "" {} -command {} }
            {command "&Ask" {} "" {} -command {} }
            {command "&Random" {} "" {} -command {} }
         }}
      }
      "&Debug Level" all watch 0 {
         {command "Show Current &Watch Status" {} "" {} -command {tsiDisplayAndSendCommand watch} }
         {separator}
         {checkbutton "&Decisions" {all option} "" {}
            -variable Debugger::watchValue(--decisions)
            -command  {resetWatch --decisions}
         }
         {checkbutton "&Phases" {all option} "" {}
            -variable Debugger::watchValue(--phases) 
            -command  {resetWatch --phases}
         }
         {command "&All Productions" {} "" {} -command {tsiDisplayAndSendCommand {watch --productions }} }
         {command "&No Productions" {} "" {} -command {tsiDisplayAndSendCommand {watch --productions remove}} }
         {checkbutton "3a. &User Productions" {all option} "" {}
            -variable Debugger::watchValue(--user) 
            -command  {resetWatch {--user} --fullwmes --nowmes}
         }
         {checkbutton "3b. &Chunks" {all option} "" {}
            -variable Debugger::watchValue(--chunks) 
            -command  {resetWatch {--chunks} --fullwmes --nowmes}
         }
         {checkbutton "3c. &Justifications" {all option} "" {}
            -variable Debugger::watchValue(--justifications) 
            -command  {resetWatch {--justifications} --fullwmes --nowmes}
         }

         {checkbutton "4. W&MEs" {all option} "" {}
            -variable Debugger::watchValue(--wmes) 
            -command  {resetWatch --wmes}
         }
         {checkbutton "5. P&references" {all option} "" {}
            -variable Debugger::watchValue(--preferences) 
            -command  {resetWatch --preferences}
         }
         {separator}
         {command "N&othing" {} "" {} -command {set watchLevel(decisions) 0; set watchLevel(phases) 0; \
                     set watchLevel(productions) 0; set watchLevel(wmes) 0; \
                     set watchLevel(preferences) 0; \
                     tsiDisplayAndSendCommand {watch none}} }
         {command "&1 Only" {} "" {} -command {tsiDisplayAndSendCommand {watch 1}} }
         {command "1-&2 Only" {} "" {} -command {tsiDisplayAndSendCommand {watch 2}} }
         {command "1-&3 Only" {} "" {} -command {tsiDisplayAndSendCommand {watch 3}} }
         {command "1-&4 Only" {} "" {} -command {tsiDisplayAndSendCommand {watch 4}} }
         {command "1-&5" {} "" {} -command {tsiDisplayAndSendCommand {watch 5}} }
         {separator}
         {cascad  "WM&E Detail"  {} export 0 {
            {radiobutton "&None" {all option} "" {}
               -variable Debugger::watchValue(WMElevel)
               -value 1 
               -command  {tsiSetWMELevel}
            }
            {radiobutton "&Time Tags" {all option} "" {}
               -variable Debugger::watchValue(WMElevel) 
               -value 2 
               -command  {tsiSetWMELevel}
            }
            {radiobutton "&Full" {all option} "" {}
               -variable Debugger::watchValue(WMElevel) 
               -value 3 
               -command  {tsiSetWMELevel}
            }
         }}
         {separator}
         {checkbutton "&Indifferent-selection" {all option} "" {}
            -variable Debugger::watchValue(--indifferent-selection) 
            -command  {resetWatch --indifferent-selection}
         }
         {checkbutton "&Backtracing" {all option} "" {}
            -variable Debugger::watchValue(--backtracing) 
            -command  {resetWatch --backtracing}
         }
         {checkbutton "Learn Pr&int" {all option} "" {}
            -variable Debugger::watchValue(--learning) 
            -command  {resetWatch --learning print noprint}
         }
      }
      "&View" all view 0 {
         {checkbutton "&Toolbar" {all option} "Show/hide Toolbar" {}
            -variable Debugger::toolbar1
            -command  {$Debugger::mainframe showtoolbar 0 $Debugger::toolbar1}
         }
         {separator}
         {checkbutton "&Main View (Ctrl-1)" {all option} "Switch to Main Notebook" {}
            -variable Debugger::view0
            -command  {Debugger::SetNotebook 0}}
         {checkbutton "&Production View (Ctrl-2)" {all option} "Switch to Production Notebook" {}
            -variable Debugger::view1
            -command  {Debugger::SetNotebook 1}}
         {checkbutton "&Info and Stats View (Ctrl-3)" {all option} "Switch to Info and Stats Notebook" {}
            -variable Debugger::view2
            -command  {Debugger::SetNotebook 2}}
         {separator}
         {command "Layout &1 - Minimal" {} "" {} -command {Debugger::LoadLayout 1} }
         {command "Layout &2 - Simple" {} "" {} -command {Debugger::LoadLayout 2} }
         {command "Layout &3 - Default" {} "" {} -command {Debugger::LoadLayout 3} }
         {command "Layout &4 - More Watch Panes" {} "" {} -command {Debugger::LoadLayout 4} }
      }
      "De&mo" all demo 0 {
         {command "GUI Demos" {} "" {} -command {} }
         {command "&Eight Puzzle" {} "" {} -command {tsiLoadSoar8Demo eight-puzzle eight-puzzle.tcl} }
         {cascad  "&Water Jug"  {} export 0 {
            {command "&Readme" {} "" {} -command {ShowFile "[file join $soar_library .. demos water-jug readme]" 0} }
            {command "Load &Water Jug" {} "" {} -command {tsiLoadSoar8Demo water-jug water-jug.tcl} }
            {command "Load Water Jug Look-&Ahead" {} "" {} -command {tsiLoadSoar8Demo water-jug water-jug-look-ahead.tcl} }
         }}
         {separator}
         {command "Non-GUI Demos" {} "" {} -command {} }
         {cascad  "&Blocksworld"  {} export 0 {
            {command "load &Blocksworld" {} "" {} -command {tsiLoadSoar8Demo blocks-world blocks-world.soar} }
            {command "load Blocksworld &Operator Subgoaling" {} "" {} -command {tsiLoadSoar8Demo blocks-world blocks-world-operator-subgoaling.soar} }
            {command "load Blocksworld &Look-Ahead" {} "" {} -command {tsiLoadSoar8Demo blocks-world blocks-world-look-ahead.soar} }
         }}
         {cascad  "E&ight-Puzzle"  {} export 0 {
            {command "&Readme" {} "" {} -command {ShowFile "[file join $soar_library .. demos eight-puzzle readme.txt]" 0} }
            {command "Load E&ight-puzzle" {} "" {} -command {tsiLoadSoar8Demo eight-puzzle eight-puzzle.soar} }
            {command "Load &Fifteen-puzzle" {} "" {} -command {tsiLoadSoar8Demo eight-puzzle fifteen-puzzle.soar} }
         }}
         {cascad  "&Missionaries"  {} export 0 {
            {command "Load &M-a-C" {} "" {} -command {tsiLoadSoar8Demo mac mac.soar} }
            {command "Load M-a-C &Planning" {} "" {} -command {tsiLoadSoar8Demo mac mac-planning.soar} }
         }}
         {cascad  "&Towers of Hanoi"  {} export 0 {
            {command "Load &Towers of Hanoi" {} "" {} -command {tsiLoadSoar8Demo towers-of-hanoi towers-of-hanoi.soar} }
            {command "Load Towers of Hanoi &Recursive" {} "" {} -command {tsiLoadSoar8Demo towers-of-hanoi towers-of-hanoi-recursive.soar} }
         }}
         {cascad  "Water &Jug"  {} export 0 {
            {command "&Readme" {} "" {} -command {ShowFile "[file join $soar_library .. demos water-jug readme]" 0} }
            {command "Load Water &Jug" {} "" {} -command {tsiLoadSoar8Demo water-jug water-jug.soar} }
            {command "Load Water Jug &Look-Ahead" {} "" {} -command {tsiLoadSoar8Demo water-jug water-jug-look-ahead.soar} }
         }}
         {separator}
         {cascad  "&Default Rules"  {} export 0 {
            {command "&Readme" {} "" {} -command {ShowFile "[file join $soar_library .. demos default readme.txt]" 0} }
            {command "&Simple" {} "" {} -command {tsiLoadSoar8DefaultProds simple.soar} }
            {command "S&election" {} "" {} -command {tsiLoadSoar8DefaultProds selection.soar} }
         }}
      }
     "&Help" all help 0 {
         {command "&About the TSI" {} "" {} -command {showAbout}}
         {separator}
         {cascad  "&Soar"  {} export 0 {
            {command "&Man Pages" {} "" {} -command {Debugger::OpenHelpFile ManPages} }
            {command "&List All Commands" {} "" {} -command {tsiDisplayAndSendCommand {help --all}} }
            {command "&Changes" {} "" {} -command {Debugger::OpenHelpFile Changes} }
            {command "L&icense" {} "" {} -command {Debugger::OpenHelpFile License} }
            {command "&Version" {} "" {} -command {tsiDisplayAndSendCommand version} }
         }}
         {cascad  "&Tcl/Tk"  {} export 0 {
            {command "&Version" {} "" {} -command {tsiDisplayAndSendCommand {info tclversion}} }
            {command "&DLL Location" {} "" {} -command {tsiDisplayAndSendCommand {info library}} }
         }}
    }
   }
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
      UpdateRaisedNotebook
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


proc Debugger::UpdateRaisedNotebook {} {
   variable notebook
   variable activeUpdate
   variable sashPosition

   set currentPage [$notebook raise]
   if {$activeUpdate} {
      switch -exact $currentPage {
         mainPage { PaneManager::Update}
         productionPage { Productions::Update }
         infoPage { InfoPage::Update }
      }
   }
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
       UpdateRaisedNotebook
#       set Debugger::toolbar1 0
   } else {
      monitor -delete before-input-phase debugUpdate
      monitor -delete before-decision-phase-cycle debugUpdate
      monitor -delete before-output-phase debugUpdate
#      set Debugger::toolbar1 1
   }
   #$Debugger::mainframe showtoolbar 0 $Debugger::toolbar1
}

proc Debugger::UpdateStats {} {

   variable learnSettings

   ##output-strings-destination -push -append-to-result

   set ls [split [tsiInternalLearn] "\n"]
   set ls2 [lrange $ls 1 [llength $ls]]
   set learnSettings "Learn:"
   foreach s $ls2 {
      set learnSettings "$learnSettings [removeFirstChar [string trim $s]] "
   }
   
   ##output-strings-destination -pop
}

proc Debugger::SetNotebook {whichNotebook} {
   variable notebook
   variable view0
   variable view1
   variable view2

   switch $whichNotebook {
      0 {
         set view1 0
         set view2 0
         $notebook raise [$notebook page 0]
      }
      1 {
         set view0 0
         set view2 0
         $notebook raise [$notebook page 1]
      }
      2 {
         set view0 0
         set view1 0
         $notebook raise [$notebook page 2]
      }
   }
}

proc tsiDebugger {} {

    global tcl_platform interp_name
    global auto_path tsiConfig tsi_library hideAgentWindow
    
    #lappend auto_path ..
    package require -exact BWidget 1.7.0

    # Add the TSI image directory to the BWidget's image path
    #   Note:  the first line is being used to initialize the Bitmap namespace
    #          there should be a more correct way to do this, though there are no
    #          negative effects to this method.
    Bitmap::get openfold
    lappend Bitmap::path [file join $tsi_library images]

    option add *TitleFrame.font {helvetica 10 bold italic}

    toplevel .tsw
    wm protocol .tsw WM_DELETE_WINDOW { quitSoar }

    #wm iconify .tsw
    #wm withdraw .tsw
    wm title .tsw "$interp_name Agent"

    Debugger::create
    ###KJC Debugger::ChangeUpdate
    #BWidget::place .tsw 0 0 center
    if {$tsiConfig(hideAgentWindow)} {
	wm withdraw .tsw
    } else {
        focus -force .tsw
    }
    #wm geom .tsw [wm geom .tsw]
    #raise .tsw
    #wm geom .tsw 638x622+127+13
    
    set tsiAgentInfo(sourceDir) source_path
    set tsiAgentInfo(sourceFile) source_file
    #wm deiconify .tsw
}

proc Debugger::UpdateSourceReloaded {} {

   UpdateRaisedNotebook

}

proc Debugger::ReloadAgent {} {

   init-soar
   excise --all
   tsiLoadAgentSource 
}


