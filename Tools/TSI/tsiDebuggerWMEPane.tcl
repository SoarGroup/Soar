namespace eval WMEPane {
   variable currentRoot
   variable treeName
   variable depthLevel
   variable favoriteSelected
   variable favoritesArray
   variable favoritesList
   variable favoritesCombo
   variable numberOfFavorites
   variable favoritesPane
}

proc WMEPane::DestroyPane { paneID } {
   variable currentRoot
   variable treeName
   variable depthLevel
   variable favoriteSelected
   variable favoritesArray
   variable favoritesList
   variable favoritesCombo
   variable numberOfFavorites
   variable favoritesPane
   
   WMETree::DestroyTree $treeName($paneID)
   destroy $favoritesPane($paneID)
   unset favoritesCombo($paneID)
   unset currentRoot($paneID)
   unset depthLevel($paneID)
   unset treeName($paneID)
   unset favoriteSelected($paneID)
   unset favoritesList($paneID)
   unset numberOfFavorites($paneID)
   unset favoritesPane($paneID)
   deleteArrayItems WMEPane::favoritesArray $paneID
}

proc WMEPane::debugPrint {} {
   variable currentRoot
   variable treeName
   variable depthLevel
   variable favoriteSelected
   variable favoritesArray
   variable favoritesList
   variable favoritesCombo
   variable numberOfFavorites
   variable favoritesPane

   foreach v {currentRoot treeName depthLevel favoriteSelected favoritesArray favoritesList favoritesCombo numberOfFavorites favoritesPane} {
      puts "\n$v is [array get $v]"
   }

}

proc WMEPane::create { frame paneID} {
   variable treeName
   variable depthLevel
   variable favoritesList
   variable favoriteSelected
   variable favoritesCombo
   variable favoritesPane
   
   set depthLevel($paneID) 0
   set treeName($paneID) "tree$paneID"

   SetUpFavorites $paneID
   
   set topf  [frame $frame.topf]
   set titf1 [TitleFrame $topf.titf1 -text "Working Memory Elements" -ipad 0]
   set parent [$titf1 getframe]
   set paramFrame [frame $parent.param]
   set favoritesPane($paneID) $paramFrame
   
   #set labf1 [LabelFrame $paramFrame.labf1 -text "Root Node: " -side left -anchor w \
   #           -relief sunken -borderwidth 2]

   menubutton $paramFrame.prefs -text "Edit" -menu $paramFrame.prefs.m -relief raised
   menu $paramFrame.prefs.m -tearoff 0 -relief raised
   
   $paramFrame.prefs.m add command -label {Delete This Entry} -command [list WMEPane::DeleteFavorite $paneID]
   $paramFrame.prefs.m add separator
   $paramFrame.prefs.m add command -label {Save Favorites List} -command [list WMEPane::SaveFavorites $paneID]
   $paramFrame.prefs.m add command -label {Reload Favorites List} -command [list WMEPane::ReloadFavorites $paneID]
   
   set favoritesCombo($paneID) [ComboBox $paramFrame.combo \
            -textvariable WMEPane::favoriteSelected($paneID) \
            -values $WMEPane::favoritesList($paneID) \
            -helptext "Root node from which WME tree is built.  You can edit this." \
            -editable 1 -width 30\
            -modifycmd [list WMEPane::SelectFavorite $paneID] \
            -command [list WMEPane::AddFavorite $paneID] \
            -postcommand [list WMEPane::AddDynamicFavorites $paneID]]
   pack $favoritesCombo($paneID) -anchor w -fill x

   set labf2 [LabelFrame $paramFrame.labf2 -text "Expand: " \
            -side left -anchor w -relief sunken -borderwidth 2]

   set spin  [SpinBox $labf2.spin -text "" \
            -range {0 5 1} -textvariable WMEPane::depthLevel($paneID) \
            -width 2 \
            -helptext "Open all children nodes to this depth"\
            -modifycmd [list WMEPane::ChangeDepth $paneID]]
            
   pack $spin -anchor w -fill x

   #pack $labf1 -padx 4 -anchor w -fill x -side left -expand yes
   pack $paramFrame.prefs $favoritesCombo($paneID) $labf2 -padx 4 -anchor nw -fill x -side left 
   pack $paramFrame -side top -padx 4 -pady 5
   
   #set sep3  [Separator $parent.sep3 -orient horizontal]
   #pack $sep3 -fill x -pady 10

   WMETree::create $parent $treeName($paneID)
   WMETree::init $treeName($paneID)
   WMETree::addRoot $treeName($paneID) S1
   SelectFavorite $paneID

   pack $titf1 -side left -padx 4 -pady 0 -fill both -expand yes
   pack $parent -side top -fill both -expand yes
   pack $topf -side top -fill both -expand yes
}

proc WMEPane::ChangeDepth {paneID} {
   variable depthLevel
   variable treeName
   variable currentRoot
      
   WMETree::reloadTree $treeName($paneID) [list $currentRoot($paneID)]
   WMETree::OpenAllNodesDepth $treeName($paneID) $depthLevel($paneID)
}

proc WMEPane::Update {} {
   variable treeName
   
   #TODO:  Change so PaneManager updates a specific pane
   foreach t [array names treeName] {
      WMETree::Update $treeName($t)
   }
}

proc WMEPane::SaveFavorites {paneID} {
   variable currentRoot
   variable favoriteSelected
   variable favoritesArray
   variable favoritesList
   variable numberOfFavorites
   variable favoritesPane
   global tsi_library

   #TODO: Add some error checking
   set chanID [open [file join $tsi_library "tsiFavoritesPrefs.data"] w]
   set addedCount 0
   foreach fave $favoritesList($paneID) {
      if {$addedCount < $numberOfFavorites($paneID)} {
         if {$fave != ""} {
            puts $chanID $fave
            puts $chanID "[lindex [split $fave " "] 0] $favoritesArray($paneID,$fave,depth)"
  	      }
  	   }
  	   incr addedCount
  	}
  	close $chanID
  	
   destroy .msgdlg
  	MessageDlg .msgdlg -parent $favoritesPane($paneID) \
  	   -message "Favorites saved to your preference file." \
  	   -type ok -icon info
  	   
}

proc WMEPane::ReloadFavorites {paneID} {
   variable favoritesPane

   SetUpFavorites $paneID
  	MessageDlg .msgdlg -parent $favoritesPane($paneID) \
  	   -message "Favorites reloaded from your preference file." \
  	   -type ok -icon info
}

proc WMEPane::AddFavorite {paneID} {
   variable favoriteSelected
   variable favoritesArray
   variable depthLevel
   variable favoritesList
   variable favoritesCombo
   variable numberOfFavorites

   # TODO:  Check for illegal characters like brackets.  And check for auto-items
	if {([lsearch $favoritesList($paneID) $favoriteSelected($paneID)] == -1)} {
      set favoritesList($paneID) [lrange $favoritesList($paneID) 0 [expr ($numberOfFavorites($paneID) - 1)]]
      lappend favoritesList($paneID) $favoriteSelected($paneID)
		set favoritesArray($paneID,$favoriteSelected($paneID)) [lindex [split $favoriteSelected($paneID) " "] 0]
		set favoritesArray($paneID,$favoriteSelected($paneID),depth) $depthLevel($paneID)
		$favoritesCombo($paneID) configure -values $favoritesList($paneID)
		incr numberOfFavorites($paneID)
   }
   AddDynamicFavorites $paneID
   SelectFavorite $paneID
}

proc WMEPane::DeleteFavorite {paneID} {
   variable favoriteSelected
   variable favoritesArray
   variable favoritesList
   variable numberOfFavorites
   variable favoritesPane

	if {([lsearch $favoritesList($paneID) $favoriteSelected($paneID)] != -1)} {
      set favoritesList($paneID) [lrange $favoritesList($paneID) 0 [expr ($numberOfFavorites($paneID) - 1)]]
   	if {([lsearch $favoritesList($paneID) $favoriteSelected($paneID)] != -1)} {
         set favoritesList($paneID) [ldelete $favoritesList($paneID) $favoriteSelected($paneID)]
         unset favoritesArray($paneID,$favoriteSelected($paneID))
         unset favoritesArray($paneID,$favoriteSelected($paneID),depth)
         incr numberOfFavorites($paneID) -1
         AddDynamicFavorites $paneID
         set favoriteSelected($paneID) [lindex $favoritesList($paneID) 0]
        	MessageDlg .msgdlg -parent . \
  	         -message "Favorites entry deleted." \
  	         -type ok -icon info
  	      SelectFavorite $paneID
      } else {
            destroy .msgdlg
           	MessageDlg .msgdlg -parent $favoritesPane($paneID) \
  	         -message "Cannot delete current states and operator favorites." \
  	         -type ok -icon error
      }
   } else {
      destroy .msgdlg
     	MessageDlg .msgdlg -parent $favoritesPane($paneID) \
         -message "Cannot find that entry.  Contact the Soar group immediately!" \
         -type ok -icon error
   }
}

proc WMEPane::SelectFavorite {paneID} {
   variable currentRoot
   variable favoriteSelected
   variable favoritesArray
   variable depthLevel
   variable favoritesList
   
   if {[string range $favoriteSelected($paneID) 0 0] == "-"} {
         set favoriteSelected($paneID) [lindex $favoritesList($paneID) 0]
   }
   set currentRoot($paneID) $favoritesArray($paneID,$favoriteSelected($paneID))
   set depthLevel($paneID) $favoritesArray($paneID,$favoriteSelected($paneID),depth)
   ChangeDepth $paneID
}

proc WMEPane::SetUpFavorites {paneID} {
   variable favoritesArray
   variable favoritesList
   variable favoriteSelected
   variable numberOfFavorites
   variable favoritesPane
   global tsi_library
   
   set favoritesList($paneID) {}
   deleteArrayItems WMEPane::favoritesArray $paneID
   set numberOfFavorites($paneID) 0

   if {[file exists [file join $tsi_library "tsiFavoritesPrefs.data"]]} {
      set chanID [open [file join $tsi_library "tsiFavoritesPrefs.data"] r]
      set defaultCommands {}
      while {![eof $chanID]} {
		   if {(![catch "gets $chanID newCommand"]) && \
             (![catch "gets $chanID newCommandParams"])} {
            if {$newCommand != ""} {
               if {([lsearch $favoritesList($paneID) $newCommand] == -1)} {
    	            lappend favoritesList($paneID) $newCommand
    	            incr numberOfFavorites($paneID)
               }
		         set newCommandParamList [split $newCommandParams " "]
		         set favoritesArray($paneID,$newCommand) [lindex $newCommandParamList 0]
		         set favoritesArray($paneID,$newCommand,depth) [lindex $newCommandParamList 1]
		      }
		   } else {
           	MessageDlg .msgdlg -parent $favoritesPane($paneID) \
  	            -message "Favorites file [file join $tsi_library "tsiFavoritesPrefs.data"] is corrupt!" \
  	            -type ok -icon error
         }
   	}
    close $chanID
	}
   set favoriteSelected($paneID) [lindex $favoritesList($paneID) 0]
}

proc WMEPane::AddDynamicFavorites {paneID} {
   variable favoritesArray
   variable favoritesList
   variable numberOfFavorites
   variable favoritesCombo
   
   
   set favoritesList($paneID) [lrange $favoritesList($paneID) 0 [expr ($numberOfFavorites($paneID) - 1)]]
   lappend favoritesList($paneID) "--- CURRENT STATES ---"
   foreach s [GetStateList] {
      set stateID [lindex $s 0]
      set stateName [lindex $s 1]
      if {$stateName != ""} {
         set newCommand "$stateID ($stateName)"
      } else {
         set newCommand "$stateID (no name attribute)"
      }
      lappend favoritesList($paneID) $newCommand
      set favoritesArray($paneID,$newCommand) $stateID
      set favoritesArray($paneID,$newCommand,depth) 1
   }      
   lappend favoritesList($paneID) "--- CURRENT OPERATORS ---"
   foreach s [GetOperatorList] {
      set stateID [lindex $s 0]
      set stateName [lindex $s 1]
      if {$stateName != ""} {
         set newCommand "$stateID ($stateName)"
      } else {
         set newCommand "$stateID (no name attribute)"
      }
      lappend favoritesList($paneID) $newCommand
      set favoritesArray($paneID,$newCommand) $stateID
      set favoritesArray($paneID,$newCommand,depth) 1
   }      
   $favoritesCombo($paneID) configure -values $favoritesList($paneID)
   
}   
