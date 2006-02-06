namespace eval WMETree {
   variable treeWidgets
   variable wmeArray
   variable openNodes
   variable count
   variable closedList
   variable deleteList
   variable treeRootLists
   variable specialFormat
}

proc WMETree::create { frame thisTree} {
   variable treeWidgets
   
   set sw   [ScrolledWindow $frame.sw \
                  -relief sunken -borderwidth 2]
   set tree [Tree $sw.tree \
                        -relief flat -borderwidth 0 -width 15 -highlightthickness 0\
		   -redraw 0 -opencmd   "WMETree::ToggleNode 1 $thisTree" \
                        -closecmd  "WMETree::ToggleNode 0 $thisTree"]
   $sw setwidget $tree

   pack $sw -side bottom -expand yes -fill both
   
   $tree bindText <ButtonPress-1>         "WMETree::select_node $thisTree $tree"
   $tree bindText <Double-ButtonPress-1>  "WMETree::select_node $thisTree $tree"
   $tree bindText <ButtonPress-3>         "WMETree::popup $thisTree $tree %X %Y 1"
   $tree bindImage <ButtonPress-1>         "WMETree::select_node $thisTree $tree"
   $tree bindImage <Double-ButtonPress-1>  "WMETree::select_node $thisTree $tree"
   $tree bindImage <ButtonPress-3>         "WMETree::popup $thisTree $tree %X %Y 1"
    
   set treeWidgets($thisTree) $tree
}

proc WMETree::init { thisTree } {
   variable treeWidgets
   variable openNodes
   variable closedList
   variable deleteList
   variable count
   variable wmeArray
   variable treeRootLists
   variable specialFormat

   set tree $treeWidgets($thisTree)
   set openNodes($thisTree) {}
   set closedList($thisTree) {}
   set deleteList($thisTree) {}
   set count($thisTree) 0
   deleteArrayItems WMETree::wmeArray $thisTree
   set treeRootLists($thisTree) {}
   set specialFormat($thisTree) {}
   
   $tree configure -deltax 10
   $tree configure -redraw 1
}

proc WMETree::addRoot {thisTree newRoot} {
   variable treeWidgets
   variable treeRootLists
   variable wmeArray

   set tree $treeWidgets($thisTree)
   lappend treeRootLists($thisTree) $newRoot
   $tree insert end root $newRoot -text "$newRoot" -data [list $newRoot ROOT] -open 1 \
         -font {courier 10 bold} -image [Bitmap::get opennode] -drawcross auto
   set wmeArray($thisTree,$newRoot,avList) {}
   set wmeArray($thisTree,$newRoot,childList) {}
   set wmeArray($thisTree,$newRoot,repeatList) {}
   UpdateFromSoarMemory $thisTree $newRoot $newRoot
   WMETree::select_node $thisTree $tree $newRoot

}

proc WMETree::DestroyTree {thisTree} {
   variable treeWidgets

   set tree $treeWidgets($thisTree)
   $tree delete [$tree nodes root]
   deleteArrayItems WMETree::wmeArray $thisTree
   unsetIfExists WMETree::treeWidgets($thisTree)
   unsetIfExists WMETree::openNodes($thisTree)
   unsetIfExists WMETree::closedList($thisTree)
   unsetIfExists WMETree::deleteList($thisTree)
   unsetIfExists WMETree::count($thisTree)
   unsetIfExists WMETree::treeRootLists($thisTree)
   unsetIfExists WMETree::specialFormat($thisTree)
}

proc WMETree::debugPrint {} {
   variable treeWidgets
   variable openNodes
   variable closedList
   variable deleteList
   variable count
   variable wmeArray
   variable treeRootLists
   variable specialFormat

   foreach v {treeWidgets openNodes closedList deleteList count wmeArray treeRootLists specialFormat} {
      puts "\n$v is [array get $v]"
   }

}

proc WMETree::deleteRoot {thisTree delRoot} {
   variable wmeArray
   variable treeRootLists
   variable treeWidgets
   
   set tree $treeWidgets($thisTree)
   set treeRootLists($thisTree) [ldelete $treeRootLists($thisTree) $delRoot]
   deleteArrayItems WMETree::wmeArray "$thisTree,$delRoot"
   $tree delete $delRoot
   
}

proc WMETree::GetOpenNodes {thisTree node} {
   variable openNodes
   variable treeWidgets
   
   set tree $treeWidgets($thisTree)
   #puts "GetOpenNodes called with $node ([$tree itemcget $node -text])"
   set children [$tree nodes $node]
   
   if {[llength $children] > 0} {
         #puts "---GON---> Node $node ([$tree itemcget $node -text]) has been open"
         lappend openNodes($thisTree) $node
         foreach c $children {
                GetOpenNodes $thisTree $c
         }
   }
}

proc WMETree::CreateSubtreeNodesForChildren {thisTree node ID} {
   
   variable wmeArray
   variable closedList
   variable count
   variable deleteList
   variable treeWidgets
   variable specialFormat
      
   #puts "Starting CreateSubtreeNodesForChildren for $node $ID"

   set tree $treeWidgets($thisTree)

   set childrenIDs {}
   set children [$tree nodes $node]
   #puts "--CSNFC--> Node $node has children $children"
   foreach c $children {
      set ttag [lindex [$tree itemcget $c -data] 1]
      lappend childrenIDs $ttag
      set childrenNodes($ttag) $c
   }

   set avAdd {}
   set childAdd {}
   set sortList {}
      
   foreach item $wmeArray($thisTree,$ID,avList) {
      if {[lsearch $childrenIDs $item] == -1} {
         lappend avAdd $item
      }
   }
   foreach item $wmeArray($thisTree,$ID,childList) {
      if {[lsearch $childrenIDs $item] == -1} {
         lappend childAdd $item
      }
   }
   set fullList [concat $wmeArray($thisTree,$ID,childList) $wmeArray($thisTree,$ID,avList)]
   foreach item $childrenIDs {
      if {[lsearch $fullList $item] == -1} {
         #puts "--CSNFC--> DELETING $childrenNodes($item) b/c $item is not in $fullList"
         lappend deleteList($thisTree) $childrenNodes($item)
      } else {
         lappend sortList $childrenNodes($item)
      }
   }
   foreach t $avAdd {
      set a [lindex $wmeArray($thisTree,$t) 0]
      set v [lindex $wmeArray($thisTree,$t) 1]
      if {([lsearch $specialFormat($thisTree) $t]!= -1)} {
         set nodeString "$a = $v (MATCHED)"
         set nodeColor red
      } else {
         set nodeString "$a = $v"
         set nodeColor black
      }
      $tree insert end $node n:$count($thisTree) \
         -text      $nodeString \
         -image   [Bitmap::get element] \
         -drawcross never \
         -fill $nodeColor  \
         -font {courier 8} \
         -data      [list value $t] 
         
      lappend sortList n:$count($thisTree)
      incr count($thisTree)
   }

   foreach t $childAdd {
      set a [lindex $wmeArray($thisTree,$t) 0]
      set v [lindex $wmeArray($thisTree,$t) 1]
      if {([lsearch $specialFormat($thisTree) $t]!= -1)} {
         set nodeString "$a = $v (MATCHED)"
         set nodeColor red
      } else {
         set nodeString "$a = $v"
         set nodeColor blue
      }
      if {([lsearch $wmeArray($thisTree,$ID,repeatList) $v]!= -1)} {
         set nodeColor red
         set nodeFont {courier 8 bold}
      } else {
         set nodeFont {courier 8}
      }
      set wmeArray($thisTree,$v,avList) {}
      set wmeArray($thisTree,$v,childList) {}
      set wmeArray($thisTree,$v,repeatList) {}
      $tree insert end $node n:$count($thisTree) \
               -text      $nodeString \
               -image   [Bitmap::get closednode] \
               -drawcross allways \
               -fill $nodeColor \
               -font $nodeFont \
               -data      [list $v $t]
      lappend sortList n:$count($thisTree)
      incr count($thisTree)
   }
   
   set lastTimeTag [lindex [$tree itemcget $node -data] 1]
   $tree itemconfigure $node -drawcross auto -data [list $ID $lastTimeTag]

   set sortList [lsort -command "compareNodes $tree" $sortList]
   $tree reorder $node $sortList

   #puts "Finishing CreateSubtreeNodesForChildren for $node $ID"
}

proc compareNodes {tree node1 node2} {
   
   return [string compare [$tree itemcget $node1 -text] [$tree itemcget $node2 -text]]

}

proc WMETree::UpdateWmeIfChanged {thisTree timeTag ID att value} {
variable closedList
variable wmeArray

   if {[isId $value]} {
      if {[lsearch $closedList($thisTree) $att] == -1} {
         if {[lsearch $wmeArray($thisTree,$ID,childList) $timeTag] != -1} {
         } else {
            lappend wmeArray($thisTree,$ID,childList) $timeTag
            set wmeArray($thisTree,$timeTag) [list $att $value]
         }
      } else {
         if {[lsearch $wmeArray($thisTree,$ID,avList) $timeTag] != -1} {
         } else {
            lappend wmeArray($thisTree,$ID,avList) $timeTag
            set wmeArray($thisTree,$timeTag) [list $att $value]
         }
      }
   } else {
      if {[lsearch $wmeArray($thisTree,$ID,avList) $timeTag] != -1} {
      } else {
         lappend wmeArray($thisTree,$ID,avList) $timeTag
         set wmeArray($thisTree,$timeTag) [list $att $value]
      }
   }
}

proc WMETree::UpdateFromSoarMemory {thisTree node ID} {
variable closedList
variable wmeArray
   #puts "Starting UpdateFromSoarMemory for $node $ID" 
   
   if [info exists wmeArray($thisTree,$ID,isFrozen)] {
      return
   }   

   output-strings-destination -push -append-to-result
   set searchString [subst "\\{($ID ^* *)\\}"]
   set wmeList [eval "print -depth 0 -internal $searchString"]
   output-strings-destination -pop
   
   if [info exists wmeArray($thisTree,$ID,avList)] {
      set wmeArray($thisTree,$ID,avList) {}
   }
   if [info exists wmeArray($thisTree,$ID,childList)] {
      set wmeArray($thisTree,$ID,childList) {}
   }
   if [info exists wmeArray($thisTree,$ID,repeatList)] {
      set wmeArray($thisTree,$ID,repeatList) {}
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
                if {[lsearch $closedList($thisTree) $w]!=-1} {
                        if {![info exists wmeArray($thisTree,$ID,repeatList)]} {
                         set wmeArray($thisTree,$ID,repeatList) [list $w]
                        } else {
                         set wmeArray($thisTree,$ID,repeatList) [lUniqueAppend $wmeArray($thisTree,$ID,repeatList) $w]
                        }
                }
                UpdateWmeIfChanged $thisTree $timeTag $object $attr $w
               }
               set index 0
         }   
      }   
   }
   
   CreateSubtreeNodesForChildren $thisTree $node $ID
   set closedList($thisTree) [lUniqueAppend $closedList($thisTree) $ID]
   #puts "Finishing UpdateFromSoarMemory for $node $ID" 
}

proc WMETree::Update {thisTree} {
   variable treeWidgets
   variable openNodes
   variable deleteList
   variable treeRootLists
      
   set tree $treeWidgets($thisTree)
   set openNodes($thisTree) $treeRootLists($thisTree)
   set deleteList($thisTree) {}
         
   #puts "Starting WMETree Update." 
   foreach o $treeRootLists($thisTree) {
      GetOpenNodes $thisTree $o
   }
   if {[lindex $openNodes($thisTree) 0] == [lindex $openNodes($thisTree) 1]} {
         set openNodes($thisTree) [ltail $openNodes($thisTree)]
   }
   #puts "--Updat--> openNodes generated.  Starting UFSM for $openNodes($thisTree)" 
   foreach n $openNodes($thisTree) {
         #puts "--Updat--> Updating for $n" 
      UpdateFromSoarMemory $thisTree $n [lindex [$tree itemcget $n -data] 0]
   }
   foreach n $deleteList($thisTree) {
         $tree delete $n
   }
   #puts "Done WMETree Update"
}

proc WMETree::OpenAllNodesDepth {thisTree depth} {
   variable treeRootLists

   foreach o $treeRootLists($thisTree) {
      OpenDepth $thisTree $o $depth
   }
}

proc WMETree::OpenDepth {thisTree node depth} {
   variable treeWidgets

   set tree $treeWidgets($thisTree)
   set childNodes [$tree nodes $node]

   foreach n $childNodes {
      set nodeData [lindex [$tree itemcget $n -data] 0]
      if {$nodeData != "value"} {
         UpdateFromSoarMemory $thisTree $n $nodeData
         if {$depth} {
            OpenDepth $thisTree $n [expr $depth - 1]
         }
      }
   }

   $tree itemconfigure $node -drawcross auto -open 1
}

proc WMETree::reloadTree {thisTree {newRoots {}} } {
   variable closedList
   variable wmeArray
   variable treeWidgets
   variable treeRootLists
   
   set tree $treeWidgets($thisTree)
   $tree delete [$tree nodes root]
   deleteArrayItems WMETree::wmeArray $thisTree
   set closedList($thisTree) [list]
   set count($thisTree) 0
   
   if {$newRoots == {} } {
      set newRoots $treeRootLists($thisTree)
   } elseif {$newRoots == {EMPTY} } {
      set newRoots {}
   }

   set treeRootLists($thisTree) {}
   foreach r $newRoots {
      addRoot $thisTree $r
   }
}

proc WMETree::ToggleNode { idx thisTree node } {
   variable closedList
   variable treeWidgets

   set tree $treeWidgets($thisTree)
   set ID [lindex [$tree itemcget $node -data] 0]

   if { $idx && [$tree itemcget $node -drawcross] == "allways" } {
      UpdateFromSoarMemory $thisTree $node $ID
      set closedList($thisTree) [lUniqueAppend $closedList($thisTree) $ID]
   }
   $tree itemconfigure $node -image [GetNodeImage $thisTree $tree $node $ID]

}

proc WMETree::select_node { thisTree tree node } {

   $tree selection set $node
   update

   #set ID [lindex [$tree itemcget $node -data] 0]
   #if { [$tree itemcget $node -drawcross] == "allways" } {
   #    UpdateFromSoarMemory $thisTree $node $ID
   #}
}

proc WMETree::DeleteTree { thisTree } {
   variable specialFormat

   unset specialFormat($thisTree)

}

proc WMETree::GetNodeImage { thisTree tree node ID } {
   variable wmeArray

   if {[info exists wmeArray($thisTree,$ID,isFrozen)]} {
      set imagePrefix "ice_"
   } else {
      set imagePrefix ""
   }
   if { [$tree itemcget $node -open] } {
       set imageName "opennode"
   } else {
      set imageName "closednode"
   }
   return [Bitmap::get $imagePrefix$imageName]
}

proc WMETree::ToggleFrozen { thisTree tree node ID } {
   variable wmeArray
   
   if {![info exists wmeArray($thisTree,$ID,isFrozen)]} {
      set wmeArray($thisTree,$ID,isFrozen) true
      output-strings-destination -push -append-to-result
      set dc [eval "stats -system -dc-count"]
      output-strings-destination -pop
      set wmeArray($thisTree,$ID,unFrozenText) [$tree itemcget $node -text]
      $tree itemconfigure $node -text "$wmeArray($thisTree,$ID,unFrozenText) (Frozen at time $dc)"
   } else {
      $tree itemconfigure $node -text $wmeArray($thisTree,$ID,unFrozenText)
      unset wmeArray($thisTree,$ID,isFrozen)
      unset wmeArray($thisTree,$ID,unFrozenText)
      WMETree::Update $thisTree
   }
   $tree itemconfigure $node -image [GetNodeImage $thisTree $tree $node $ID]
   
}

proc WMETree::popup { thisTree tree x y auto_finish node } {
   variable wmeArray
   
   $tree selection set $node

   set ID [lindex [$tree itemcget $node -data] 0]
   if [winfo exists .wmepopup] {
      destroy .wmepopup
   }
   menu .wmepopup  -tearoff 0
   if {$ID != "value"} {
      foreach w [PaneManager::GetWatchPanes] {
         .wmepopup  add command -label "Add $ID to [lindex $w 1]" \
                          -command [list WatchPane::addRoot [lindex $w 0] $ID]
      }
      .wmepopup  add command -label "Open $ID in New WME Pane" \
                       -command [list puts "hi"]
      .wmepopup add separator
      if {![info exists wmeArray($thisTree,$ID,isFrozen)]} {
         .wmepopup  add command -label "Freeze this Item" \
                     -command [list WMETree::ToggleFrozen $thisTree $tree $node $ID]
      } else {
         .wmepopup  add command -label "Unfreeze this Item" \
                       -command [list WMETree::ToggleFrozen $thisTree $tree $node $ID]
      }
      .wmepopup add separator
   } else {
      .wmepopup  add command -label "Sorry But Commands Available Only For Soar IDs" 
   }
   if {$auto_finish} {
      popup_finish $x $y
   }
}

proc WMETree::popup_finish { x y } {
   update
   tk_popup .wmepopup $x $y
}