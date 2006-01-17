namespace eval StateTree {
    variable count
    variable dblclick
    variable currentRoot
    variable tree
    variable openNodes
    variable closedList
    variable wmeArray
    variable count
}

proc StateTree::create { nb } {
variable tree
    
    set frame [$nb insert end StateTree -text "State Stack"]
    set pw    [PanedWindow $frame.pw -side top]

    set pane  [$pw add -weight 1]
    set title [TitleFrame $pane.lf -text "State Stack"]
    set sw    [ScrolledWindow [$title getframe].sw \
                  -relief sunken -borderwidth 2]
    set tree  [Tree $sw.tree \
                   -relief flat -borderwidth 0 -width 15 -highlightthickness 0\
		   -redraw 0 -opencmd   "StateTree::moddir 1 $sw.tree" \
                   -closecmd  "StateTree::moddir 0 $sw.tree"]
    $sw setwidget $tree

    pack $sw    -side top  -expand yes -fill both
    pack $title -fill both -expand yes

 
    pack $pw -fill both -expand yes

    $tree bindText  <ButtonPress-1>        "StateTree::select tree 1 $tree"
    $tree bindText  <Double-ButtonPress-1> "StateTree::select tree 2 $tree"

    $nb itemconfigure StateTree \
        -createcmd "StateTree::init $tree" \
         -raisecmd  {
            Debugger::updateRaisedNotebook
        } \
        -leavecmd {
            set Debugger::sashPosition(StateTree) $PanedWindow::panePercentage
            return 1
        }
}

proc StateTree::GetOpenNodes {node} {
   variable tree 
   variable openNodes
   
   set children [$tree nodes $node]
   
   if {[llength $children] > 0} {
      lappend openNodes $node
      foreach c $children {
         GetOpenNodes $c
      }
   }
}

proc StateTree::ShowOpenNodes {} {
   variable openNodes
   
   set openNodes {}
   StateTree::GetOpenNodes home
}

proc StateTree::CreateSubtreeNodesForChildren {tree node ID} {
   
   variable wmeArray
   variable closedList
   variable count
   
   set childrenIDs {}
   set children [$tree nodes $node]
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
   if {$ID != "StateStack"} {
      foreach item $childrenIDs {
         if {[lsearch $fullList $item] == -1} {
            $tree delete $childrenNodes($item)
         } else {
            lappend sortList $childrenNodes($item)
         }
      }
   } else {
      foreach item $childrenIDs {
         if {[lsearch $fullList $item] == -1} {
            $tree delete $childrenNodes($item)
         } else {
            lappend sortList $childrenNodes($item)
         }
         if {[info exists wmeArray($item,newOp)]} {
            $tree itemconfigure $childrenNodes($item) -text $wmeArray($item,nodeString)
            unset wmeArray($item,newOp)
         }
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
      #puts "Creating node for $a $v."
      if {[info exists wmeArray($t,nodeString)]} {
         set nodeString $wmeArray($t,nodeString)
         set nodeColor red
      } else {
         set nodeString "$a -=> $v"
         set nodeColor black
      }
      if {([lsearch $wmeArray($ID,repeatList) $v]!= -1)} {
          set wmeArray($v,avList) {}
          set wmeArray($v,childList) {}
          set wmeArray($v,repeatList) {}
         $tree insert end $node n:$count \
             -text      $nodeString \
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
             -text      $nodeString \
             -image     [Bitmap::get closednode] \
             -drawcross allways \
             -fill $nodeColor \
             -font {courier 8 bold} \
             -data      $v \
             -timeTag $t
      }
      lappend sortList n:$count
      incr count
   }

   if {$ID == "StateStack"} {
      #puts "In CSNFC open last node $node $ID"
      set children [$tree nodes $node]
      set lastChild [lindex $children [expr [llength $children] -1]]
   } else {
      set sortList [lsort -command "compareNodes $tree" $sortList]
      $tree reorder $node $sortList
      $tree itemconfigure $node -drawcross auto -data $ID
   }
}

proc StateTree::GetOpInfo {ID} {
variable wmeArray
   
   output-strings-destination -push -append-to-result
   set searchString [subst "\\{($ID ^operator *)\\}"]
   set op [removeLastChar [lindex [eval "print -internal -depth 0 $searchString"] 3]]
   if {[info exists wmeArray($ID,op)] && ($op == $wmeArray($ID,op))} {
      output-strings-destination -pop
      return 0
   }

   set wmeArray($ID,op) $op
   
   set wmeArray($ID,newOp) 1
   set searchString [subst "\\{($op ^name *)\\}"]
   set opName [removeLastChar [lindex [eval "print -internal -depth 0 $searchString"] 3]]
   if {$op == ""} {
      set wmeArray($ID,nodeString) "<$ID>: NO selected operator"
   } else {
      if {$opName == ""} {
         set wmeArray($ID,nodeString) "<$ID>: $op (no ^name)"
      } else {
         set wmeArray($ID,nodeString) "<$ID>: $op $opName"
      }
   }
   output-strings-destination -pop
   return 1
}

proc StateTree::UpdateWmeIfChanged {timeTag ID att value} {
variable closedList
variable wmeArray
   set retValue 0
   if {[isId $value]} {
      if {[lsearch $closedList $att] == -1} {
         if {[lsearch $wmeArray($ID,childList) $timeTag] != -1} {
         } else {
            lappend wmeArray($ID,childList) $timeTag
            set wmeArray($timeTag) [list $att $value]
            set retValue 1
         }
      } else {
         if {[lsearch $wmeArray($ID,avList) $timeTag] != -1} {
         } else {
            lappend wmeArray($ID,avList) $timeTag
            set wmeArray($timeTag) [list $att $value]
            set retValue 1
         }
      }
   } else {
      if {[lsearch $wmeArray($ID,avList) $timeTag] != -1} {
      } else {
         lappend wmeArray($ID,avList) $timeTag
         set wmeArray($timeTag) [list $att $value]
            set retValue 1
      }
   }
   return $retValue
}

proc StateTree::UpdateFromSoarMemory {tree node ID} {
variable closedList
variable wmeArray

   #puts "in UpdateFromSoarMemory"
   set someChanged 0
   
   if {$ID == "StateStack"} {
      
      output-strings-destination -push -append-to-result
      set searchString [subst "\\{(* ^superstate *)\\}"]
      set stateStack [eval "print -internal -depth 0 $searchString"]
    
      if [info exists wmeArray($ID,childList)] {
         set wmeArray($ID,childList) {}
      }
      if [info exists wmeArray($ID,repeatList)] {
         set wmeArray($ID,repeatList) {}
      }
      set delList [concat [array get wmeArray *newOp] [array get wmeArray *op] [array get wmeArray *nodeString]]
      foreach {x y} $delList {
         unset wmeArray($x)
      }

      foreach {t o a v} $stateStack {
         #puts "Calling UpdateWmeIfChanged for $t $o $a $v"
         incr someChanged [UpdateWmeIfChanged $o "StateStack" "Sub-State" $o]
         GetOpInfo $o
         if {$wmeArray($o,op) != ""} {
            incr someChanged [UpdateWmeIfChanged $wmeArray($o,op) "StateStack" "Operator" $wmeArray($o,op)]
         }
      }
      #puts "Finished calling UpdateWmeIfChanged for all wmes"
    
    
     output-strings-destination -pop
    
   } else {
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
                  incr someChanged [UpdateWmeIfChanged $timeTag $object $attr $w]
               }
               set index 0
            }   
         }   
      }
   }
   
   if {$someChanged} {
      CreateSubtreeNodesForChildren $tree $node $ID
   }
   set closedList [lUniqueAppend $closedList $ID]
}

proc StateTree::Update {} {
   variable tree
   variable openNodes
   variable currentRoot
   
   #puts "------\nUpdate calling initial UpdateFromSoarMemory"
   UpdateFromSoarMemory $tree home $currentRoot
   set openNodes [list]
   GetOpenNodes home
   foreach n $openNodes {
      #puts "Update calling UpdateFromSoarMemory for $n [$tree itemcget $n -data]"
      UpdateFromSoarMemory $tree $n [$tree itemcget $n -data]
   }
   #puts "Update finished\n------"

}

proc StateTree::init { tree args } {
    global   tcl_platform
    variable count
    variable currentRoot
    variable closedList 
    variable count
    variable wmeArray
   
    set currentRoot "StateStack"

    set closedList [list]
    set count 0
    $tree insert end root home -text "$currentRoot" -data "$currentRoot" -open 1 \
       -font {courier 10 bold} -image [Bitmap::get opennode] -drawcross auto
   
    $tree configure -deltax 10

    set closedList [list]
    set wmeArray($currentRoot,avList) {}
    set wmeArray($currentRoot,childList) {}
    set wmeArray($currentRoot,repeatList) {}

    #puts "-\nStarting inits UpdateFromSoarMemory"
    UpdateFromSoarMemory $tree home $currentRoot
    #puts "Finishing inits UpdateFromSoarMemory\n-"
    
    StateTree::select tree 1 $tree home
    $tree configure -redraw 1

}

proc StateTree::reloadTree {} {
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

   #puts "Reload calling UpdateFromSoarMemory"
   UpdateFromSoarMemory $tree home $currentRoot
   
   StateTree::select tree 1 $tree home
 
}

proc StateTree::moddir { idx tree node } {
variable closedList
   #puts "In MODDIR"
   if { $idx && [$tree itemcget $node -drawcross] == "allways" } {
      set ID [$tree itemcget $node -data]
      #puts "Moddir calling UpdateFromSoarMemory"
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

proc StateTree::select { where num tree node } {
    variable dblclick

    set dblclick 1
    if { $num == 1 } {
        if { $where == "tree" && [lsearch [$tree selection get] $node] != -1 } {
            unset dblclick
            after 500 "StateTree::edit tree $tree $node"
            return
        }
        if { $where == "tree" } {
            select_node $tree $node
        }
    }
}


proc StateTree::select_node { tree node } {
    $tree selection set $node
    update

    set ID [$tree itemcget $node -data]
    if { [$tree itemcget $node -drawcross] == "allways" } {
        #puts "Select_node calling UpdateFromSoarMemory"
        UpdateFromSoarMemory $tree $node $ID
    }

}


proc StateTree::edit { where tree node } {
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


proc StateTree::expand { tree but } {
    if { [set cur [$tree selection get]] != "" } {
        if { $but == 0 } {
            $tree opentree $cur
        } else {
            $tree closetree $cur
        }
    }
}

