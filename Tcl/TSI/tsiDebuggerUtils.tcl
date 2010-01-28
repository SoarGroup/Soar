proc CreateInfoPane { pane1 title } {

   set titf       [TitleFrame $pane1.titf -text $title -side left]
   set infoTitle  [$titf getframe]

   text $infoTitle.text \
                 -font {courier 9} \
                 -setgrid false -bd 2 -wrap none\
                 -yscrollcommand [list $infoTitle.scrolly set] \
                 -xscrollcommand [list $infoTitle.scrollx set] 
   scrollbar $infoTitle.scrolly -command [list $infoTitle.text yview]
   scrollbar $infoTitle.scrollx -orient horizontal -command [list $infoTitle.text xview]

   pack $infoTitle.scrolly -side right -fill y
   pack $infoTitle.scrollx -side bottom -fill x
   pack $infoTitle.text -side left -expand true -fill both
   pack $infoTitle -fill both -expand yes
   pack $titf -side top -fill both -padx 4 -expand yes
   pack $pane1 -fill both -expand yes

   return $infoTitle.text
}

proc getFiringCount {soarString} {
   if {[string range $soarString 0 1] == "No"} {
      return ""
   } else {
      return [string trim [lindex [split $soarString ":"] 0]]
   }
}

proc getInterrupt {soarString} {
   set answer [string trim [lindex [split $soarString ":"] 1]]
   if {[string match $answer "on"]} {
      return 1
   } else {
      return 0
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

proc deleteArrayItems {theArray searchString} {
   # Deletes all items in an array whose first index matches searchstring
   # Note:  wil delete array(test,field1) but no array(test)   
   foreach i [array names $theArray $searchString,*] {
      unset [subst $theArray]($i)
   }
}

proc lhead {theList} {
   return [lindex $theList 0]
}

proc ltail {theList} {
   return [lrange $theList 1 [llength $theList]]
}

proc unsetIfExists {theVar} {
   if {[info exists $theVar]} {
      unset $theVar
   }
}

proc GetStateList {} {
   
   ##output-strings-destination -push -append-to-result
   set searchString [subst "\\{(* ^superstate *)\\}"]
   set wmeList [eval "print --depth 0 --internal $searchString"]
   ##output-strings-destination -pop
   
   set index 0
   set resultIDs {}
   foreach w $wmeList {
      if {![isPref $w]} {
         if {($index == 0) || ($index == 2)} {
            incr index
         } elseif {$index == 1} {
            lappend resultIDs $w
            incr index
         } else {
            set index 0
         }   
      }   
   }
   set resultList {}
   foreach i [lsort $resultIDs] {
      lappend resultList [list $i [getSoarName $i]]
   }
   return $resultList
}

proc GetOperatorList {} {
   
   ##output-strings-destination -push -append-to-result
   set searchString [subst "\\{(* ^operator * +)\\}"]
   set wmeList [eval "print --depth 0 --internal $searchString"]
   ##output-strings-destination -pop
   
   set index 0
   set resultIDs {}
   foreach w $wmeList {
      if {$index < 3} {
         incr index
      } elseif {$index ==3} {
         incr index
         lappend resultIDs $w
      } else {
         set index 0
      }   
   }
   set resultList {}
   foreach i [lsort $resultIDs] {
      lappend resultList [list $i [getSoarName $i]]
   }
   return $resultList
}

proc getSoarName {ID} {
   
   ##output-strings-destination -push -append-to-result
   set searchString [subst "\\{($ID ^name *)\\}"]
   set soarName [removeLastChar [lindex [eval "print --internal --depth 0 $searchString"] 3]]
   ##output-strings-destination -pop
   return $soarName
}
