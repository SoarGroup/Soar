namespace eval MatchTree {
    variable count
    variable dblclick
    variable currentRoot
    variable tree
    variable openNodes
    variable closedList
    variable wmeArray
    variable count
    variable matchProduction
    variable textItem
    variable paramFrame
}

proc MatchTree::CreateProductionMenu {} {
  global tsiConfig tcl_version
   variable paramFrame
  if [winfo exists $paramFrame.prod.m] {
    destroy $paramFrame.prod.m
  } else {
    menubutton $paramFrame.prod -text {Choose} -menu $paramFrame.prod.m -relief raised
  }

  menu $paramFrame.prod.m -tearoff 0 -relief raised
  $paramFrame.prod.m add command -label {Reread User Productions} -command "MatchTree::CreateProductionMenu"

  output-strings-destination -push -append-to-result
  set userProds [print -user]
  set defProds  [print -default]
  output-strings-destination -pop
 
  if {$tsiConfig(maxUserProdMenu) > 0} {
      set userProds [lrange $userProds 0 $tsiConfig(maxUserProdMenu)]
  }
  set userProds [concat $userProds $defProds]

   foreach v $userProds {
      set firstAsterisk [string first "*" $v]
      set firstLetter [string range $v 0 [expr $firstAsterisk - 1]]
      if {![info exists alphList($firstLetter)]} {
	      set newIndex [getMenuInsertionIndex \
		     $paramFrame.prod.m "[string toupper $firstLetter]'s" 1]
	      $paramFrame.prod.m insert $newIndex cascade \
		     -label "[string toupper $firstLetter]'s" \
		     -menu $paramFrame.prod.m.$firstLetter
	      menu $paramFrame.prod.m.$firstLetter -tearoff 0
	      set alphList($firstLetter) 1
      }
      set newIndex [getMenuInsertionIndex $paramFrame.prod.m.$firstLetter $v 0]
      set newCommand [subst "set MatchTree::matchProduction $v ; MatchTree::Update"]
      $paramFrame.prod.m.$firstLetter add command \
         -command "$newCommand" \
         -label $v
  }
  if {[expr $tcl_version >= 8.0]} {
     foreach item [array names alphList] {
	     set numEntries [$paramFrame.prod.m.$item index last]
	     for {set i 40} {$i < $numEntries} {incr i 40} {
	        $paramFrame.prod.m.$item entryconfigure $i -columnbreak 1
	     }
     }
  }
  return $paramFrame.prod
}

proc MatchTree::create { nb } {
   variable tree
   variable matchProduction
   variable textItem
   variable paramFrame
       
    set frame [$nb insert end MatchTree -text "Production Match"]

    set pw2   [PanedWindow $frame.pw2 -side top]
    set pane1  [$pw2 add]
    #set pane2  [$pw2 add -minsize 300]
    set pane2  [$pw2 add]
    
    set paramFrame [frame $frame.param]
    set matchProduction ""
    
    set ent   [LabelEntry $paramFrame.ent -label "Production Chosen: " -labelwidth 18 -labelanchor e \
                   -textvariable MatchTree::matchProduction \
                   -width 40 -editable 0\
                   -helptext "Production to match current WMEs against" \
                   -command {WMETree::ChangeRoot}]

    set prodMenu [CreateProductionMenu]
    pack $prodMenu $ent -side left -padx 5 
    pack $paramFrame -side top -padx 4 -anchor w

    set sep3  [Separator $frame.sep3 -orient horizontal]
    pack $sep3 -side top -fill x -pady 10

    set titf1 [TitleFrame $pane1.titf1 -text "Match"]
    set titf2 [TitleFrame $pane2.titf2 -text "WMEs"]

    set matchFrame1 [$titf1 getframe]
    set sw [ScrolledWindow $matchFrame1.sw -relief sunken -borderwidth 2 -auto both]
    set tx [text $sw.text -setgrid 1 -bd 2 -font {courier 9} -width 15]
    $sw setwidget $tx
    set textItem $sw.text
    pack $tx -fill both -expand yes -pady 4
    pack $sw -anchor w -expand yes -fill both
    pack $titf1 -side top -fill both -padx 4 -expand yes

    set matchFrame2 [$titf2 getframe]
    set sw2    [ScrolledWindow $matchFrame2.sw2 \
                  -relief sunken -borderwidth 2]
    set tree  [Tree $sw2.tree \
                   -relief flat -borderwidth 0 -width 15 -highlightthickness 0\
		   -redraw 0 -opencmd   "MatchTree::moddir 1 $sw2.tree" \
                   -closecmd  "MatchTree::moddir 0 $sw2.tree"]
    $sw2 setwidget $tree
    pack $sw2 -anchor w -expand yes -fill both
    pack $titf2 -side top -fill both -padx 4 -expand yes

    pack $pw2 -fill both -expand yes
    
    $tree bindText  <ButtonPress-1>        "MatchTree::select tree 1 $tree"
    $tree bindText  <Double-ButtonPress-1> "MatchTree::select tree 2 $tree"

    $nb itemconfigure MatchTree \
        -createcmd "MatchTree::init $tree" \
         -raisecmd  {
            Debugger::updateRaisedNotebook
        } \
        -leavecmd {
            set Debugger::sashPosition(MatchTree) $PanedWindow::panePercentage
            return 1
        }
}

proc MatchTree::GetOpenNodes {node} {
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

proc MatchTree::ShowOpenNodes {} {
   variable openNodes
   
   set openNodes {}
   MatchTree::GetOpenNodes home
}

proc MatchTree::CreateSubtreeNodesForChildren {tree node ID} {
   
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
   if {$ID != "Matched WMEs"} {
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
      if {[info exists wmeArray($t,nodeString)]} {
         set nodeString "$a = $v (MATCHED)"
         set nodeColor red
      } else {
         set nodeString "$a = $v"
         set nodeColor blue
      }
      $tree insert end $node n:$count \
         -text      $nodeString \
         -image     [Bitmap::get element] \
         -drawcross never \
         -fill $nodeColor  \
         -font {courier 8} \
         -data      value \
         -timeTag $t
      lappend sortList n:$count
      incr count
   }

   foreach t $childAdd {
      set a [lindex $wmeArray($t) 0]
      set v [lindex $wmeArray($t) 1]
      if {[info exists wmeArray($t,nodeString)]} {
         if {[info exists wmeArray($t,isTop)]} {
            set nodeString "$v"
         } else {
            set nodeString "$a -=> $v (MATCHED)"
         }
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

   if {$ID == "Matched WMEs"} {
      #puts "In CSNFC open last node $node $ID"
      set children [$tree nodes $node]
      set lastChild [lindex $children [expr [llength $children] -1]]
   } else {
      set sortList [lsort -command "compareNodes $tree" $sortList]
      $tree reorder $node $sortList
      $tree itemconfigure $node -drawcross auto -data $ID
   }
}

proc MatchTree::UpdateWmeIfChanged {timeTag ID att value} {
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

proc MatchTree::UpdateFromSoarMemory {tree node ID} {
   variable closedList
   variable wmeArray
   variable openNodes
   variable currentRoot
   variable matchProduction
   
   #puts "in MatchTree::UpdateFromSoarMemory"
   set someChanged 0
   
   if {$ID == "Matched WMEs"} {
      
      if {$matchProduction != ""} {

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

         #Must add code to handle bracketed conditions
         output-strings-destination -push -append-to-result
         set outText [eval "matches $matchProduction -wmes"]
         set matchList [list]
         set printMode 0
         foreach l [split $outText "\n"] {
            if {![catch "set listLength [llength $l]"]} {
              if {$listLength > 0} {
                  if {([string first "*** Matches For Left ***" $l] != -1) || \
                     ([string first "*** Complete Matches ***" $l] != -1)} {
                        set printMode 1
                  } elseif {([string first "*** Matches for Right ***" $l] != -1)} {
                        set printMode 0
                  } elseif {$printMode} {
                     #puts "Printing $l."
                     lappend matchList $l
                  }
               }
            } else {
            	   tk_dialog .error {Error} \
							 "There were problems parsing this match output.  Display may not be accurate." info 0 Ok
            }
         }
   
         foreach l $matchList {
            set l [split [string trim $l] " ()"]
            foreach {d1 t o a v p d2} $l {
               #puts "0 Starting : $t,$o,$a,$v,$p"
               set wmeArray($o,childList) {}
               set wmeArray($o,repeatList) {}
               set wmeArray($o,avList) {}
               #puts "1 Calling UpdateWmeIfChanged for $o Matched WMEs $o"
               incr someChanged [UpdateWmeIfChanged $o "Matched WMEs" " " $o]
               set wmeArray($o,nodeString) "$o$a$v"
               set wmeArray($o,isTop) 1
               #puts "2 Calling UpdateWmeIfChanged for: $t,$o,$a,$v,$p"
               incr someChanged [UpdateWmeIfChanged $t $o $a $v]
               set wmeArray($t,nodeString) "$o$a$v"
            }
         }
         #puts "Finished calling UpdateWmeIfChanged for all wmes"
    
         output-strings-destination -pop
      }
    
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

proc MatchTree::Update {} {
   variable tree
   variable openNodes
   variable currentRoot
   variable matchProduction
   variable textItem
      
   #puts "------\nUpdate calling initial UpdateFromSoarMemory"
   UpdateFromSoarMemory $tree home $currentRoot
   set openNodes [list]
   GetOpenNodes home
   foreach n $openNodes {
      #puts "Update calling UpdateFromSoarMemory for $n [$tree itemcget $n -data]"
      UpdateFromSoarMemory $tree $n [$tree itemcget $n -data]
   }
   #puts "Update finished\n------"
   
   output-strings-destination -push -append-to-result
   $textItem configure -state normal
   $textItem delete 1.0 end
   if {$matchProduction != ""} {
      #Must add code to handle bracketed conditions
      set outText [eval "matches $matchProduction"]
      set restBlack 0
      foreach l [split $outText "\n"] {
         if {[string first "matches" $l] != -1} {
            $textItem insert 1.0 "[string trimleft $l]\n\n" blueText
         } else {
            if {$restBlack || ([string first ">>>>" $l] != -1)} {
               $textItem insert end "[string trimleft $l]\n"
               set restBlack 1
            } else {
               $textItem insert end "[string trimleft $l]\n" redText
            }
         }
      }
     $textItem insert end "----------------------------------\n"
     $textItem insert end "Matches to Working Memory Elements"
     $textItem insert end "\n----------------------------------\n"
     
      set outText [eval "matches $matchProduction -wmes"]
      set printMode 0
      foreach l [split $outText "\n"] {
         if {$l != ""} {
            if {([string first "*** Matches For Left ***" $l] != -1) || \
                ([string first "*** Complete Matches ***" $l] != -1)} {
                  set printMode 1
            } elseif {([string first "*** Matches for Right ***" $l] != -1)} {
                  set printMode 0
            } elseif {$printMode} {
                  $textItem insert end "[string trimleft $l]\n" redText
            }
         }
      }
      $textItem tag configure redText -foreground red
      $textItem tag configure blueText -foreground blue
      $textItem see 1.0
   }
   
   output-strings-destination -pop
   update

}

proc MatchTree::init { tree args } {
    global   tcl_platform
    variable count
    variable currentRoot
    variable closedList 
    variable count
    variable wmeArray
   
    set currentRoot "Matched WMEs"

    set closedList [list]
    set count 0
    $tree insert end root home -text "$currentRoot" -data "$currentRoot" -open 1 \
       -font {courier 10 bold} -image [Bitmap::get opennode] -drawcross auto
   
    $tree configure -deltax 10

    set closedList [list]
    set wmeArray($currentRoot,avList) {}
    set wmeArray($currentRoot,childList) {}
    set wmeArray($currentRoot,repeatList) {}

    MatchTree::select tree 1 $tree home
    $tree configure -redraw 1

}

proc MatchTree::reloadTree {} {
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
   
   MatchTree::select tree 1 $tree home
 
}

proc MatchTree::moddir { idx tree node } {
variable closedList

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

proc MatchTree::select { where num tree node } {
    variable dblclick

    set dblclick 1
    if { $num == 1 } {
        if { $where == "tree" && [lsearch [$tree selection get] $node] != -1 } {
            unset dblclick
            after 500 "MatchTree::edit tree $tree $node"
            return
        }
        if { $where == "tree" } {
            select_node $tree $node
        }
    }
}


proc MatchTree::select_node { tree node } {
    $tree selection set $node
    update

    set ID [$tree itemcget $node -data]
    if { [$tree itemcget $node -drawcross] == "allways" } {
        #puts "Select_node calling UpdateFromSoarMemory"
        UpdateFromSoarMemory $tree $node $ID
    }

}


proc MatchTree::edit { where tree node } {
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


proc MatchTree::expand { tree but } {

    if { [set cur [$tree selection get]] != "" } {
        if { $but == 0 } {
            $tree opentree $cur
        } else {
            $tree closetree $cur
        }
    }
}

