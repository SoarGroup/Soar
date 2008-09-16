##########################################################################
# File         : tsiPopUp.tcl
# Author       : Randolph M. Jones
# Date	       : 09/27/96 US, 27/09/96 non-US
# Description 
#
# This file implements a pop-up menu for the soar user
# interface. Pressing the right mouse-button when positioned on part
# of the soar windows causes it to appear.
#
##########################################################################
#
#

### w is the window the pop-up menu will appear in
### agentW is the window the pop-up commands will be sent to (usually
### the agent interaction window)
proc soarPopUp {w {agentW 0}} {
  if {$agentW == 0} {
    set agentW $w
  }
  if [winfo exists $w] {
    bind $w <3> "
      setSoarStringInfo $w @%x,%y
      doPopupMenu $agentW %X %Y
      break
    "
  } else {
    tk_dialog .error {Warning} \
              "Attempted to set up Soar Pop-ups in non-existent window $w" \
              warning 0 Dismiss

  }
}

proc setSoarStringInfo {w i} {
   global soarinfotype soarinfoname soaraugmentationid soaraugmentationvalue
   global soaraugmentationatt
   global termTextPromptValue
   set constituent_chars {A-Za-z0-9$%&*+/:<=>?_-}
   set whitespace " \f\n\r\t\v"
   set soarinfotype {}
   set soarinfoname {}
   if {(![regexp \[$constituent_chars\] [$w get $i]]) && \
       ([$w get $i] != {^}) && ([$w get $i] != {.})} {
      return
   }
   ### If this window doesn't have a prompt, make up something that will
   ### never be the prompt
   if ![info exists termTextPromptValue($w)] {
      set termTextPromptValue($w) {StUpIdArTiFiCiAlPrOmpT>*%#}
   }
   if {[$w get $i] != {^}} {
      for {$w mark set infostart "$i - 1 char"} \
          {[$w compare infostart >= "$i linestart"]} \
          {$w mark set infostart {infostart - 1 char}} {
         if ![regexp \[$constituent_chars\] [$w get infostart]] {
            if {([$w get infostart] != {^}) && ([$w get infostart] != {.})} {
               $w mark set infostart {infostart + 1 char}
            }
            break
         }
         if [$w compare infostart == 1.0] {
            break
         }
      }
      if [$w compare infostart < "$i linestart"] {
         $w mark set infostart "$i linestart"
      }
   } else {
      $w mark set infostart $i
   }
   for {$w mark set infoend "$i + 1 char"} \
       {[$w compare infoend < "$i lineend"]} \
       {$w mark set infoend {infoend + 1 char}} {
      if ![regexp \[$constituent_chars\] [$w get infoend]] {
         break
      }
   }
   set rc [$w get infostart infoend]
   if {[scan $rc {^%s} soarinfoname] == 1} {
      set soarinfotype augmentation
      set soaraugmentationatt $soarinfoname
      ## Find the closest open parenthesis or prompt to the left of the
      ## augmentation
      set paren [$w search -backwards -exact {(} infostart 1.0]
      if {$paren == {}} {
         set prompt [$w search -backwards -exact $termTextPromptValue($w) \
                                                 infostart 1.0]
         if {$prompt == {}} {
            set formstart 1.0
         } else {
            set formstart $prompt
         }
      } else {
         set prompt [$w search -backwards -exact $termTextPromptValue($w) \
                                                 infostart $paren]
         if {$prompt == {}} {
            set formstart $paren
         } else {
            set formstart $prompt
         }
      }
      ## Now look for the first identifier
      set id [$w search -forwards -regexp -count length \
               "\[$whitespace\(\]\[A-Za-z\]\[0-9\]+\[$whitespace\]" \
               $formstart infostart]
      if {$id == {}} {
         set soaraugmentationid {}
      } else {
         set soaraugmentationid [$w get "$id + 1 char" \
                                        "$id + [expr $length - 1] chars"]
         if {$soaraugmentationid == {*}} {
            set soaraugmentationid {}
         }
      }
      ## Now find the value of the augmentation
      set soaraugmentationvalue {}
      for {$w mark set valstart infoend} \
          {[$w compare valstart < end]} \
          {$w mark set valstart {valstart + 1 char}} {
         if ![regexp \[$whitespace\] [$w get valstart]] {
            break
         }
      }
      if [$w compare valstart < end] {
         for {$w mark set valend valstart} \
             {[$w compare valend < end]} \
             {$w mark set valend {valend + 1 char}} {
            if ![regexp \[$constituent_chars\] [$w get valend]] {
               break
            }
         }
         set soaraugmentationvalue [$w get valstart valend]
         if {$soaraugmentationvalue == {*}} {
            set soaraugmentationvalue {}
         }
      }
   } elseif {[scan $rc {.%s} soarinfoname] == 1} {
      set soarinfotype beheadedaugmentation
      set soaraugmentationatt $soarinfoname
      set soaraugmentationid {}
      ## Now find the value of the augmentation
      set soaraugmentationvalue {}
      for {$w mark set valstart infoend} \
          {[$w compare valstart < end]} \
          {$w mark set valstart {valstart + 1 char}} {

         if ![regexp \[$whitespace\] [$w get valstart]] {
            break
         }
      }
      if [$w compare valstart < end] {
         for {$w mark set valend valstart} \
             {[$w compare valend < end]} \
             {$w mark set valend {valend + 1 char}} {
            if ![regexp \[$constituent_chars\] [$w get valend]] {
               break
            }
         }
         set soaraugmentationvalue [$w get valstart valend]
         if {$soaraugmentationvalue == {*}} {
            set soaraugmentationvalue {}
         }
      }
   } elseif {[scan $rc {%d} n] == 1} {
      set soarinfoname $n
      output-strings-destination -push -append-to-result
      set x [catch "wmes $n" res]
      output-strings-destination -pop
      if {!$x && ![scan $res {No wme %s} f1]} {
         set soarinfotype wmenumber
      } else {
         set soarinfotype constant
         ## Find the closest open parenthesis or prompt to the left of the
         ## augmentation
         set paren [$w search -backwards -exact {(} infostart 1.0]
         if {$paren == {}} {
            set prompt [$w search -backwards -exact $termTextPromptValue($w) \
                                                    infostart 1.0]
            if {$prompt == {}} {
               set formstart 1.0
            } else {
               set formstart $prompt
            }
         } else {
            set prompt [$w search -backwards -exact $termTextPromptValue($w) \
                                                    infostart $paren]
            if {$prompt == {}} {
               set formstart $paren
            } else {
               set formstart $prompt
            }
         }
         ## Now look for the first identifier
         set id [$w search -forwards -regexp -count length \
                  "\[$whitespace\(\]\[A-Za-z\]\[0-9\]+\[$whitespace\]" \
                  $formstart infostart]
         if {$id == {}} {
            set soaraugmentationid {}
         } else {
            set soaraugmentationid [$w get "$id + 1 char" \
                                           "$id + [expr $length - 1] chars"]
            if {$soaraugmentationid == {*}} {
               set soaraugmentationid {}
            }
         }
         ## and then search for the nearest augmentation attribute
         set soaraugmentationatt {}
         set attstart [$w search -backwards -regexp {[.^]} infostart $formstart]
         if {$attstart != {}} {
            set attend [$w search -forwards -regexp \[^$constituent_chars\] \
                            "$attstart + 1 chars" infostart]
            if {$attend != {}} {
               set soaraugmentationatt [$w get "$attstart + 1 char" $attend]
               if {$soaraugmentationatt == {*}} {
                  set soaraugmentationatt {}
               }
            }
            ## Beheaded augmentation shouldn't use the ID we found
            if {[$w get $attstart] == {.}} {
               set soaraugmentationid {}
            }
         }
      }
   } elseif {[scan $rc {%1s%d%s} f1 f2 f3] == 2} {
      set soarinfoname $rc
      set soarinfotype identifier
      ## Find the closest open parenthesis or prompt to the left of the
      ## augmentation
      set paren [$w search -backwards -exact {(} infostart 1.0]
      if {$paren == {}} {
         set prompt [$w search -backwards -exact $termTextPromptValue($w) \
                                                 infostart 1.0]
         if {$prompt == {}} {
            set formstart 1.0
         } else {
            set formstart $prompt
         }
      } else {
         set prompt [$w search -backwards -exact $termTextPromptValue($w) \
                                                 infostart $paren]
         if {$prompt == {}} {
            set formstart $paren
         } else {
            set formstart $prompt
         }
      }
      ## Now look for the first identifier
      set id [$w search -forwards -regexp -count length \
               "\[$whitespace\(\]\[A-Za-z\]\[0-9\]+\[$whitespace\]" \
               $formstart infostart]
      if {$id == {}} {
         set soaraugmentationid {}
      } else {
         set soaraugmentationid [$w get "$id + 1 char" \
                                        "$id + [expr $length - 1] chars"]
         if {$soaraugmentationid == {*}} {
            set soaraugmentationid {}
         }
      }
      set soaraugmentationatt {}
      if {$soaraugmentationid == $soarinfoname} {
         set soaraugmentationid {}
      } else {
         ## and then search for the nearest augmentation attribute
         set attstart [$w search -backwards -regexp {[.^]} infostart $formstart]
         if {$attstart != {}} {
            set attend [$w search -forwards -regexp \[^$constituent_chars\] \
                            "$attstart + 1 chars" infostart]
            if {$attend != {}} {
               set soaraugmentationatt [$w get "$attstart + 1 char" $attend]
               if {$soaraugmentationatt == {*}} {
                  set soaraugmentationatt {}
               }
            }
            ## Beheaded augmentation shouldn't use the ID we found
            if {[$w get $attstart] == {.}} {
               set soaraugmentationid {}
            }
         }
      }
   } elseif {([string index $rc 0] == {<}) &&
             ([string index $rc [expr [string length $rc] - 1]] == {>})} {
      set soarinfoname $rc
      set soarinfotype variable
   } else {
      set soarinfoname $rc
      set soarinfotype constant
      # Is this an existing production?
      output-strings-destination -push -append-to-result
      set x [catch "print $rc" res]
      output-strings-destination -pop
      if {!$x && [scan $res {sp %s} foo] == 1} {
         set soarinfotype production
      } else {
         ## Find the closest open parenthesis or prompt to the left of the
         ## augmentation
         set paren [$w search -backwards -exact {(} infostart 1.0]
         if {$paren == {}} {
            set prompt [$w search -backwards -exact $termTextPromptValue($w) \
                                                    infostart 1.0]
            if {$prompt == {}} {
               set formstart 1.0
            } else {
               set formstart $prompt
            }
         } else {
            set prompt [$w search -backwards -exact $termTextPromptValue($w) \
                                                    infostart $paren]
            if {$prompt == {}} {
               set formstart $paren
            } else {
               set formstart $prompt
            }
         }
         ## Now look for the first identifier
         set id [$w search -forwards -regexp -count length \
                  "\[$whitespace\(\]\[A-Za-z\]\[0-9\]+\[$whitespace\]" \
                  $formstart infostart]
         if {$id == {}} {
            set soaraugmentationid {}
         } else {
            set soaraugmentationid [$w get "$id + 1 char" \
                                           "$id + [expr $length - 1] chars"]
            if {$soaraugmentationid == {*}} {
               set soaraugmentationid {}
            }
         }
         ## and then search for the nearest augmentation attribute
         set soaraugmentationatt {}
         set attstart [$w search -backwards -regexp {[.^]} infostart $formstart]
         if {$attstart != {}} {
            set attend [$w search -forwards -regexp \[^$constituent_chars\] \
                            "$attstart + 1 chars" infostart]
            if {$attend != {}} {
               set soaraugmentationatt [$w get "$attstart + 1 char" $attend]
               if {$soaraugmentationatt == {*}} {
                  set soaraugmentationatt {}
               }
            }
            ## Beheaded augmentation shouldn't use the ID we found
            if {[$w get $attstart] == {.}} {
               set soaraugmentationid {}
            }
         }
      }
   }
}

### Need to do:
###  production find (pf)

### Legend:
###   constant: print wmes (need: preferences)
###   production: print matches excise
###      (might be constant AND production)
###   number: print wmes remove-wme
###      (need to distinguish real wme numbers)
###   identifier: preferences print wmes
###      (need to check if identifier is also production)
###      (preferences ^operator if a state wme)
###   augmentation: preferences print wmes
###   beheadedaugmentation: print wmes
###   default: print -stack, matches, run

### w is the window the popup commands get sent to, NOT necessarily the
### window the the popup menu appears in
proc doPopupMenu {w x y} {
   global soarinfotype soarinfoname soaraugmentationid soaraugmentationvalue
   global soaraugmentationatt
   if ![winfo exists .popup] {
      menu .popup  -tearoff 0
      .popup add cascade -label {Print} -menu .popup.print
      .popup add cascade -label {Preferences} -menu .popup.pref
      .popup add cascade -label {WMEs} -menu .popup.wme
      .popup add cascade -label {Production} -menu .popup.prod
#      .popup add separator
#      .popup add cascade -label {Run} -menu .popup.run
      menu .popup.print  -tearoff 0
      menu .popup.pref  -tearoff 0
      menu .popup.wme  -tearoff 0
      menu .popup.prod  -tearoff 0
      menu .popup.run  -tearoff 0
#      .popup.run add command -label Run \
#         -command [list termTextSendCommand $w {run}]
#      .popup.run add command -label Step \
#         -command [list termTextSendCommand $w {run 1}]
#      .popup.run add command -label Stop \
#         -command "termTextSendCommand $w stop-soar"
   }
   # Print sub-menu
   switch $soarinfotype {
      {wmenumber} -
      {constant} {
	 .popup entryconfigure {Print} -state normal
	 .popup.print delete 0 end
         if {$soarinfotype == {wmenumber}} {
	    .popup.print add command -label "print $soarinfoname" \
                -command [list termTextSendCommand $w [list print $soarinfoname]]
         }
         if {($soaraugmentationid != {}) && ($soaraugmentationatt != {})} {
            .popup.print add command -label [list print \
              ($soaraugmentationid ^$soaraugmentationatt $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list print \
                      [list ($soaraugmentationid ^$soaraugmentationatt \
                            $soarinfoname)]]]
         }
         if {$soaraugmentationid != {}} {
            .popup.print add command -label [list print \
              ($soaraugmentationid ^* $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list print \
                      [list ($soaraugmentationid ^* $soarinfoname)]]]
         }
         if {$soaraugmentationatt != {}} {
            .popup.print add command -label [list print \
              (* ^$soaraugmentationatt $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list print \
                      [list (* ^$soaraugmentationatt $soarinfoname)]]]
         }
         .popup.print add command -label [list print (* ^* $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list print [list (* ^* $soarinfoname)]]]
	 .popup.print add command -label {print -stack} \
             -command [list termTextSendCommand $w [list print -stack]]
	 .popup entryconfigure {Preferences} -state disabled
	 .popup entryconfigure {WMEs} -state normal
	 .popup.wme delete 0 end
         if {$soarinfotype == {wmenumber}} {
	    .popup.wme add command -label "wmes $soarinfoname" \
                -command [list termTextSendCommand $w [list wmes $soarinfoname]]
         }
         if {($soaraugmentationid != {}) && ($soaraugmentationatt != {})} {
            .popup.wme add command -label [list wmes \
              ($soaraugmentationid ^$soaraugmentationatt $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list wmes \
                      [list ($soaraugmentationid ^$soaraugmentationatt \
                            $soarinfoname)]]]
         }
         if {$soaraugmentationid != {}} {
            .popup.wme add command -label [list wmes \
              ($soaraugmentationid ^* $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list wmes \
                      [list ($soaraugmentationid ^* $soarinfoname)]]]
         }
         if {$soaraugmentationatt != {}} {
            .popup.wme add command -label [list wmes \
              (* ^$soaraugmentationatt $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list wmes \
                      [list (* ^$soaraugmentationatt $soarinfoname)]]]
         }
         .popup.wme add command -label [list wmes (* ^* $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list wmes [list (* ^* $soarinfoname)]]]
         if {$soarinfotype == {wmenumber}} {
	    .popup.wme add command -label "remove-wme $soarinfoname" \
                -command [list termTextSendCommand $w \
                               [list remove-wmewme $soarinfoname]]
         }
	 .popup entryconfigure {Production} -state normal
	 .popup.prod delete 0 end
	 .popup.prod add command -label {matches} \
             -command "termTextSendCommand $w matches"
     }
     {production} {
	 .popup entryconfigure {Print} -state normal
	 .popup.print delete 0 end
	 .popup.print add command -label "print $soarinfoname" \
             -command [list termTextSendCommand $w [list print $soarinfoname]]
	 .popup.print add command -label {print -stack} \
             -command [list termTextSendCommand $w [list print -stack]]
	 .popup entryconfigure {Preferences} -state disabled
	 .popup entryconfigure {WMEs} -state disabled
	 .popup entryconfigure {Production} -state normal
	 .popup.prod delete 0 end
	 .popup.prod add command -label {matches} \
             -command "termTextSendCommand $w matches"
	 .popup.prod add command -label "matches $soarinfoname" \
             -command [list termTextSendCommand $w [list matches $soarinfoname]]
	 .popup.prod add command -label "matches $soarinfoname -timetags" \
             -command [list termTextSendCommand $w \
                            [list matches $soarinfoname -timetags]]
	 .popup.prod add command -label "matches $soarinfoname -wmes" \
             -command [list termTextSendCommand $w \
                            [list matches $soarinfoname -wmes]]
	 .popup.prod add command -label "excise $soarinfoname" \
             -command [list termTextSendCommand $w [list excise $soarinfoname]]
     }
     {identifier} {
	 .popup entryconfigure {Print} -state normal
	 .popup.print delete 0 end
	 .popup.print add command -label "print $soarinfoname" \
             -command [list termTextSendCommand $w [list print $soarinfoname]]
	 .popup.print add command -label "print -depth ... $soarinfoname" \
             -command "termTextRewriteUserText $w; \
                       $w delete {promptEnd + 1 chars} end; \
                       [list termTextInsert $w \
                             "print -depth  $soarinfoname"]; \
                       $w mark set insert {promptEnd + 14 chars}"
         if {($soaraugmentationid != {}) && ($soaraugmentationatt != {})} {
            .popup.print add command -label [list print \
              ($soaraugmentationid ^$soaraugmentationatt $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list print \
                      [list ($soaraugmentationid ^$soaraugmentationatt \
                            $soarinfoname)]]]
         }
         if {$soaraugmentationid != {}} {
            .popup.print add command -label [list print \
              ($soaraugmentationid ^* $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list print \
                      [list ($soaraugmentationid ^* $soarinfoname)]]]
         }
         if {$soaraugmentationatt != {}} {
            .popup.print add command -label [list print \
              (* ^$soaraugmentationatt $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list print \
                      [list (* ^$soaraugmentationatt $soarinfoname)]]]
         }
         .popup.print add command -label [list print (* ^* $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list print [list (* ^* $soarinfoname)]]]
	 .popup.print add command -label {print -stack} \
             -command [list termTextSendCommand $w [list print -stack]]
	 .popup entryconfigure {Preferences} -state normal
	 .popup.pref delete 0 end
	 .popup.pref add command -label "preferences $soarinfoname ..." \
               -command "termTextRewriteUserText $w; \
                         $w delete {promptEnd + 1 chars} end; \
                         [list termTextInsert $w \
                               "preferences $soarinfoname "]"
	 .popup.pref add command -label "preferences $soarinfoname ... -names" \
               -command "termTextRewriteUserText $w; \
                         $w delete {promptEnd + 1 chars} end; \
                         [list termTextInsert $w \
                               "preferences $soarinfoname  -names"]; \
                         $w mark set insert {insert - 7 chars}"
	 .popup.pref add command -label \
                                 "preferences $soarinfoname ... -timetags" \
               -command "termTextRewriteUserText $w; \
                         $w delete {promptEnd + 1 chars} end; \
                         [list termTextInsert $w \
                               "preferences $soarinfoname  -timetags"]; \
                         $w mark set insert {insert - 10 chars}"
	 .popup.pref add command -label "preferences $soarinfoname ... -wmes" \
               -command "termTextRewriteUserText $w; \
                         $w delete {promptEnd + 1 chars} end; \
                         [list termTextInsert $w \
                               "preferences $soarinfoname  -wmes"]; \
                         $w mark set insert {insert - 6 chars}"
	 .popup entryconfigure {WMEs} -state normal
	 .popup.wme delete 0 end
	 .popup.wme add command -label "wmes $soarinfoname" \
             -command [list termTextSendCommand $w [list wmes $soarinfoname]]
         if {($soaraugmentationid != {}) && ($soaraugmentationatt != {})} {
            .popup.wme add command -label [list wmes \
              ($soaraugmentationid ^$soaraugmentationatt $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list wmes \
                      [list ($soaraugmentationid ^$soaraugmentationatt \
                            $soarinfoname)]]]
         }
         if {$soaraugmentationid != {}} {
            .popup.wme add command -label [list wmes \
              ($soaraugmentationid ^* $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list wmes \
                      [list ($soaraugmentationid ^* $soarinfoname)]]]
         }
         if {$soaraugmentationatt != {}} {
            .popup.wme add command -label [list wmes \
              (* ^$soaraugmentationatt $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list wmes \
                      [list (* ^$soaraugmentationatt $soarinfoname)]]]
         }
         .popup.wme add command -label [list wmes (* ^* $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list wmes [list (* ^* $soarinfoname)]]]
	 .popup entryconfigure {Production} -state normal
	 .popup.prod delete 0 end
	 .popup.prod add command -label {matches} \
             -command "termTextSendCommand $w matches"
      }
     {augmentation} {
	 .popup entryconfigure {Print} -state normal
	 .popup.print delete 0 end
         if {($soaraugmentationid != {}) && ($soaraugmentationvalue != {})} {
            .popup.print add command -label [list print \
              ($soaraugmentationid ^$soarinfoname $soaraugmentationvalue)] \
              -command [list termTextSendCommand $w \
                     [list print \
                      [list ($soaraugmentationid ^$soarinfoname \
                             $soaraugmentationvalue)]]]
         }
         if {$soaraugmentationid != {}} {
            .popup.print add command -label [list print \
              ($soaraugmentationid ^$soarinfoname *)] \
              -command [list termTextSendCommand $w \
                     [list print \
                      [list ($soaraugmentationid ^$soarinfoname *)]]]
         }
         if {$soaraugmentationvalue != {}} {
            .popup.print add command -label [list print \
              (* ^$soarinfoname $soaraugmentationvalue)] \
              -command [list termTextSendCommand $w \
                     [list print \
                      [list (* ^$soarinfoname $soaraugmentationvalue)]]]
         }
         .popup.print add command -label [list print (* ^$soarinfoname *)] \
              -command [list termTextSendCommand $w \
                     [list print [list (* ^$soarinfoname *)]]]
	 .popup.print add command -label {print -stack} \
             -command [list termTextSendCommand $w [list print -stack]]
	 .popup entryconfigure {Preferences} -state normal
	 .popup.pref delete 0 end
	 .popup.pref add command -label \
                  "preferences $soaraugmentationid $soarinfoname" \
             -command [list termTextSendCommand $w \
                 [list preferences $soaraugmentationid $soarinfoname]]
	 .popup.pref add command -label \
                  "preferences $soaraugmentationid $soarinfoname -names" \
             -command [list termTextSendCommand $w \
                 [list preferences $soaraugmentationid $soarinfoname -names]]
	 .popup.pref add command -label \
                  "preferences $soaraugmentationid $soarinfoname -timetags" \
             -command [list termTextSendCommand $w \
                 [list preferences $soaraugmentationid $soarinfoname -timetags]]
	 .popup.pref add command -label \
                  "preferences $soaraugmentationid $soarinfoname -wmes" \
             -command [list termTextSendCommand $w \
                 [list preferences $soaraugmentationid $soarinfoname -wmes]]
	 .popup entryconfigure {WMEs} -state normal
	 .popup.wme delete 0 end
         if {($soaraugmentationid != {}) && ($soaraugmentationvalue != {})} {
            .popup.wme add command -label [list wmes \
              ($soaraugmentationid ^$soarinfoname $soaraugmentationvalue)] \
              -command [list termTextSendCommand $w \
                     [list wmes \
                      [list ($soaraugmentationid ^$soarinfoname \
                             $soaraugmentationvalue)]]]
         }
         if {$soaraugmentationid != {}} {
            .popup.wme add command -label [list wmes \
              ($soaraugmentationid ^$soarinfoname *)] \
              -command [list termTextSendCommand $w \
                     [list wmes \
                      [list ($soaraugmentationid ^$soarinfoname *)]]]
         }
         if {$soaraugmentationvalue != {}} {
            .popup.wme add command -label [list wmes \
              (* ^$soarinfoname $soaraugmentationvalue)] \
              -command [list termTextSendCommand $w \
                     [list wmes \
                      [list (* ^$soarinfoname $soaraugmentationvalue)]]]
         }
         .popup.wme add command -label [list wmes (* ^$soarinfoname *)] \
              -command [list termTextSendCommand $w \
                     [list wmes [list (* ^$soarinfoname *)]]]
	 .popup entryconfigure {Production} -state normal
	 .popup.prod delete 0 end
	 .popup.prod add command -label {matches} \
             -command "termTextSendCommand $w matches"
      }
      {beheadedaugmentation} {
	 .popup entryconfigure {Print} -state normal
	 .popup.print delete 0 end
         if {$soaraugmentationvalue != {}} {
            .popup.print add command -label [list print \
              (* ^$soarinfoname $soaraugmentationvalue)] \
              -command [list termTextSendCommand $w \
                     [list print \
                      [list (* ^$soarinfoname $soaraugmentationvalue)]]]
         }
         .popup.print add command -label [list print (* ^* $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list print [list (* ^$soarinfoname *)]]]
	 .popup.print add command -label {print -stack} \
             -command [list termTextSendCommand $w [list print -stack]]
	 .popup entryconfigure {Preferences} -state disabled
         if {$soaraugmentationvalue != {}} {
            .popup.wme add command -label [list wmes \
              (* ^$soarinfoname $soaraugmentationvalue)] \
              -command [list termTextSendCommand $w \
                     [list wmes \
                      [list (* ^$soarinfoname $soaraugmentationvalue)]]]
         }
         .popup.wme add command -label [list wmes (* ^* $soarinfoname)] \
              -command [list termTextSendCommand $w \
                     [list wmes [list (* ^$soarinfoname *)]]]
	 .popup entryconfigure {Production} -state normal
	 .popup.prod delete 0 end
	 .popup.prod add command -label {matches} \
             -command "termTextSendCommand $w matches"
      }
      {default} {
	 .popup entryconfigure {Print} -state normal
	 .popup.print delete 0 end
	 .popup.print add command -label {print -stack} \
             -command [list termTextSendCommand $w [list print -stack]]
	 .popup entryconfigure {Preferences} -state disabled
	 .popup entryconfigure {WMEs} -state disabled
	 .popup entryconfigure {Production} -state normal
	 .popup.prod delete 0 end
	 .popup.prod add command -label {matches} \
             -command "termTextSendCommand $w matches"
      }
   }
   update
   tk_popup .popup $x $y
}

