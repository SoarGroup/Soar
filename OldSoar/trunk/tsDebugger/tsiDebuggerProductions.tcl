namespace eval Productions {
   variable notebook
   variable mainframe
   variable productionListbox
   variable breakIcon
   variable groupingColors
}

proc Productions::create { frame } {
   variable productionListbox
   variable breakIcon
   variable groupingColors
   global tsi_library
   
   set breakIcon(nogroup,0) [image create photo -file [file join $tsi_library images nogroup.gif]]
   set breakIcon(nogroup,1) [image create photo -file [file join $tsi_library images nogroup_break.gif]]
   set breakIcon(topgroup,0) [image create photo -file [file join $tsi_library images topgroup.gif]]
   set breakIcon(topgroup,1) [image create photo -file [file join $tsi_library images topgroup_break.gif]]
   set breakIcon(midgroup,0) [image create photo -file [file join $tsi_library images midgroup.gif]]
   set breakIcon(midgroup,1) [image create photo -file [file join $tsi_library images midgroup_break.gif]]
   set breakIcon(bottomgroup,0) [image create photo -file [file join $tsi_library images bottomgroup.gif]]
   set breakIcon(bottomgroup,1) [image create photo -file [file join $tsi_library images bottomgroup_break.gif]]
   set groupingColors {black darkblue darkgreen}

   set paneWin [PanedWindow $frame.pw -side top -weights available]
   set pane1 [$paneWin add -weight 35]
   set pane2 [$paneWin add -weight 55]

   # Set up list of user productions
   set labprodframe [TitleFrame $pane1.labf1 -text "Production List" -side left]
   set sw [ScrolledWindow [$labprodframe getframe].sw -relief sunken -borderwidth 2]
   set productionListbox [ListBox $sw.lb -height 8 -width 20 -highlightthickness 0 -selectmode single \
                           -background white -redraw 1]
   $sw setwidget $productionListbox
   pack $sw -fill both -expand yes

   $productionListbox bindText  <ButtonPress-1> "Productions::select"
   $productionListbox bindText  <Double-ButtonPress-1> "Productions::toggleBreakpoint"
   $productionListbox bindImage  <ButtonPress-1> "Productions::select"
   $productionListbox bindImage  <Double-ButtonPress-1> "Productions::toggleBreakpoint"

   # Create contents of production pages
   MatchTree::create $pane2 $productionListbox
   
   # Pack everything
   pack $labprodframe -fill both -expand yes
   pack $pane1 $pane2 -fill both -expand yes
   pack $paneWin -fill both -expand yes
   pack $frame -fill both -expand yes -padx 4 -pady 4
   
}

proc Productions::Update {} {
   variable productionListbox
   variable breakIcon
   
   global tsiConfig tcl_version

   output-strings-destination -push -append-to-result
   set userProds [print -user]
   set defProds  [print -default]
   output-strings-destination -pop
 
   if {$tsiConfig(maxUserProdMenu) > 0} {
      set userProds [lrange $userProds 0 $tsiConfig(maxUserProdMenu)]
   # TODO: Should put up a warning when production list is truncated
   }
   
   set lastChosen [$productionListbox selection get]
   $productionListbox delete [$productionListbox items]
   set productionColor "black"
   
   set finalProdList {}
   foreach v [lsort -dictionary [concat $userProds $defProds]] {
      set v [regsub -all \[|\] $v ""]
      set productionGroup [string range $v 0 [expr [string first "*" $v] - 1]]
      if {[info exists productionGroupCount($productionGroup)]} {
         incr productionGroupCount($productionGroup)
      } else {
         set productionGroupCount($productionGroup) 1
      }
      set productionGroupCount($productionGroup,maxvalue) $productionGroupCount($productionGroup)
      lappend finalProdList $v
      
   }

   output-strings-destination -push -append-to-result
   foreach v $finalProdList {
      set fc [getFiringCount [uplevel #0 eval "firing-count $v"]]
      set bp [getInterrupt [uplevel #0 eval "interrupt $v"]]
      if {($fc == "") || ($fc == "0")} {
         set nodeString "$v"
      } else {
         set nodeString "$v  -->  x$fc"
      }
      if {$bp} {
         set bpString "break"
      } else {
         set bpString "nobreak"
      }
      set productionGroup [string range $v 0 [expr [string first "*" $v] - 1]]
      if {$productionGroupCount($productionGroup,maxvalue) == 1} {
         set productionColor [getNextColor $productionColor]
         $productionListbox insert end $v -text $nodeString -fill $productionColor\
                  -data {enabled $bpString nogroup} -image $breakIcon(nogroup,$bp)
      } else {
         if {$productionGroupCount($productionGroup,maxvalue) == $productionGroupCount($productionGroup)} {
            set productionColor [getNextColor $productionColor]
            $productionListbox insert end $v -text $nodeString -fill $productionColor\
                  -data {enabled $bpString topgroup} -image $breakIcon(topgroup,$bp)
         } else {
            if {$productionGroupCount($productionGroup) == 1} {
               $productionListbox insert end $v -text $nodeString -fill $productionColor\
                     -data {enabled $bpString bottomgroup} -image $breakIcon(bottomgroup,$bp)
            } else {
               $productionListbox insert end $v -text $nodeString -fill $productionColor\
                     -data {enabled $bpString midgroup} -image $breakIcon(midgroup,$bp)
            }
         }
      incr productionGroupCount($productionGroup) -1
      }
   }
   output-strings-destination -pop
   if {[$productionListbox exists $lastChosen]} {
      $productionListbox selection set $lastChosen
      $productionListbox see $lastChosen
   }
}

proc Productions::getNextColor {someColor} {
   variable groupingColors
   
   set colorIndex [expr [lsearch $groupingColors $someColor] + 1]
   if {$colorIndex >= [llength $groupingColors]} {
      set colorIndex 0
   }
   return [lindex $groupingColors $colorIndex]
}

proc Productions::select { node } {
   variable productionListbox

   $productionListbox selection set $node
   if {$MatchTree::matchProduction != $node} {
      set MatchTree::matchProduction $node
      MatchTree::Update
   }
}

proc Productions::toggleBreakpoint { node } {
   variable productionListbox
   variable breakIcon
      
   $productionListbox selection set $node
   set isEnabled [lindex [$productionListbox itemcget $node -data] 0]
   set isBreakpointSet [lindex [$productionListbox itemcget $node -data] 1]
   set groupIcon [lindex [$productionListbox itemcget $node -data] 2]
   if {$isBreakpointSet == "nobreak"} {
      set isBreakpointSet "break"
      eval "interrupt $node -on"
      $productionListbox itemconfigure $node -image $breakIcon($groupIcon,1)
   } else {
      set isBreakpointSet "nobreak"
      eval "interrupt $node -off"
      $productionListbox itemconfigure $node -image $breakIcon($groupIcon,0)
   }
   $productionListbox itemconfigure $node -data [list $isEnabled $isBreakpointSet $groupIcon]
   if {$MatchTree::matchProduction != $node} {
      set MatchTree::matchProduction $node
   }
   MatchTree::Update
}
