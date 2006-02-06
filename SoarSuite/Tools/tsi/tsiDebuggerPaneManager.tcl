namespace eval PaneManager {
   variable paneList
   variable paneSizes
   variable paneContents
   variable paneObjects
   variable lastPaneNameCounter
   variable notebookFrame
   variable watchPanes
   variable stateTextWidget
   variable MatchesTextWidget
   variable OPrefsTextWidget
   variable soarConsoleText
   variable watchNames
   variable topPane
   variable layoutInitialized
}

proc PaneManager::_init { topFrame } {
   variable paneList
   variable paneSizes
   variable paneContents
   variable paneObjects
   variable lastPaneNameCounter
   variable notebookFrame
   variable topPane
   variable watchPanes
   variable watchNames
   variable stateTextWidget
   variable MatchesTextWidget
   variable OPrefsTextWidget
   variable soarConsoleText
   variable layoutInitialized

   set lastPaneNameCounter 0
   set paneList {}
   set notebookFrame $topFrame
   set topPane ""
   set watchPanes {}
   set stateTextWidget ""
   set MatchesTextWidget ""
   set OPrefsTextWidget ""
   set soarConsoleText ""
   set paneObjects {}
   ###KJC unsetIfExists PaneManager::paneContents
   ###KJC unsetIfExists PaneManager::watchNames
   
}

proc PaneManager::Reset {} {
   variable notebookFrame
   variable topPane
      
   # Cut off soar console
   # Kill the monitors (state stack, match, prefs)
   # Initialize Debugger variables if any
   # Initialize PaneManager variables
   # Reset pane id counters
   # Redo all the Debugger bindings
   # Call Destroy on all the pane items to clean up trees and such
   
   DeletePaneObjects
   destroy $topPane
   _init $notebookFrame
   set Debugger::cycleCanvas ""
   set Debugger::soarConsoleText ""
   MiscPanes::DeleteMonitors
}

proc PaneManager::DeletePaneObjects {} {
   variable paneObjects
   
   foreach p $paneObjects {
      #puts "DeletePaneObjects $p"
      switch [lindex $p 1] {
         WME {
            WMEPane::DestroyPane [lindex $p 0]
         }
         Watch {
            WatchPane::DestroyPane [lindex $p 0]
         }
      }
   }
}

proc PaneManager::Update {} {

   WMEPane::Update
   WatchPane::Update

}

proc PaneManager::DeletePaneObjects {} {
   variable paneObjects
   
   foreach p $paneObjects {
      #puts "DeletePaneObjects $p"
      switch [lindex $p 1] {
         WME {
            WMEPane::DestroyPane [lindex $p 0]
         }
         Watch {
            WatchPane::DestroyPane [lindex $p 0]
         }
      }
   }
}
proc PaneManager::LoadLayout { {whichLayout 1} } {
   variable notebookFrame
   
   switch $whichLayout {
      1 {
         #wm geom .tsw 594x273+1005+73
         set layoutList {h-w-100-m-1 Soar-w-90-m-1 Buttons-w-15-m-1}
         wm geom .tsw 825x674+7+7
      }
      2 {
         #wm geom .tsw 594x1007+1005+73
         set layoutList {h-w-45-m-1 {v-w-50-m-1 {h-w-50-m-1 StateStack-w-50-m-1 Watch-w-50-m-1} WME-w-50-m-1} Soar-w-40-m-1 Buttons-w-15-m-1}
         #set layoutList {h-w-44-m-1 {v-w-50-m-1 {h-w-50-m-1 StateStack-w-40-m-1 Watch-w-60-m-1} WME-w-50-m-1} Soar-w-49-m-1 Buttons-w-7-m-1}
         wm geom .tsw 825x674+7+7
        }
      3 {
         #wm geom .tsw 1256x1053+199+11
         set layoutList {v-w-82-m-1 {h-w-45-m-1 {v-w-50-m-1 {h-w-50-m-1 StateStack-w-40-m-1 OPrefs-w-60-m-1} WME-w-50-m-1} Soar-w-40-m-1 Buttons-w-15-m-1} {h-w-14-m-1 Watch-w-50-m-1 Watch-w-50-m-1}}
         #set layoutList {v-w-82-m-1 {h-w-49-m-1 {v-w-50-m-1 {h-w-50-m-1 StateStack-w-40-m-1 OPrefs-w-60-m-1} WME-w-50-m-1} Soar-w-49-m-1 Buttons-w-0-m-1} {h-w-14-m-1 Watch-w-50-m-1 Watch-w-50-m-1}}
         wm geom .tsw 1006x674+7+7
        }
      4 {
         #wm geom .tsw 1256x1053+199+11
         set layoutList {v-w-82-m-1 {h-w-44-m-1 {v-w-50-m-1 {h-w-50-m-1 StateStack-w-40-m-1 Matches-w-60-m-1} WME-w-50-m-1} Soar-w-40-m-1 Buttons-w-15-m-1} {h-w-14-m-1 {v-w-50-m-1 {h-w-50-m-1 Watch-w-50-m-1 Watch-w-50-m-1}} {v-w-50-m-1 {h-w-50-m-1 Watch-w-50-m-1 Watch-w-50-m-1}}}}
         #set layoutList {v-w-82-m-1 {h-w-48-m-1 {v-w-50-m-1 {h-w-50-m-1 StateStack-w-40-m-1 Matches-w-60-m-1} WME-w-50-m-1} Soar-w-49-m-1 Buttons-w-0-m-1} {h-w-14-m-1 {v-w-50-m-1 {h-w-50-m-1 Watch-w-50-m-1 Watch-w-50-m-1}} {v-w-50-m-1 {h-w-50-m-1 Watch-w-50-m-1 Watch-w-50-m-1}}}}
         wm geom .tsw 1006x674+7+7
      }
   }
   
   ProcessLayout $notebookFrame $layoutList
   FinishLayout

}

proc PaneManager::FinishLayout {} {
   variable notebookFrame
   variable stateTextWidget
   variable OPrefsTextWidget
   variable MatchesTextWidget
   variable layoutInitialized

   set layoutInitialized 1
   pack $notebookFrame
   
   # set up soarconsoletext var and do popup
   if {$stateTextWidget != ""} {
      SetupContextMenus $stateTextWidget
   }
   if {$OPrefsTextWidget != ""} {
      SetupContextMenus $OPrefsTextWidget
   }
   if {$MatchesTextWidget != ""} {
      SetupContextMenus $MatchesTextWidget
   }
   
}

proc PaneManager::CreatePanedFrame { parentFrame orientation } {
   variable paneList
   variable paneContents
   variable topPane

   #puts "CreatePanedFrame called."
   set paneID [GetNewPaneName]
   set paneWin [PanedWindow $parentFrame.$paneID -side $orientation -weights available] 

   if {$paneList == {} } {
      set topPane $paneWin
   }
   lappend paneList $paneID
   set paneContents($paneID,pw) $paneWin
   set paneContents($paneID,panes) {}
   
   return $paneID
}

proc PaneManager::AddPane {paneID type {weight 10} {minsize 300}} {
   variable paneContents
   variable watchPanes
   variable watchNames
   variable stateTextWidget
   variable MatchesTextWidget
   variable OPrefsTextWidget
   variable soarConsoleText
   variable layoutInitialized
   variable paneObjects
   
   #puts "AddPane called with $paneID $type $weight $minsize"
   set thePanedWindow $paneContents($paneID,pw)
   set newPane [$thePanedWindow add -weight $weight -minsize $minsize]
   #puts "Added $newPane"
   #puts "- to $thePanedWindow"
   
   set newPaneID [GetNewPaneName]
   lappend paneContents($paneID,panes) $newPane
   
   switch -exact -- $type {
      none {}
      Watch {
         lappend watchPanes $newPaneID
         set watchNames($newPaneID) "Watch View [llength $watchPanes]"
         # Create Watch Pane here
         WatchPane::create $newPane $newPaneID $watchNames($newPaneID)
         lappend paneObjects [list $newPaneID $type]
      }
      StateStack {
         # Check if there is already a state stack
         if {$stateTextWidget == ""} {
            set stateTextWidget [MiscPanes::CreateStateStack $newPane]
         }
      }
      OPrefs {
         # Check if there is already a state stack
         if {$OPrefsTextWidget == ""} {
            set OPrefsTextWidget [MiscPanes::CreateOPrefStack $newPane]
         }
      }
      Matches {
         # Check if there is already a state stack
         if {$MatchesTextWidget == ""} {
            set MatchesTextWidget [MiscPanes::CreateMatchesPane $newPane]
         }
      }
      WME {
         WMEPane::create $newPane $newPaneID   
         lappend paneObjects [list $newPaneID $type]
      }
      Soar {
         # Check if there is already a Soar command line window
         if {$soarConsoleText == ""} {
            set soarConsoleText [MiscPanes::SetupSoarConsole $newPane $layoutInitialized]
            set Debugger::soarConsoleText $soarConsoleText
         }
      }
      Buttons {
         # Check if there is already a button canvas
         if {$Debugger::cycleCanvas == ""} {
            set Debugger::cycleCanvas [MiscPanes::CreateExecutionFrame $newPane]
         }
      }
      
   }
   return $newPane
}


proc PaneManager::PackPanes { paneID } {
   variable paneContents

   foreach p $paneContents($paneID,panes) {
      #puts "packing $p"
      pack $p -fill both -expand yes
   }
      #puts "packing $paneContents($paneID,pw)"
   
   pack $paneContents($paneID,pw) -fill both -expand yes

}   

proc PaneManager::GetWatchPanes {} {
   variable watchPanes
   variable watchNames
   
   set returnList {}
   foreach p $watchPanes {
      lappend returnList [list $p $watchNames($p)]
   }
   return $returnList
}

proc PaneManager::GetNewPaneName {} {
   variable lastPaneNameCounter
   
   incr lastPaneNameCounter
   
   return "mpane$lastPaneNameCounter"
}

proc PaneManager::SetupContextMenus {theTextWidget} {
   variable soarConsoleText
   # Set-up right-clicking in State Stack now that soar console has been created
   soarPopUp $theTextWidget $soarConsoleText
}

proc PaneManager::ProcessLayout {theFrame layoutList} {
   variable paneContents
   
   #Improviser eval PaneManager::TD
   #puts "Layout list is $layoutList"
   set theHead [lhead $layoutList]
   set theTail [ltail $layoutList]
   
   if {$theHead == {}} {
      #puts "Head is empty.  Returning."
      return
   }
   if {[llength $theHead] > 1} {
      set theTail [concat [list [lrange $theHead 1 [llength $theHead]]] $theTail]
      set theHead [lindex $theHead 0]
   }
   #puts "--The head is $theHead\n ([llength $theHead]) The tail is $theTail"
   
   # Note:  we could always store away the pane params somewhere if need be
   array set paneParams [GetPaneParameters $theHead]
   switch -exact $paneParams(type) {
      h {
         set newPWID [PaneManager::CreatePanedFrame $theFrame left]
         foreach i $theTail {
            #puts "Horizontal Recursing for $i"
            if {[IsASplit [lindex $i 0]]} {
               unset paneParams
               array set paneParams [GetPaneParameters [lindex $i 0]]
               set newPane [AddPane $newPWID none $paneParams(w) $paneParams(m)]
               ProcessLayout $newPane $i
            } else {
               ProcessLayout $newPWID $i
            }
         }
         pack $paneContents($newPWID,pw) -fill both -expand yes
      }
      v {
         set newPWID [PaneManager::CreatePanedFrame $theFrame top]
         foreach i $theTail {
            #puts "Vertical Recursing for $i"
            #set newPane [AddPane $newPWID none]
            if {[IsASplit [lindex $i 0]]} {
               unset paneParams
               array set paneParams [GetPaneParameters [lindex $i 0]]
               set newPane [AddPane $newPWID none $paneParams(w) $paneParams(m)]
               ProcessLayout $newPane $i
            } else {
               ProcessLayout $newPWID $i
            }
         }
         pack $paneContents($newPWID,pw) -fill both -expand yes
      }
      default {
         #puts "Adding a Content Pane $paneParams(type) $theTail"
         AddPane $theFrame $paneParams(type) $paneParams(w) $paneParams(m)
      }
   }
}


proc PaneManager::GetPaneParameters {paneString} {
   
   array set returnArray {}
   set stringSplit [split $paneString "-"]
   set returnArray(type) [lindex $stringSplit 0]
   set stringSplit [lrange $stringSplit 1 [llength $stringSplit]]
   foreach {a v} $stringSplit {
      set returnArray($a) $v
   }
   #puts "Returning array [array get returnArray]"
   return [array get returnArray]
}

proc PaneManager::IsASplit { paneString } {
   return [string match "\[hv\]*" $paneString]
}