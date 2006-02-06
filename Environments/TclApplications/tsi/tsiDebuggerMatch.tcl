namespace eval MatchTree {
   variable matchProduction
   variable matchTextItem
   variable paramFrame
   variable prodViewItem
   variable productionListbox
   variable matchWMEs
   variable matchTimeTags
}

proc MatchTree::create { frame productionListboxparam} {
   variable matchProduction
   variable matchTextItem
   variable paramFrame
   variable prodViewItem
   variable productionListbox
   variable matchWMEs
   variable matchTimeTags
   
   set productionListbox $productionListboxparam
   set matchProduction ""
   set matchWMEs {}
   set matchTimeTags {}
      
   set pw1   [PanedWindow $frame.pw1 -side left -weights available]
   set paneNew      [$pw1 add -weight 35]
   set prodViewItem [CreateInfoPane $paneNew "Production"]
   set pane1b  [$pw1 add -weight 65]

   set pw2   [PanedWindow $pane1b.pw2 -side top -weights available]
   set paneNew      [$pw2 add -weight 33]
   set matchTextItem [CreateInfoPane $paneNew "Match Output"]

   set pane3  [$pw2 add -weight 33]
   set titf2 [TitleFrame $pane3.titf2 -text "WMEs Currently Matched" -side left]
   set matchFrame2 [$titf2 getframe]

   WMETree::create $matchFrame2 matchtree
   WMETree::init matchtree
   
   pack $titf2 -side top -fill both -padx 4 -expand yes
   pack $pane3 -fill both -expand yes
   pack $pw2 -fill both -expand yes
   pack $pane1b -fill both -expand yes
   pack $pw1 -fill both -expand yes
   pack $frame -fill both -expand yes
}


proc MatchTree::Update {} {
   variable matchProduction
   variable matchTextItem
   variable prodViewItem
   variable matchWMEs
   variable matchTimeTags
   
   output-strings-destination -push -append-to-result

   $matchTextItem delete 1.0 end
   $matchTextItem configure -state normal
   $prodViewItem configure -state normal
   if {$matchProduction != ""} {
      $prodViewItem delete 1.0 end
      $prodViewItem insert 1.0 [eval "print $matchProduction"]
      $matchTextItem delete 1.0 end

      # WARNING Must add code to handle bracketed conditions
      set outText [eval "matches $matchProduction"]
      set restBlack 0
      foreach l [split $outText "\n"] {
         if {[string first "matches" $l] != -1} {
            $matchTextItem insert 1.0 "[string trimleft $l]\n\n" blueText
         } else {
            if {$restBlack || ([string first ">>>>" $l] != -1)} {
               $matchTextItem insert end "[string trimleft $l]\n"
               set restBlack 1
            } else {
               $matchTextItem insert end "[string trimleft $l]\n" redText
            }
         }
      }

      $matchTextItem insert end "----------------------------------\n"
      $matchTextItem insert end "Matches to Working Memory Elements"
      $matchTextItem insert end "\n----------------------------------\n"
    
      set outText [eval "matches $matchProduction -wmes"]
      set printMode 0
      set matchList [list]
      foreach l [split $outText "\n"] {
         ## WARNING:  CHANGED so that $l is in brackets.  Not tested fully.
         if {![catch "set listLength [llength {$l}]"]} {
            if {$listLength > 0} {
               if {([string first "*** Matches For Left ***" $l] != -1) || \
                   ([string first "*** Complete Matches ***" $l] != -1)} {
                  set printMode 1
               } elseif {([string first "*** Matches for Right ***" $l] != -1)} {
                  set printMode 0
               } elseif {$printMode} {
                  lappend matchList $l
                 $matchTextItem insert end "[string trimleft $l]\n" redText
               }
            }
         } else {
         	   tk_dialog .error {Error} \
							 "There were problems parsing this match output.  Display may not be accurate." info 0 Ok
         }
      }
      set matchWMEs {}
      set matchTimeTags {}
      set WMETree::specialFormat(matchtree) {}
      foreach l $matchList {
         set l [split [string trim $l] " ()"]
         foreach {d1 t o a v p d2} $l {
            #puts "Adding $t $o $a $v $p"
            set matchWMEs [lUniqueAppend $matchWMEs $o]
            set matchTimeTags [lUniqueAppend $matchTimeTags $t]
            lappend WMETree::specialFormat(matchtree) $t
         }
      }
      #puts "Matched WMEs: $matchWMEs"
      #puts "Matched Time Tags: $matchTimeTags"

      $matchTextItem tag configure redText -foreground red
      $matchTextItem tag configure blueText -foreground blue
      $matchTextItem see 1.0
   }
   
   output-strings-destination -pop
   $matchTextItem configure -state disabled
   $prodViewItem configure -state disabled

   #UpdateMatchWMEs
   if {$matchWMEs != {}} {
      WMETree::reloadTree matchtree $matchWMEs
      #WMETree::OpenAllNodesDepth matchtree 1
   } else {
      WMETree::reloadTree matchtree {NONE}
   }
   update

}