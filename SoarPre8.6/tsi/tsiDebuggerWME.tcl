namespace eval WMETree {
    variable count
    variable dblclick
    variable currentRoot
    variable tree
    variable openNodes
    variable  closedList
    variable  wmeArray
    variable  count
    variable rootEntry
    variable depthLevel
    variable favoriteSelected
    variable favoritesArray
    variable favoritesList
    variable favoritesCombo
}

proc WMETree::ChangeRoot {} {
   reloadTree
}
proc WMETree::ChangeDepth {} {
   variable depthLevel
   
   OpenDepth home $depthLevel
}

proc WMETree::ChangeFavorite {} {
variable currentRoot
variable favoriteSelected
variable favoritesArray
variable depthLevel
variable favoritesCombo
variable favoritesList
global tsi_library

	if {($favoriteSelected != "") && (([lsearch $favoritesList $favoriteSelected] == -1) || \
	      ($favoritesArray($favoriteSelected) != $currentRoot) || \
	      ($favoritesArray($favoriteSelected,depth) != $depthLevel))} {
   	if {[file exists [file join $tsi_library "tsiFavoritesPrefs.data"]]} {
         set chanID [open [file join $tsi_library "tsiFavoritesPrefs.data"] a]
		   puts $chanID $favoriteSelected
		   puts $chanID "$currentRoot $depthLevel"
   	   close $chanID
    	}
      if {([lsearch $favoritesList $favoriteSelected] == -1)} {
      	lappend favoritesList $favoriteSelected
      }
      set favoritesArray($favoriteSelected) $currentRoot
      set favoritesArray($favoriteSelected,depth) $depthLevel
      $favoritesCombo configure -values $favoritesList
   }
}

proc WMETree::SelectFavorite {} {
variable currentRoot
variable favoriteSelected
variable favoritesArray
variable depthLevel
variable favoritesList

	if {([lsearch $favoritesList $favoriteSelected] != -1)} {
      set currentRoot $favoritesArray($favoriteSelected)
      reloadTree
      OpenDepth home $favoritesArray($favoriteSelected,depth)
      set depthLevel $favoritesArray($favoriteSelected,depth)
   }
}

proc WMETree::SetUpFavorites {} {
variable favoritesArray
variable favoritesList
variable favoriteSelected
global tsi_library
   
   set favoritesList {}
   catch "unset favoritesArray"
     if {[file exists [file join $tsi_library "tsiFavoritesPrefs.data"]]} {
         set chanID [open [file join $tsi_library "tsiFavoritesPrefs.data"] r]
         set defaultCommands {}
         while {![eof $chanID]} {
		   if {(![catch "gets $chanID newCommand"]) && \
		       (![catch "gets $chanID newCommandParams"])} {
            if {$newCommand != ""} {
               if {([lsearch $favoritesList $newCommand] == -1)} {
      	         lappend favoritesList $newCommand
               }
		         set newCommandParamList [split $newCommandParams " "]
		         set favoritesArray($newCommand) [lindex $newCommandParamList 0]
		         set favoritesArray($newCommand,depth) [lindex $newCommandParamList 1]
		      }
		   } else {
   	   	tk_dialog .error {Error} "Favorites file [file join $tsi_library "tsiFavoritesPrefs.data"] is corrupt!" info 0 Ok
         }
    	}
      close $chanID
	}
   set favoriteSelected [lindex $favoritesList 0]
   set lastFavoriteSelected $favoriteSelected
}
   
proc WMETree::create { nb } {
variable tree
variable depthLevel
variable favoritesList
variable favoriteSelected
variable rootEntry
variable favoritesCombo
    
    set depthLevel 0
    
    set frame [$nb insert end wmeTree -text "Working Memory Elements"]
    
    set topf  [frame $frame.topf]
    set titf1 [TitleFrame $topf.titf1 -text "Working Memory Elements" -ipad 0]
    
    
    set parent [$titf1 getframe]
    set paramFrame [frame $parent.param]
    set ent   [LabelEntry $paramFrame.ent -label "Rootnode ID:" -labelwidth 12 -labelanchor w \
                   -textvariable WMETree::currentRoot \
                   -width 4 \
                   -helptext "Soar ID of Root of Display Tree" \
                   -command {WMETree::ChangeRoot}]
    set rootEntry $ent

    set spin  [SpinBox $paramFrame.spin -label "Auto-expand tree to depth:" -underline 0 \
                   -labelwidth 24 -labelanchor w \
                   -range {1 10 1} -textvariable WMETree::depthLevel \
                   -width 2 \
                   -helptext "Open all children nodes to this depth"\
                   -modifycmd {WMETree::ChangeDepth}]
                   
    SetUpFavorites
    set favoritesCombo [ComboBox $paramFrame.combo -underline 0 \
                   -textvariable WMETree::favoriteSelected \
                   -label "Favorite Display Specs:" -labelwidth 21 \
                   -values $favoritesList \
                   -helptext "Favorite View Layouts" \
                   -width 30 -editable 1\
                   -command {WMETree::ChangeFavorite} \
                   -modifycmd {WMETree::SelectFavorite}]

    pack $ent $spin $favoritesCombo -padx 4 -side left 
    pack $paramFrame -side top -padx 4
    
    set sep3  [Separator $parent.sep3 -orient horizontal]
    pack $sep3 -fill x -pady 10

    set sw    [ScrolledWindow $parent.sw \
                  -relief sunken -borderwidth 2]
    set tree  [Tree $sw.tree \
                   -relief flat -borderwidth 0 -width 15 -highlightthickness 0\
		   -redraw 0 -opencmd   "WMETree::moddir 1 $sw.tree" \
                   -closecmd  "WMETree::moddir 0 $sw.tree"]
    $sw setwidget $tree
    pack $sw -side bottom -expand yes -fill both

#    pack $titf1 $titf3 -padx 4 -side top -fill both -pady 0 -expand yes
    pack $titf1 -side left -padx 4 -pady 0 -fill both -expand yes
    pack $parent -side top
    pack $topf -side top -fill both -expand yes
    

    $tree bindText  <ButtonPress-1>        "WMETree::select tree 1 $tree"
    $tree bindText  <Double-ButtonPress-1> "WMETree::select tree 2 $tree"

    $nb itemconfigure wmeTree \
        -createcmd "WMETree::init $tree" \
         -raisecmd  {
            Debugger::updateRaisedNotebook
        } \
        -leavecmd {
            set Debugger::sashPosition(wmeTree) $PanedWindow::panePercentage
            return 1
        }
        
   return $frame
}

proc moveSelection {tree amt} {
	set node [$tree selection get]
	set ind [$tree index $node]
	set par [$tree parent $node]
	set allsibs [$tree nodes $par]
	if {$amt == "parent"} {
   	# Immediate move to parent node
   	#puts "Immediate move to parent."
   	set newnode $par
	}
   if {($ind == 0) && ($amt <0)} {
      # Moving from children to parent
      #puts "Moving from children to parent"
	   set newnode $par
   }
   if {($amt > 0) && [$tree itemcget $node -open] && ([llength [$tree nodes $node]] > 0)} {
	   # Move to top of this node's children
	   #puts "Move to top of this node's children"
	   set allsibs [$tree nodes $node]
      set newnode [lindex $allsibs 0]
   }
   if {![info exists newnode]} {
  	   # Find next node from siblings
  	   set newnode [lindex $allsibs [expr $ind + $amt]]

  	   if {$newnode == ""} {
  	      #puts "New node does not exist."
     	   # If it doesn't exist find appropriate node
         if {$amt > 0} {
            #puts "Finding appropriate node"
            # If no appropriate, we'll save this as
           	set defnewnode [lindex $allsibs [expr [llength $allsibs] - 1]]
            while {($par != "root") && ($newnode == "")} {
              	#puts "...trying siblings of $par"
              	set ind [$tree index $par]
              	set par [$tree parent $par]
     	         set allsibs [$tree nodes $par]
  	            set newnode [lindex $allsibs [expr $ind + 1]]
  	            #puts "...newnode returns $newnode"
  	         }
  	         if {$newnode == ""} {
  	            #puts "Setting default node"
  	            set newnode $defnewnode
  	         }
         } else {
            #puts "Setting to lead child."
        	   set newnode [lindex $allsibs 0]
         }
      } else {
         # If moving up to an open node set to last of children
  	      set allsibs [$tree nodes $newnode]
  	      if {($amt < 0) && ([llength $allsibs] > 0) && [$tree itemcget $newnode -open]} {
            #puts "Moving up to an open node.  Setting to last of children"
     	      #puts "open: [$tree itemcget $newnode -open]"
     	      set allsibs [$tree nodes $newnode]
     	      #puts "allsibs $allsibs"
        	   set newnode [lindex $allsibs [expr [llength $allsibs] - 1]]
         } else {
            #puts "Standard move."
         }
      }
   }
	$tree selection set $newnode
	$tree see $newnode
	FileTree::select_node $tree $list $list2 $newnode
}

proc WMETree::GetOpenNodes {node} {
   variable tree 
   variable openNodes
   
   #puts "GetOpenNodes called with $node"
   set children [$tree nodes $node]
   
   if {[llength $children] > 0} {
      #puts "Node [$tree itemcget $node -text] has been open"
      lappend openNodes $node
      foreach c $children {
         GetOpenNodes $c
      }
   }
}

proc WMETree::CreateSubtreeNodesForChildren {tree node ID} {
   
   variable wmeArray
   variable closedList
   variable count

   #puts "CreateSubtreeNodesForChildren called with $tree $node $ID [lsearch $closedList $ID]"

   set childrenIDs {}
   set children [$tree nodes $node]
   #puts "Node $node has children $children"
   foreach c $children {
      set ttag [$tree itemcget $c -timeTag]
      lappend childrenIDs $ttag
      set childrenNodes($ttag) $c
   }

   set avAdd {}
   set childAdd {}
   set sortList {}
      
   foreach item $wmeArray($ID,avList) {
      if {[lsearch $childrenIDs $item] == -1} {
         lappend avAdd $item
      }
   }
   foreach item $wmeArray($ID,childList) {
      if {[lsearch $childrenIDs $item] == -1} {
         lappend childAdd $item
      }
   }
   set fullList [concat $wmeArray($ID,childList) $wmeArray($ID,avList)]
   foreach item $childrenIDs {
      if {[lsearch $fullList $item] == -1} {
         $tree delete $childrenNodes($item)
      } else {
         lappend sortList $childrenNodes($item)
      }
   }
   foreach t $avAdd {
      set a [lindex $wmeArray($t) 0]
      set v [lindex $wmeArray($t) 1]
      $tree insert end $node n:$count \
         -text      "$a = $v" \
         -image     [Bitmap::get element] \
         -drawcross never \
         -fill blue  \
         -font {courier 8} \
         -data      value \
         -timeTag $t

      lappend sortList n:$count
      incr count
   }

   foreach t $childAdd {
      set a [lindex $wmeArray($t) 0]
      set v [lindex $wmeArray($t) 1]
      if {([lsearch $wmeArray($ID,repeatList) $v]!= -1)} {
          set wmeArray($v,avList) {}
          set wmeArray($v,childList) {}
          set wmeArray($v,repeatList) {}
          #puts "new lists set up"
         $tree insert end $node n:$count \
             -text      "$a -=> $v" \
             -image     [Bitmap::get closednode] \
             -drawcross allways \
             -fill red \
             -font {courier 8 bold} \
             -data      $v \
             -timeTag $t
      } else {
          set wmeArray($v,avList) {}
          set wmeArray($v,childList) {}
          set wmeArray($v,repeatList) {}
         $tree insert end $node n:$count \
             -text      "$a -=> $v" \
             -image     [Bitmap::get closednode] \
             -drawcross allways \
             -fill black \
             -font {courier 8 bold} \
             -data      $v \
             -timeTag $t
      }
      lappend sortList n:$count
      incr count
   }
   
   $tree itemconfigure $node -drawcross auto -data $ID

   set sortList [lsort -command "compareNodes $tree" $sortList]
   $tree reorder $node $sortList
}

proc compareNodes {tree node1 node2} {
   
   return [string compare [$tree itemcget $node1 -text] [$tree itemcget $node2 -text]]

}
proc WMETree::UpdateWmeIfChanged {timeTag ID att value} {
variable closedList
variable wmeArray

   if {[isId $value]} {
      if {[lsearch $closedList $att] == -1} {
         if {[lsearch $wmeArray($ID,childList) $timeTag] != -1} {
         } else {
            lappend wmeArray($ID,childList) $timeTag
            set wmeArray($timeTag) [list $att $value]
         }
      } else {
         if {[lsearch $wmeArray($ID,avList) $timeTag] != -1} {
         } else {
            lappend wmeArray($ID,avList) $timeTag
            set wmeArray($timeTag) [list $att $value]
         }
      }
   } else {
      if {[lsearch $wmeArray($ID,avList) $timeTag] != -1} {
      } else {
         lappend wmeArray($ID,avList) $timeTag
         set wmeArray($timeTag) [list $att $value]
      }
   }
}

proc WMETree::UpdateFromSoarMemory {tree node ID} {
variable closedList
variable wmeArray
   
   output-strings-destination -push -append-to-result
   set searchString [subst "\\{($ID ^* *)\\}"]
   set wmeList [eval "print -depth 0 -internal $searchString"]
   output-strings-destination -pop
   
   if [info exists wmeArray($ID,avList)] {
      set wmeArray($ID,avList) {}
   }
   if [info exists wmeArray($ID,childList)] {
      set wmeArray($ID,childList) {}
   }
   if [info exists wmeArray($ID,repeatList)] {
      set wmeArray($ID,repeatList) {}
   }

   set index 0

   foreach w $wmeList {
      if {![isPref $w]} {
         if {$index == 0} {
             set timeTag [removeFirstChar $w]
             incr index
         } elseif {$index == 1} {
             set object $w
             incr index
         } elseif {$index ==2} {
             set attr $w
             incr index
         } else {
             if {[isClosedParen $w]} {
                set w [removeLastChar $w]
                if {[lsearch $closedList $w]!=-1} {
                   if {![info exists wmeArray($ID,repeatList)]} {
                      set wmeArray($ID,repeatList) [list $w]
                   } else {
                      set wmeArray($ID,repeatList) [lUniqueAppend $wmeArray($ID,repeatList) $w]
                   }
                }
                UpdateWmeIfChanged $timeTag $object $attr $w
             }
             set index 0
         }   
      }   
   }
   
   CreateSubtreeNodesForChildren $tree $node $ID
   set closedList [lUniqueAppend $closedList $ID]
}

proc WMETree::Update {} {
   variable tree
   variable openNodes
   
   set openNodes [list home]
   
   GetOpenNodes home
   foreach n $openNodes {
      UpdateFromSoarMemory $tree $n [$tree itemcget $n -data]
   }
}

proc WMETree::OpenDepth {node depth} {
   variable tree
   variable openNodes
   
   set childNodes [$tree nodes $node]

   foreach n $childNodes {
      set nodeData [$tree itemcget $n -data]
      if {$nodeData != "value"} {
         UpdateFromSoarMemory $tree $n $nodeData
         if {$depth} {
            OpenDepth $n [expr $depth - 1]
         }
      }
   }

   $tree itemconfigure $node -drawcross auto -open 1
}

proc WMETree::init { tree args } {
    global   tcl_platform
    variable count
    variable currentRoot
    variable closedList
    variable count
    variable wmeArray
   
    set currentRoot "S1"

    set closedList [list]
    set count 0
    $tree insert end root home -text "$currentRoot" -data "$currentRoot" -open 1 \
       -font {courier 10 bold} -image [Bitmap::get opennode] -drawcross auto
   
    $tree configure -deltax 10


    set closedList [list]
    set wmeArray($currentRoot,avList) {}
    set wmeArray($currentRoot,childList) {}
    set wmeArray($currentRoot,repeatList) {}
    UpdateFromSoarMemory $tree home $currentRoot
    
    WMETree::select tree 1 $tree home
    $tree configure -redraw 1

}

proc WMETree::reloadTree {} {
variable currentRoot
variable tree
variable closedList
variable wmeArray

   $tree delete [$tree nodes root]
   unset wmeArray
   set closedList [list]
   set count 0
   $tree insert end root home -text "$currentRoot" -data "$currentRoot" -open 1 \
      -font {courier 10 bold} -image [Bitmap::get opennode] -drawcross auto
   
   $tree configure -deltax 10


   set closedList [list]
   set wmeArray($currentRoot,avList) {}
   set wmeArray($currentRoot,childList) {}
   set wmeArray($currentRoot,repeatList) {}

   UpdateFromSoarMemory $tree home $currentRoot
   
   WMETree::select tree 1 $tree home
 
}

proc WMETree::moddir { idx tree node } {
variable closedList

   if { $idx && [$tree itemcget $node -drawcross] == "allways" } {
      set ID [$tree itemcget $node -data]
      UpdateFromSoarMemory $tree $node $ID
      set closedList [lUniqueAppend $closedList $ID]
      if { [llength [$tree nodes $node]] } {
         $tree itemconfigure $node -image [Bitmap::get opennode]
      } else {
         $tree itemconfigure $node -image [Bitmap::get closednode]
      }
   } else {
      $tree itemconfigure $node -image [Bitmap::get [lindex {closednode opennode} $idx]]
      set ID [$tree itemcget $node -data]
   }
}

proc WMETree::select { where num tree node } {
    variable dblclick

    set dblclick 1
    if { $num == 1 } {
        if { $where == "tree" && [lsearch [$tree selection get] $node] != -1 } {
            unset dblclick
            after 500 "WMETree::edit tree $tree $node"
            return
        }
        if { $where == "tree" } {
            select_node $tree $node
        }
    }
}


proc WMETree::select_node { tree node } {
    $tree selection set $node
    update

    set ID [$tree itemcget $node -data]
    if { [$tree itemcget $node -drawcross] == "allways" } {
        UpdateFromSoarMemory $tree $node $ID
    }

}


proc WMETree::edit { where tree node } {
    variable dblclick

    if { [info exists dblclick] } {
        return
    }

    if { $where == "tree" && [lsearch [$tree selection get] $node] != -1 } {
        set res [$tree edit $node [$tree itemcget $node -text]]
        if { $res != "" } {
            $tree itemconfigure $node -text $res
            $tree selection set $node
        }
        return
    }

}


proc WMETree::expand { tree but } {
   #puts "In expand"
    if { [set cur [$tree selection get]] != "" } {
        if { $but == 0 } {
            $tree opentree $cur
        } else {
            $tree closetree $cur
        }
    }
}

