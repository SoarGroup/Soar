namespace eval WatchPane {
   variable watchList
   variable treeName
}

proc WatchPane::create { frame paneID paneTitle} {
   variable treeName
   variable watchList
   
   set treeName($paneID) "tree$paneID"
   set watchList($paneID) {}
   
   set topf  [frame $frame.topf]
   set titf1 [TitleFrame $topf.titf1 -text "$paneTitle" -ipad 0]
   set parent [$titf1 getframe]
   
   WMETree::create $parent $treeName($paneID)
   WMETree::init $treeName($paneID)

   set myTree $WMETree::treeWidgets($treeName($paneID))
   $myTree bindText <ButtonPress-3>         "WatchPane::popup $paneID $myTree %X %Y"

   pack $titf1 -side left -padx 4 -pady 0 -fill both -expand yes
   pack $parent -side top -fill both -expand yes
   pack $topf -side top -fill both -expand yes
}

proc WatchPane::Update {} {
   variable treeName
   
   #TODO:  Change so PaneManager updates a specific pane
   foreach t [array names treeName] {
      WMETree::Update $treeName($t)
   }
}

proc WatchPane::addRoot {paneID newRoot} {
   variable treeName
   variable watchList
   
   if {[lsearch $watchList($paneID) $newRoot] == -1} {
      WMETree::addRoot $treeName($paneID) $newRoot
      WMETree::Update $treeName($paneID)
      lappend watchList($paneID) $newRoot
   } else {
      tk_dialog .error {Existence Error} "Don't worry.  $newRoot is already on the watch list." info 0 Ok
   }
}

proc WatchPane::deleteRoot {paneID delRoot} {
   variable watchList
   variable treeName
   if {[lsearch $watchList($paneID) $delRoot] != -1} {
      set watchList($paneID) [ldelete $watchList($paneID) $delRoot]
      WMETree::deleteRoot $treeName($paneID) $delRoot
   } else {
      tk_dialog .error {Existence Error} "Cannot delete $delRoot because it is not on the watch list.  Contact the Soar group immediately!" info 0 Ok
   }
}

proc WatchPane::DestroyPane {paneID} {
   variable treeName
   variable watchList
   
   WMETree::DestroyTree $treeName($paneID)
   unset watchList($paneID)
   unset treeName($paneID)
}

proc WatchPane::debugPrint {} {
   variable treeName
   variable watchList

   foreach v {treeName watchList} {
      puts "\n$v is [array get $v]"
   }

}


proc WatchPane::popup { paneID tree x y node } {
   variable treeName
   variable watchList

   WMETree::popup $treeName($paneID) $tree $x $y 0 $node
   set ID [lindex [$tree itemcget $node -data] 0]
   if {$ID != "value"} {
      #puts "Not a value"
      if {[lsearch $watchList($paneID) $ID] != -1} {
         #puts "Adding new popup command."
         .wmepopup  add command -label "Remove $ID from Watch View" \
                       -command [list WatchPane::deleteRoot $paneID $ID]
      } 
   }
   
   WMETree::popup_finish $x $y
}
