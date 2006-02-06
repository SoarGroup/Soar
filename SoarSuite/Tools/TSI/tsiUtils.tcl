##########################################################################
# File         : tsiUtils.tcl
# Author       : Gordon D. Baxter, Frank E. Ritter plus others.
# Date	       : 16/2/97
# Description  : 
#
#
# Purpose
#
# This is a start of a graphic display for Soar7.
# It contains a number of procedures to set up Soar-specific displays, in 
# particular to monitor the rules that can fire, and the current contents
# of the goal stack.  It also allows for the printing of objects into a
# dedicated window.
#
# TABLE OF CONTENTS
#
#	i.	How to use
# 	i.	TODO
#	ii.	Global variables
#	I.	mkMonitor
#	II.	Monitor helper commands, killWindow, etc.
#	III.	Actually set up the monitoring windows
#	IV.	Protect init-soar from missing window
#	V.	Bind keys
#	VI.	Functions for keybindings
#	VII.	Create the menus
#	VIII.	True utility functions
#	IX.	Force an operator selection
#	X.	Changes to other functions
#	XI.	Some useful aliases
#
##########################################################################
#
# 



###
###	i.	How to use
### 

# You can explicitly iconify windows using
# 
# wm iconify name-of-window
# wm deiconify name-of-window



###
### 	i.	TODO
###

## * Advanced task: click on pgs elements and open examination window
## * needs to come up in a fixed and better place
## * some would like a continous listing, so you can scroll back.
##   I suspect that doing that will lead to problems
## * Fix setPrintDepth to use real Soar variables



###
###	ii.	Global variables
###

### Arrays to hold monitor information.

global matchSet
global printStack
global opreferencesMonitor
global tsiConfig

#
# Note: It may be useful to have the match set display update when a rule
# is removed too.  The monitor production-just-about-to-be-excised does
# not give you the right result, and there is no production-just-excised
# monitor available.
#


set matchSet(event1) after-output-phase
set matchSet(event2) after-init-soar
set matchSet(event3) wm-changes
set matchSet(event4) after-decision-cycle
set matchSet(event5) production-just-added
set matchSet(length) 5

set printStack(event1) after-decision-cycle
set printStack(event2) after-init-soar
set printStack(length) 2

set opreferencesMonitor(event1) after-decision-cycle
set opreferencesMonitor(event2) after-init-soar
set opreferencesMonitor(event3) after-output-phase
set opreferencesMonitor(length) 3



###
###	I.	mkMonitor
###

## this function creates a window W name WNAME for displaying 
## a command C at WHEN (which is a monitor location).

proc mkMonitorWin {w monitorName {xsize 70} {ysize 10}} {
    if {[winfo exists $w] == 0} {

      toplevel $w
      wm title $w "View $monitorName"
      wm iconname $w "View $monitorName"
      wm minsize $w 1 1

      # add a menubar

      #frame $w.menubar -relief raised -bd 2
      #pack $w.menubar  -fill x

      #addTSIFileMenu $w
      #addTSIShowMenu $w
      #addTSIMemoryMenu $w
      #addTSIWatchMenu $w
      #addTSIViewMenu $w
      #addTSICommandsMenu $w
      #addTSIDemoMenu $w
      #addTSIHelpMenu $w

      # stick on a scrolling text region

      makeScrollableText $w $xsize $ysize

      # bind keys using "default" settings

      tsiBindKeys $w

      # bind mouse-buttons too

      bind $w <Double-Button-2> \
              [list printObject .printObjectWindow {Print Object}]
  }
}

proc printObject {winName winString} {
    if ![winfo exists $winName] {
      toplevel $winName
      wm title $winName $winString
      wm iconname $winName $winString

      # add a menubar

      frame $winName.menubar -relief raised -bd 2
      pack $winName.menubar  -fill x

      # add an options menu to support:
      # - Dismissal of window (Shortcut Alt-D)
      # - Quit Soar      (Shortcut Alt-X)

      menubutton $winName.menubar.options -text Options \
                       -menu $winName.menubar.options.menu
      pack $winName.menubar.options -side left

      menu $winName.menubar.options.menu -tearoff 0
      $winName.menubar.options.menu add command -label {Clear Window} \
                              -command [list clearWindow $winName]
      $winName.menubar.options.menu add command -label {Set Print Depth} \
                              -command {setPrintDepth}
      $winName.menubar.options.menu add command -label Dismiss \
                              -command [list destroy $winName]
      $winName.menubar.options.menu add separator
      $winName.menubar.options.menu add command -label {Quit Soar} \
                              -command {quitSoar}

      # add the standard Soar Commands menu
#      addTSICommandsMenu $winName

      # add a help menu on the right
      addTSIHelpMenu $winName

      # stick on a scrolling text
      makeScrollableText $winName 80 20

      # use the "default" binding of keys
      tsiBindKeys $winName

      # bind mouse button 2 to allow recursion
      bind $winName <Double-Button-2> \
                    [list printObject $winName {Print Object}]

      # Set up the Pop-Up command menu
       soarPopUp $winName.text
    }

    ## Could print an error message on null selection,
    ## but probably too annonying
    if {![catch {selection get} sresult]} {
      output-strings-destination -push -append-to-result
      $winName.text configure -state normal
      $winName.text insert end [print [string trim [selection get]]]
      $winName.text see end
      $winName.text configure -state disabled
      selection clear
      output-strings-destination -pop
      update idletasks}
}


###
###	II.	Monitor helper commands, killWindow, etc.
###

# Procedure to destroy window WIN and remove the actions associated
# with the Soar monitor MON associated with that window.  It also
# unsets any global variable that may be associated with the window,
# thus allowing a new one to be created.

proc killWindow {win {mon 0}} {

  global matchSet printStack opreferencesMonitor monitorWindowPairs
  global monitor

  if {$mon == 0 && [info exists monitorWindowPairs($win)] } {
      set mon $monitorWindowPairs($win)
  }
  if {[info exists monitorWindowPairs($win)] } {
      unset monitorWindowPairs($win)}
  if ([winfo exists $win]) {destroy $win}

  if {[string compare $mon {matchSet}] == 0} {
    set events $matchSet(length)
    for {set i 1} {$i <= $events} {incr i} {
      monitor -delete $matchSet(event$i) $matchSet(id$i)}
      set monitor(matches) 0
  } elseif {[string compare $mon {printStack}] == 0} {
    set events $printStack(length)
    for {set i 1} {$i <= $events} {incr i} {
      set monitor(print-stack) 0
      monitor -delete $printStack(event$i) $printStack(id$i)}
  } elseif {[string compare $mon {opreferencesMonitor}] == 0} {
    set events $opreferencesMonitor(length)
    for {set i 1} {$i <= $events} {incr i} {
      set monitor(opreferences) 0
      monitor -delete $opreferencesMonitor(event$i) $opreferencesMonitor(id$i)}
     }   
}


# proc clearWindow - clears the text from a window
# Simply deletes the text from start to end, and resets the text widget
# such that the top o fthe text is displayed

proc clearWindow {win} {

  $win.text configure -state normal
  $win.text delete 1.0 end
  $win.text see end
  $win.text configure -state disabled
}

#
# procedure to put the results of command into window w
# Any existing text is deleted first, and the window always scrolls to
# show the last line of the text.
#

proc ShowInMonitor {w command} {
    output-strings-destination -push -append-to-result
    $w.text configure -state normal
    $w.text delete 1.0 end
    $w.text insert end [eval $command]
    $w.text see end
    $w.text configure -state disabled
    output-strings-destination -pop
    update
}

## this updates all the displays (both of them, currently)
proc UpdateDisplays {} {
    
    if {[winfo exists .ms] != 0} {
      ShowInMonitor .ms {matches}}
    if {[winfo exists .pgs] != 0} {
      ShowInMonitor .pgs {print -stack}}
    if {[winfo exists .opreferences] != 0} {
      ShowInMonitor .opreferences {preferences}}

    update idletasks
}

###
###	III.	Actually set up the monitoring windows
###

global monitorWindowPairs

## This sets up a matches monitor if none exists

proc makeMatchesMonitor {agentW {x 0} {y 315}} {
  global matchSet matchSetFrame
  global monitorWindowPairs tcl_platform
  set matchSetFrame .ms

  if {[winfo exists $matchSetFrame] == 0} {

    mkMonitorWin $matchSetFrame matchSet
    wm geometry $matchSetFrame +$x+$y

    ### I am uncomfortable to have these hard and fast number sprinkled
    ### throughout the code.  They should eventually be user definable
    ### or at least determined from true window dimensions rather than
    ### machine type.  RMJ
    ## On the Mac, make it smaller.
    if {$tcl_platform(platform) != {unix}} { wm geometry .ms  68x8}

    # Add Soar monitors:
    for {set i 1} {$i <= $matchSet(length)} {incr i} {
      set matchSet(id$i) [monitor -add $matchSet(event$i) \
	      [list ShowInMonitor $matchSetFrame {matches}]]}
    set monitorWindowPairs($matchSetFrame) matchSet
     soarPopUp $matchSetFrame.text $agentW

  }
  ShowInMonitor $matchSetFrame {matches}
  update idletasks
}

## This sets up a pgs printer

proc makePrintGoalStackMonitor {agentW {x 0} {y 30}} {
  global printStack printStackFrame
  global monitorWindowPairs tcl_platform
  set printStackFrame .pgs

  if {[winfo exists $printStackFrame] == 0} {

    mkMonitorWin $printStackFrame printStack
    ## set where it appears on the screen
    wm geometr $printStackFrame +$x+$y

    ## On the Mac, make it smaller.
    if {$tcl_platform(platform) != {unix}} { wm geometr .pgs  68x8}

    # Set up Soar monitors
    for {set i 1} {$i <= $printStack(length)} {incr i} {
      set printStack(id$i) \
          [monitor -add $printStack(event$i) \
                   [list ShowInMonitor $printStackFrame \
                         {output-strings-destination -push -append-to-result
                          set cnt [stats -system -dc-count]
                          set trace [print -stack]
                          output-strings-destination -pop
                          format "%s\n%s" $cnt $trace}]]
    }

    set monitorWindowPairs($printStackFrame) printStack
    soarPopUp $printStackFrame.text $agentW
  }
  ShowInMonitor $printStackFrame {stats -system -dc-count; print -stack}
  update idletasks
}

## This sets up a preferences printer

proc makeOPreferencesMonitor {agentW {x 0} {y 130}} {
  global opreferencesMonitor opreferencesFrame
  global monitorWindowPairs tcl_platform
  set opreferencesFrame .opreferences

  if {[winfo exists $opreferencesFrame] == 0} {

    mkMonitorWin $opreferencesFrame opreferencesMonitor 40 10
    wm geometr $opreferencesFrame +$x+$y
    
    ## On the Mac, make it smaller.
    #if {$tcl_platform(platform) != {unix}} { wm geometr .pgs  68x8}

    # Set up Soar monitors
    for {set i 1} {$i <= $opreferencesMonitor(length)} {incr i} {
      set opreferencesMonitor(id$i) \
          [monitor -add $opreferencesMonitor(event$i) \
	      [list ShowInMonitor $opreferencesFrame {preferences}]]}
    set monitorWindowPairs($opreferencesFrame) opreferencesMonitor
    soarPopUp $opreferencesFrame.text $agentW
  }
  update idletasks
}

###
###	IV.	Protect init-soar from missing window
###
# If any of the monitor windows don't exist but their monitors do, 
# the user has deleted them with the window system, so clearn up after them.
proc cleanupMonitorWindows {} {
  global monitor tsiConfig
  if $tsiConfig(debug) {puts {starting check Monitor Windows}}
  if {$monitor(print-stack) && ![winfo exists .pgs]} { \
          killWindow .pgs}
  if {$monitor(matches) && ![winfo exists .ms]} { \
          killWindow .ms}
  if {$monitor(opreferences) && ![winfo exists .opreferences]} { \
          killWindow .opreferences}
}

###
###	IV.	Bind keys
###

proc tsiBindKeys {win} {
   
  # We don't want to override the environment step/run, etc.  So disable

return

  # bind keys for all monitor windows

  bind $win <space> { tsiDisplayAndSendCommand {d 1}  }
  bind $win <r> { tsiDisplayAndSendCommand {run 1} }
  bind $win <e> { tsiDisplayAndSendCommand {run 1 e} }
  bind $win <i> { tsiDisplayAndSendCommand {init-soar} }
  bind $win <p> [list printObject .printObjectWindow {Print Object}]
  bind $win <question> [list UpdateDisplays]

  # bind numeric keys too

  bind $win 1 { tsiDisplayAndSendCommand {d 1} }
  bind $win 2 { tsiDisplayAndSendCommand {d 2} }
  bind $win 3 { tsiDisplayAndSendCommand {d 3} }
  bind $win 4 { tsiDisplayAndSendCommand {d 4} }
  bind $win 5 { tsiDisplayAndSendCommand {d 5} }
  bind $win 6 { tsiDisplayAndSendCommand {d 6} }
  bind $win 7 { tsiDisplayAndSendCommand {d 7} }
  bind $win 8 { tsiDisplayAndSendCommand {d 8} }
  bind $win 9 { tsiDisplayAndSendCommand {d 9} }

  # bind Ctrl-c too, so it works anywhere

  bind $win <Control-c> [list stop-soar]
}



###
###	V.	Functions for keybindings
###

proc makeScrollableText {win width height {colour lightgrey}} {
  global tsiConfig
  text $win.text -font $tsiConfig(normalFont) \
                 -bg $colour \
                 -setgrid true -width $width -height $height -bd 2 \
                 -yscrollcommand [list $win.scroll set]
  scrollbar $win.scroll -command [list $win.text yview]

  pack $win.scroll -side right -fill y
  pack $win.text -side left -expand true -fill both
}

proc setPrintDepth {} {

  # needs to generate a dialog box which shows the current default print
  # depth and lets the user change it

  set defaultPrintDepth 1

  set pr {Enter new default print depth value}
  set defaultPrintDepth [getOneValueDialog $pr $defaultPrintDepth]
}

#
# proc quitSoar - confirms that quit is what is really required
#

proc quitSoar {} {
   global tsiConfig sio_socketList sio_envSocketList
   global localAgents _agent _kernel localAgentPtrs
   
   if { [grab current] == "" } {   # fix for bugzilla bug 396
     if { ![tk_dialog .tsiQuit {Quit Soar} {Really quit from Soar?} \
	     question 0 {Ok} {Cancel}] } {
      
       if ![string compare $tsiConfig(mode) off] {
	   
	   ## explicitly shutdown the kernel to warn clients,
	   ## (eg SoarJavaDebugger) so they can clean up (eg shut themselves down) 
	   $_kernel Shutdown
	   
	   #delete kernel object (this will also delete any agents that are
	   #  still around)
	   set result [$_kernel -delete];
	   #don't leave bad pointers around
	   if {[info exists localAgentPtrs]}  {unset localAgentPtrs};
	   if {[info exists _agent]}  {unset _agent};
	   if {[info exists _kernel]} {unset _kernel};
	       
	   exit
	   }

       # Otherwise socketio is on, and we may need to disconnect some sockets
       if [info exists sio_socketList] {
	  puts "Disconnecting sio_socketList : $sio_socketList"
	  foreach sock $sio_socketList {
	     SIO_SendSocketCommand $sock "socket-shutdown"
	  }
       } else {
	  puts "sio_socketList is not-defined."
       }
       if [info exists sio_envSocketList] { 
	  puts "Disconnecting sio_envSocketList : $sio_envSocketList"
	  foreach sock $sio_envSocketList {
	     SIO_SendSocketCommand $sock "socket-shutdown"
	  }
       } else {
	  puts "sio_envSocketList is not-defined."
       }


       # Check to see if this is defined, and if so, call it.
       if { [llength [info procs simulatorQuit]] > 0 } {
 	   simulatorQuit ;  #this just closes files
       }
	   ## best to delete agents explicitly in case other
	   ## processes are listening for them, eg SoarJavaDebugger
       if [info exists localAgents] {
	   foreach name $localAgents {
	       destroyAgent $name
	   }
       }

       #delete kernel object (this will also delete any agents that are
       #  still around)
       set result [$_kernel -delete];
       #don't leave bad pointers around
       if {[info exists localAgentPtrs]}  {unset localAgentPtrs};
       if [info exists _agent]  {unset _agent};
       if [info exists _kernel] {unset _kernel};

       exit
      }
   }
}



###
### 	proc TSIHelp - shows the key bindings in a window
###

proc TSIHelp {} {
   global tsiConfig 
   tsiInfoWindow .tsiHelp {
Several keys are bound in most windows.  In addition, you can
type 'p' on selected items in monitor windows.

space-bar : run 1 decision cycle 
        1-9 : run 1-9 decision cycle(s)

       r : run 1 elaboration cycle
       e : run 1 elaboration cycle
       i : init-soar
       p : print selected object
       ? : flush display buffers

       p : print a selected item on a monitor

       control-c: stop soar
       
MOUSE BUTTONS:

left : double-click = select       
middle : double-click = print selection
} {TSI Help} $tsiConfig(helpFont)
}

proc showAbout {} {
  global tsiConfig

  tsiInfoWindow .tsiAbout " 
The TsDebugger, based on the original TSI, Tcl Soar Interfiace,
TSDebugger ver. $tsiConfig(ControlPanelVersion), expertise: $tsiConfig(expertise), debug: $tsiConfig(debug)
Major contributors (in alphabetical order):
    Mazin Assanie
    Gordon D. Baxter
    Karen Coulter
    Randolph M. Jones
    Douglas J. Pearson
    Frank E. Ritter
    Karl B. Schwamb
    Glenn Taylor
    Scott Wallace
   

For help, see the help menu.

" {About the TSI} $tsiConfig(helpFont)
}


###
###	VI.	Create the menus
###

set menusourcedir [pwd]
proc sourceSoarFile {} {
   global menusourcedir
   set olddir [pwd]
   tsiDisplayAndSendCommand "cd \"$menusourcedir\""
   set file [tk_getOpenFile -title {Source ...}]
   if {$file != {}} {
	if [file isfile $file] {
           set menusourcedir [file dirname $file]
           set filename [file tail $file]
	   tsiDisplayAndSendCommand "cd \"$menusourcedir\""
  	   tsiDisplayAndSendCommand "source \"$filename\""
        } else {
	   echo "invalid file"
        }
   }
   tsiDisplayAndSendCommand "cd \"$olddir\""
}
proc saveReteNet {} {
   set file [tk_getSaveFile -title {Save rete-net to...}]
   if {$file != {}} {
      tsiDisplayAndSendCommand "rete-net --save \"$file\""
      }
}
proc openLogFile {} {
   set file [tk_getSaveFile -title {Log output to file...}]
   if {$file != {}} {
      tsiDisplayAndSendCommand "log \"$file\""
      }
}

proc appendLogFile  {} {
   set file [tk_getSaveFile -title {Append output to file...}]
   if {$file != {}} {
      tsiDisplayAndSendCommand "log --existing \"$file\""
      }
}

proc loadReteNet  {} {
   set file [tk_getOpenFile -title {Load rete-net from...}]
   if {$file != {}} {
      tsiDisplayAndSendCommand "rete-net --load \"$file\""
      }
}
proc addTSIFileMenu {w} {
  global tsiConfig tsiSuppressMenus
  
  if $tsiConfig(debug) {puts "\n>>> starting making file menu"}
  if {[info exists tsiSuppressMenus(file)]} {
      return
  }

  if [winfo exists $w.menubar.file.m] {
    destroy $w.menubar.file.m
  } else {
    menubutton $w.menubar.file -text {File} -menu $w.menubar.file.m
  }

  # Menu 
  menu $w.menubar.file.m -tearoff 0
   
  set menuSuppressionCount 0
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(file,$menuSuppressionCount)]} {
     $w.menubar.file.m add command -label {About the TSI} \
         -command [list showAbout]
   }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(file,$menuSuppressionCount)]} {
      $w.menubar.file.m add command -label {Source ...} \
          -command {set file [tk_getOpenFile -title {Source ...}]; \
                if {$file != {}} {tsiDisplayAndSendCommand "source \"$file\""}}
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(file,$menuSuppressionCount)]} {
    $w.menubar.file.m add separator
      $w.menubar.file.m add command -label {Save Production Memory ...} \
        -command {set file [tk_getSaveFile -title {Save Rete-Net To...}]
                  if {$file != {}} {
                     tsiDisplayAndSendCommand "rete-net --save \"$file\""}}
   }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(file,$menuSuppressionCount)]} {
      $w.menubar.file.m add command -label {Load Production Memory ...} \
        -command {set file [tk_getOpenFile -title {Load Rete-Net From...}]
                  if {$file != {}} {
                     tsiDisplayAndSendCommand "rete-net --load \"$file\""}}
  }

  $w.menubar.file.m add separator
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(file,$menuSuppressionCount)]} {
     $w.menubar.file.m add command -label {Log To New File ...} \
         -command {set file [tk_getSaveFile -title {New Log File}]
                if {$file != {}} {tsiDisplayAndSendCommand "log \"$file\""}}
   }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(file,$menuSuppressionCount)]} {
     $w.menubar.file.m add command -label {Log To Existing File ...} \
         -command {set file [tk_getOpenFile -title {Log File}]
                if {$file != {}} {
                   tsiDisplayAndSendCommand "log --existing \"$file\""}}
   }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(file,$menuSuppressionCount)]} {
     $w.menubar.file.m add command -label {Turn Logging Off} \
         -command {tsiDisplayAndSendCommand {log --off}}
   }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(file,$menuSuppressionCount)]} {
   $w.menubar.file.m add command -label {Query Logging Status} \
         -command {tsiDisplayAndSendCommand {log --query}}
   }
  $w.menubar.file.m add separator
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(file,$menuSuppressionCount)]} {
     $w.menubar.file.m add command -label {Quit Soar} -command {quitSoar}
  }

  # Pack menu in menubar frame.

  pack $w.menubar.file -side left -padx 5
}

proc addTSIShowMenu {w} {
  global tsiConfig tsiSuppressMenus

  if $tsiConfig(debug) {puts "\n>>> starting making show menu"}
  if {[info exists tsiSuppressMenus(show)]} {
      return
  }

  if [winfo exists $w.menubar.show.m] {
    destroy $w.menubar.show.m
  } else {
    menubutton $w.menubar.show -text {Show} -menu $w.menubar.show.m
  }

  menu $w.menubar.show.m  -tearoff 0
  set menuSuppressionCount 0
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(show,$menuSuppressionCount)]} {
     $w.menubar.show.m add command -label {Print Stack} \
                           -command {tsiDisplayAndSendCommand {print --stack}}
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(show,$menuSuppressionCount)]} {
    $w.menubar.show.m add command -label {Print Stack Operators} \
          -command {tsiDisplayAndSendCommand {print --stack --operators}}
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(show,$menuSuppressionCount)]} {
  $w.menubar.show.m add command -label {Match Set} \
             -command {tsiDisplayAndSendCommand {matches}}
  }
  $w.menubar.show.m add separator
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(show,$menuSuppressionCount)]} {
     $w.menubar.show.m add cascade -label {List} -menu $w.menubar.show.m.list

	set subMenuCount 0
      # List sub-menu
        menu $w.menubar.show.m.list -tearoff 0

      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
        $w.menubar.show.m.list add command -label {Agents} \
          -command {tsiDisplayAndSendCommand {tsiListAgents}}
        $w.menubar.show.m.list add separator
      }
 
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
        $w.menubar.show.m.list add command -label {Chunks} \
           -command {tsiDisplayAndSendCommand {print --chunks}}
      }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
        $w.menubar.show.m.list add command -label {Default productions} \
           -command {tsiDisplayAndSendCommand {print --defaults}}
      }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {          
        $w.menubar.show.m.list add command -label {User productions} \
          -command {tsiDisplayAndSendCommand {print --user}}
      }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {           
        $w.menubar.show.m.list add command -label {All productions} \
          -command {tsiDisplayAndSendCommand {print --all}}
      }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
     
        $w.menubar.show.m.list add command -label {Justifications} \
           -command {tsiDisplayAndSendCommand {print --justifications}}
      }
        $w.menubar.show.m.list add separator
 
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
        $w.menubar.show.m.list add command -label {Commands} \
          -command {tsiDisplayAndSendCommand {info commands}}
      }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
        $w.menubar.show.m.list add command -label {Procedures} \
          -command {tsiDisplayAndSendCommand {info procs}}
      }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
      
        $w.menubar.show.m.list add command -label {Variables} \
          -command {tsiDisplayAndSendCommand {info globals}}
      }
}
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(show,$menuSuppressionCount)]} {
      $w.menubar.show.m add cascade -label {Variables} \
                          -menu $w.menubar.show.m.variables
      
      set subMenuCount 0
      menu $w.menubar.show.m.variables -tearoff 0

      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
        $w.menubar.show.m.variables add command \
          -label {Attribute Preferences Mode} \
         -command {tsiDisplayAndSendCommand {set attribute_preferences_mode}}
      }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
      
        $w.menubar.show.m.variables add command -label {Learning} \
         -command {tsiDisplayAndSendCommand {learn}}
      }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
      
        $w.menubar.show.m.variables add command -label {Indifferent selection} \
         -command {tsiDisplayAndSendCommand {indifferent-selection}}
      }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
      
        $w.menubar.show.m.variables add command -label {Default WME Depth} \
         -command {tsiDisplayAndSendCommand {set default_wme_depth}}
      }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
      
        $w.menubar.show.m.variables add command -label {Max Chunks} \
         -command {tsiDisplayAndSendCommand {set max_chunks}}
      }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
      
        $w.menubar.show.m.variables add command -label {Max Elaborations} \
         -command {tsiDisplayAndSendCommand {set max_elaborations}}
      }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
      
        $w.menubar.show.m.variables add command -label {O-Support Mode} \
          -command {tsiDisplayAndSendCommand {set o_support_mode}}
       }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
       
        $w.menubar.show.m.variables add command -label {Save Backtraces} \
          -command {tsiDisplayAndSendCommand {set save_backtraces}}
      }
      incr subMenuCount
      if {![info exists tsiSuppressMenus(show,$menuSuppressionCount,$subMenuCount)]} {     
      
        $w.menubar.show.m.variables add command -label {Warnings} \
         -command {tsiDisplayAndSendCommand {set warnings}}
      }
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(show,$menuSuppressionCount)]} {

     # Stats sub-menu
      $w.menubar.show.m add cascade -label {Stats} \
                          -menu $w.menubar.show.m.stats
      menu $w.menubar.show.m.stats -tearoff 0
      $w.menubar.show.m.stats add command -label {Firing menuSuppressionCounts} \
                       -command {tsiDisplayAndSendCommand {firing-menuSuppressionCounts}}
      $w.menubar.show.m.stats add command -label {System} \
                      -command {tsiDisplayAndSendCommand {stats --system}}
      $w.menubar.show.m.stats add command -label {Memory} \
                      -command {tsiDisplayAndSendCommand {stats --memory}}
      $w.menubar.show.m.stats add command -label {Rete} \
                      -command {tsiDisplayAndSendCommand {stats --rete}}

      $w.menubar.show.m add separator
      $w.menubar.show.m add command -label {Current Directory} \
                           -command {tsiDisplayAndSendCommand {pwd}}
  }
  pack $w.menubar.show -side left -padx 5
}




proc addTSIMemoryMenu {w} {
  global tsiConfig tsiSuppressMenus
  if $tsiConfig(debug) {puts "\n>>> starting making menu window-memory"}
 # MEMORY menu
  if {[info exists tsiSuppressMenus(memory)]} {
      return
  }

  if [winfo exists $w.menubar.mem.m] {
    destroy $w.menubar.mem.m
  } else {
    menubutton $w.menubar.mem -text {Memory} -menu $w.menubar.mem.m
  }

  menu $w.menubar.mem.m  -tearoff 0
  set menuSuppressionCount 0
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(memory,$menuSuppressionCount)]} {
     $w.menubar.mem.m add command -label {Clear WMEs (init-soar)} \
        -command {tsiDisplayAndSendCommand {init-soar}}
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(memory,$menuSuppressionCount)]} {
      $w.menubar.mem.m add cascade -label {Excise Productions} \
             -menu $w.menubar.mem.m.excise
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(memory,$menuSuppressionCount)]} {
      $w.menubar.mem.m add command -label {Add WME} -command {AddWmeDialog}
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(memory,$menuSuppressionCount)]} {
      $w.menubar.mem.m add command -label {Select Operator...} \
               -command {CreateOpDialog}
  }

  # Cascade for "Excise"

  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(memory,$menuSuppressionCount)]} {
     menu $w.menubar.mem.m.excise -tearoff 0
      $w.menubar.mem.m.excise add command -label {All productions} \
                           -command {tsiDisplayAndSendCommand {excise --all}}
      $w.menubar.mem.m.excise add command -label {Chunks} \
                     -command {tsiDisplayAndSendCommand {excise --chunks}}
      $w.menubar.mem.m.excise add command -label {Task productions} \
                     -command {tsiDisplayAndSendCommand {excise --task}}
   }
  pack $w.menubar.mem -side left -padx 5
}


proc addTSIWatchMenu {w} {
  ### Initialize variable settings for menus
  global tsiConfig tsiSuppressMenus

  if $tsiConfig(debug) {puts "\n>A> Hello from the watch menu 1"} 
  if {[info exists tsiSuppressMenus(watch)]} {
      return
  }

  tsiDetermineWatchSettings

  if $tsiConfig(debug) {puts "\n>A> Hello from the watch menu 2"} 

  if [winfo exists $w.menubar.watch.m] {
    destroy $w.menubar.watch.m
  } else {
    menubutton $w.menubar.watch -text {Watch} -menu $w.menubar.watch.m
  }

  menu $w.menubar.watch.m -tearoff 0
  $w.menubar.watch.m add command -label {Show Current Watch Status} \
           -command {tsiDisplayAndSendCommand watch}
  $w.menubar.watch.m add separator
  $w.menubar.watch.m add checkbutton -label {1. Decisions} \
           -variable watchValue(decisions) \
           -command {resetWatch decisions}
  $w.menubar.watch.m add checkbutton -label {2. Phases} \
           -variable watchValue(phases) \
           -command {resetWatch phases}
  $w.menubar.watch.m add command -label {All Productions} \
           -command {tsiDisplayAndSendCommand {watch --productions }}
  $w.menubar.watch.m add command -label {No Productions} \
           -command {tsiDisplayAndSendCommand {watch --productions remove}}
  $w.menubar.watch.m add checkbutton -label {3a. User Productions} \
           -variable {watchValue(--user)} \
           -command {resetWatch {--user} print noprint}
  $w.menubar.watch.m add checkbutton -label {3b. Chunks} \
           -variable {watchValue(--chunks)} \
           -command {resetWatch {--chunks} print noprint}
  $w.menubar.watch.m add checkbutton -label {3c. Justifications} \
           -variable {watchValue(--justifications)} \
           -command {resetWatch {--justifications} print noprint}
  ### -default doesn't appear to work in soar 7.0.4
  #$w.menubar.watch.m add checkbutton -label {3b. Default Productions} \
  #         -variable {watchValue(productions --default)} \
  #         -command {resetWatch {productions --default} print noprint}
  $w.menubar.watch.m add checkbutton -label {4. WMEs} \
           -variable watchValue(wmes) \
           -command {resetWatch wmes}
  $w.menubar.watch.m add checkbutton -label {5. Preferences} \
           -variable watchValue(preferences) \
           -command {resetWatch preferences}
  $w.menubar.watch.m add separator
  $w.menubar.watch.m add command -label {Nothing} \
           -command {set watchLevel(decisions) 0; set watchLevel(phases) 0; \
                     set watchLevel(productions) 0; set watchLevel(wmes) 0; \
                     set watchLevel(preferences) 0; \
                     tsiDisplayAndSendCommand {watch none}}
  $w.menubar.watch.m add command -label {1 only} \
           -command {tsiDisplayAndSendCommand {watch 1}}
  $w.menubar.watch.m add command -label {1-2 only} \
           -command {tsiDisplayAndSendCommand {watch 2}}
  $w.menubar.watch.m add command -label {1-3 only} \
           -command {tsiDisplayAndSendCommand {watch 3}}
  $w.menubar.watch.m add command -label {1-4 only} \
           -command {tsiDisplayAndSendCommand {watch 4}}
  $w.menubar.watch.m add command -label {1-5} \
           -command {tsiDisplayAndSendCommand {watch 5}}
  $w.menubar.watch.m add separator
  $w.menubar.watch.m add cascade -label {WME Detail} \
             -menu $w.menubar.watch.m.wme
  $w.menubar.watch.m add separator
  $w.menubar.watch.m add checkbutton -label {BackTracing} \
           -variable watchValue(--backtracing) \
           -command {resetWatch --backtracing}
  $w.menubar.watch.m add checkbutton -label {Learn print} \
           -variable watchValue(learning) \
           -command {resetWatch --learning print noprint}

  # Subcascade for "WME Detail"

  menu $w.menubar.watch.m.wme -tearoff 0
  $w.menubar.watch.m.wme add radiobutton -label {None} \
           -variable watchValue(WMElevel) -value 1 \
           -command {tsiSetWMELevel}
  $w.menubar.watch.m.wme add radiobutton -label {Time Tags} \
           -variable watchValue(WMElevel) -value 2 \
           -command {tsiSetWMELevel}
  $w.menubar.watch.m.wme add radiobutton -label {Full} \
           -variable watchValue(WMElevel) -value 3 \
           -command {tsiSetWMELevel}

  pack $w.menubar.watch -side left -padx 5

  if $tsiConfig(debug) {puts "\n>A> Done "} 
}

### The original Soar "watch" command is renamed in tsiInitAgent
proc watch {args} {

   set x [eval tsiInternalWatch $args]
   if {$args != {}} {
      tsiDetermineWatchSettings
   }
   return $x
}
 
### The original Soar "learn" command is renamed in tsiInitAgent
proc learn {args} {
   
   variable Debugger::learnSettings
   
   if {[string trim $args]==""} {
     tsiInternalLearn
   } else {
     tsiInternalLearn $args
   }
   
   output-strings-destination -push -append-to-result

   set ls [split [tsiInternalLearn] "\n"]
   set ls2 [lrange $ls 1 [llength $ls]]
   set learnSettings "Learn:"
   foreach s $ls2 {
      set learnSettings "$learnSettings [removeFirstChar [string trim $s]] "
   }
   output-strings-destination -pop
}

proc init-soar {args} {
   
   tsiInternalInitSoar
   if {[info proc envInitSoar] != ""} {
      envInitSoar
   }
   Debugger::UpdateRaisedNotebook
   echo "\nAll Soar WMEs have been removed.\n"
}

proc excise {args} {
   if {[string trim $args]==""} {
     tsiInternalExcise
   } else {
     tsiInternalExcise $args
   }
   Debugger::UpdateRaisedNotebook
   echo "\nProductions excised.\n"
}

proc tsiDetermineWatchSettings {} {
   global tsiConfig

   if $tsiConfig(debug) {puts "\n>A> Determining Watch Settings"} 

   ##output-strings-destination -push -append-to-result

   if $tsiConfig(debug) {puts "\n>A> Determining Watch Settings 1"} 

   set watchString [tsiInternalWatch]

   if $tsiConfig(debug) {puts "\n>A> Determining Watch Settings 2"} 

   ##output-strings-destination -pop

   if $tsiConfig(debug) {puts "\n>A> Determining Watch Settings 3"} 

   set Debugger::watchValue(decisions) [regexp {Decisions:[ ]+on} $watchString]

   if $tsiConfig(debug) {puts "\n>A> Determining Watch Settings 4"} 

   set Debugger::watchValue(phases) [regexp {Phases:[ ]+on} $watchString]
   ### Default doesn't appear to work in Soar 7.0.4
   #set {Debugger::watchValue(productions -default)} \
   #     [regexp {default productions:[ ]+on} $watchString]

   if $tsiConfig(debug) {puts "\n>A> Determining Watch Settings done"} 

   set {Debugger::watchValue(--user)} \
       [regexp {user productions:[ ]+on} $watchString]
   set {Debugger::watchValue(--chunks)} \
       [regexp {chunks:[ ]+on} $watchString]
   set {Debugger::watchValue(--justifications)} \
       [regexp {justifications:[ ]+on} $watchString]
   regexp {WME detail level:[ ]+([1-3])} $watchString dummy Debugger::watchValue(WMElevel)
   if $tsiConfig(debug) {puts "\n>A> Determining Watch Settings done"} 

   set Debugger::watchValue(wmes) [regexp {Working memory changes:[ ]+on} $watchString]
   set Debugger::watchValue(preferences) [regexp {firings/retractions:[ ]+on} $watchString]
   set Debugger::watchValue(learning) [regexp {Learning:[ ]+-print} $watchString]
   set Debugger::watchValue(backtracing) [regexp {Backtracing:[ ]+on} $watchString]
   set Debugger::watchValue(aliases) [regexp {Alias printing:[ ]+on} $watchString]

   if $tsiConfig(debug) {puts "\n>A> Determining Watch Settings done"} 

}


proc addTSIViewMenu {w} {
  global soar_library printStackFrame matchSetFrame opreferencesFrame
  global tsiConfig tsiSuppressMenus

  if $tsiConfig(debug) {puts "\n>>> starting AddTSIViewMenu"}
  if {[info exists tsiSuppressMenus(view)]} {
      return
  }

  if [winfo exists $w.menubar.monitor.m] {
    destroy $w.menubar.monitor.m
  } else {
    menubutton $w.menubar.monitor -text {View} -menu $w.menubar.monitor.m
  }

  menu $w.menubar.monitor.m -tearoff 0
  set menuSuppressionCount 0
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(view,$menuSuppressionCount)]} {
     $w.menubar.monitor.m add checkbutton -label {Print -Stack} \
           -variable monitor(print-stack) \
           -command "if \$monitor(print-stack) { \
                        makePrintGoalStackMonitor $w.t \
                     } else { \
                        killWindow \$printStackFrame printStack
                        catch [list destroy \$printStackFrame] \
                     }"
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(view,$menuSuppressionCount)]} {
     $w.menubar.monitor.m add checkbutton -label {Match Set} \
           -variable monitor(matches) \
           -command "if \$monitor(matches) { \
                        makeMatchesMonitor $w.t \
                     } else { \
                        killWindow \$matchSetFrame matchSet
                        catch [list destroy \$matchSetFrame] \
                     }"
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(view,$menuSuppressionCount)]} {
      $w.menubar.monitor.m add checkbutton -label {Op preferences} \
           -variable monitor(opreferences) \
           -command "if \$monitor(opreferences) { \
                        makeOPreferencesMonitor $w.t \
                     } else { \
                        killWindow \$opreferencesFrame opreferencesMonitor
                        catch [list destroy \$opreferencesFrame] \
                     }"
  }
  pack $w.menubar.monitor -side left -padx 5
}

proc addTSIProductionMenu {w} {
  global tsiConfig tcl_version tsiConfig tsiSuppressMenus

  if {[info exists tsiSuppressMenus(production)]} {
      return
  }
  ### Initialize variable settings for menus
  if [winfo exists $w.menubar.prod.m] {
    destroy $w.menubar.prod.m
  } else {
    menubutton $w.menubar.prod -text {Productions} -menu $w.menubar.prod.m
  }

  menu $w.menubar.prod.m -tearoff 0
  $w.menubar.prod.m add command -label {Reread User Productions} -command "addTSIProductionMenu $tsiConfig(tswFrame)"

  output-strings-destination -push -append-to-result
  set userProds [print -user]
  set defProds  [print -default]
  output-strings-destination -pop
 
  # tsiConfig(maxUserProdMenu) set in tsi-defaults.tcl
  if {$tsiConfig(maxUserProdMenu) > 0} {
      set userProds [lrange $userProds 0 $tsiConfig(maxUserProdMenu)]
  }
  set userProds [concat $userProds $defProds]

  foreach v $userProds {
      set commandstr1 "tsiDisplayAndSendCommand {matches $v}"
      set commandstr2 "tsiDisplayAndSendCommand {matches $v -wmes}"
      set commandstr3 "termTextInsert $w.t $v"
      set commandstr4 "tsiDisplayAndSendCommand {print $v}"
      set commandstr5 "tsiDisplayAndSendCommand {pwatch $v}"
      set firstAsterisk [string first "*" $v]
      set firstLetter [string range $v 0 [expr $firstAsterisk - 1]]
      if {![info exists alphList($firstLetter)]} {
	  set newIndex [getMenuInsertionIndex \
		  $w.menubar.prod.m "[string toupper $firstLetter]'s" 1]
	  $w.menubar.prod.m insert $newIndex cascade \
		  -label "[string toupper $firstLetter]'s" \
		  -menu $w.menubar.prod.m.$firstLetter
	  menu $w.menubar.prod.m.$firstLetter -tearoff 0
	  set alphList($firstLetter) 1
      }
      set newIndex [getMenuInsertionIndex $w.menubar.prod.m.$firstLetter $v 0]
      $w.menubar.prod.m.$firstLetter insert $newIndex cascade -label $v \
	      -menu $w.menubar.prod.m.$firstLetter.$v
      menu $w.menubar.prod.m.$firstLetter.$v -tearoff 0
      $w.menubar.prod.m.$firstLetter.$v add command -label {matches} \
	      -command $commandstr1
      $w.menubar.prod.m.$firstLetter.$v add command -label {matches -wmes} \
	      -command $commandstr2
      $w.menubar.prod.m.$firstLetter.$v add command \
	      -label {print production} \
	      -command $commandstr4
      $w.menubar.prod.m.$firstLetter.$v add command \
	      -label {print production name}  \
	      -command $commandstr3
      $w.menubar.prod.m.$firstLetter.$v add command \
	      -label {pwatch production} \
	      -command $commandstr5
  }
  if {[expr $tcl_version >= 8.0]} {
      foreach item [array names alphList] {
	  set numEntries [$w.menubar.prod.m.$item index last]
	  for {set i 40} {$i < $numEntries} {incr i 40} {
	      $w.menubar.prod.m.$item entryconfigure $i -columnbreak 1
	  }
      }
  }
  pack $w.menubar.prod -side left -padx 5

}

# This procedure determines at what index the menu entry with the label
# string should be placed in menu in order to keep the menu in alphabetical
# order.  Alphabetization starts at menu index startIndex - usually 0, unless
# some entries must remain at the top of the menu.
proc getMenuInsertionIndex {menu string startIndex} {
    set insertionIndex $startIndex
    set test 0
    set numItems [$menu index end]
    if { $numItems != "none" } {
	for {} {$insertionIndex <= $numItems} {incr insertionIndex} {
	    set test [string compare $string \
		    [$menu entrycget $insertionIndex -label]]
	    if { $test < 0 } {
		return $insertionIndex
	    }
	}
    }
    return $insertionIndex
}



#
# Proc addTSICommandsMenu
#
# Add a menu of soar commands to a specified window
#

proc addTSICommandsMenu {w} {
   global tsiConfig tsiSuppressMenus
  if {[info exists tsiSuppressMenus(commands)]} {
      return
  }
  if [winfo exists $w.menubar.commands.m] {
    destroy $w.menubar.commands.m
  } else {
    menubutton $w.menubar.commands -text Commands \
                                   -menu $w.menubar.commands.m
  }
   
  set menuSuppressionCount 0
  # pack the options and commands at the left
  pack $w.menubar.commands -side left

  # Commands menu contains Soar commands
  menu $w.menubar.commands.m -tearoff 0

  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(commands,$menuSuppressionCount)]} {
     $w.menubar.commands.m add command -label {run- All Agents 1 Decision Cycle} \
	   -command {tsiDisplayAndSendCommand {run-soar 1 d}}
   }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(commands,$menuSuppressionCount)]} {
      $w.menubar.commands.m add command -label {Run All Agents 1 Elaboration Cycle} \
	   -command {tsiDisplayAndSendCommand {run-soar 1 e}}
   }
  incr menuSuppressionCount
   if { $tsiConfig(debug) > 0 } {
       puts "menuSuppressionCount is $menuSuppressionCount tsiSuppressMenus(commands,$menuSuppressionCount) [array get tsiSuppressMenus]" }
  if {![info exists tsiSuppressMenus(commands,$menuSuppressionCount)]} {
      $w.menubar.commands.m add command -label {Init-Soar} \
	   -command {tsiDisplayAndSendCommand {init-soar}}
   }
  $w.menubar.commands.m add separator
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(commands,$menuSuppressionCount)]} {
      if {![info exists tsiConfig(hasEnvironment)] || $tsiConfig(hasEnvironment) == 0} {
         $w.menubar.commands.m add command -label {Stop Soar} \
	         -command {tsiDisplayAndSendCommand {stop-soar}}
      } else {
         $w.menubar.commands.m add command -label {Stop Soar} \
	         -command {tsiDisplayAndSendCommand {stop}}
      }
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(commands,$menuSuppressionCount)]} {
      $w.menubar.commands.m add command -label {Run Forever} \
	      -command {tsiDisplayAndSendCommand {run}}
   }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(commands,$menuSuppressionCount)]} {
      $w.menubar.commands.m add cascade -label {Indifferent Selection} \
             -menu $w.menubar.commands.m.is
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(commands,$menuSuppressionCount)]} {
      $w.menubar.commands.m add command -label {Print selected object} \
           -command [list printObject .printObjectWindow {Print Object}]
   }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(commands,$menuSuppressionCount)]} {
      $w.menubar.commands.m add command -label {Search} \
           -command [list searchWindow $w]
      #$w.menubar.commands.m add separator
      #$w.menubar.commands.m add command -label {Dismiss this window} \
   #                       -command [list killWindow $w]
   }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(commands,$menuSuppressionCount)]} {
  
     # Cascade for "Indifferent Selection"
      ### Need to keep menu in synch with reality, like with watch RMJ

      menu $w.menubar.commands.m.is -tearoff 0
      $w.menubar.commands.m.is add radiobutton -label {-first} \
        -variable isLevel -value {first} \
        -command {tsiDisplayAndSendCommand "indifferent-selection -$isLevel"}
      $w.menubar.commands.m.is add radiobutton -label {-last} \
        -variable isLevel -value {last} \
        -command {tsiDisplayAndSendCommand "indifferent-selection -$isLevel"}
      $w.menubar.commands.m.is add radiobutton -label {-ask} \
        -variable isLevel -value {ask} \
        -command {tsiDisplayAndSendCommand "indifferent-selection -$isLevel"}
      $w.menubar.commands.m.is add radiobutton -label {-random} \
        -variable isLevel -value {random} \
        -command {tsiDisplayAndSendCommand "indifferent-selection -$isLevel"}
   }
}


proc addTSIDemoMenu {w} {
  global soar_library tcl_platform tsiConfig tsiSuppressMenus

  if {[info exists tsiSuppressMenus(demos)]} {
      return
  }
  if [winfo exists $w.menubar.demo.m] {
    destroy $w.menubar.demo.m
  } else {
    menubutton $w.menubar.demo -text {Demos} -menu $w.menubar.demo.m 
  }

  menu $w.menubar.demo.m -tearoff 0 -postcommand "tsiUpdateDemoMenu $w"
  tsiUpdateDemoMenu $w
}
proc tsiUpdateDemoMenu {w} {
    global soar_library tcl_platform tsiConfig

    $w.menubar.demo.m delete 0 end

	if {([version] >= 8.0) && [regexp {.*ON} [soar8]]} {
	# create the demo menu for Soar8/Operand2
	set soar_demos [file join [file dirname $soar_library] [tsiSetDemoDir]]
	
	$w.menubar.demo.m add command -label {GUI Demos} \
		-font $tsiConfig(boldFont)
	$w.menubar.demo.m add command -label {Eight Puzzle} \
		-command {tsiLoadSoar8Demo eight-puzzle eight-puzzle.tcl}
	$w.menubar.demo.m add cascade -label {Water jug} \
		-menu $w.menubar.demo.m.wjgui

	$w.menubar.demo.m add separator
	
	
	## add the nonGUI demos 
	$w.menubar.demo.m add command -label {Non-GUI Demos} \
		-font $tsiConfig(boldFont)
	$w.menubar.demo.m add cascade -label {Blocks World} \
		-menu $w.menubar.demo.m.bw
	$w.menubar.demo.m add cascade -label {Eight Puzzle} \
		-menu $w.menubar.demo.m.ep
 	$w.menubar.demo.m add cascade -label {Missionaries} \
		-menu $w.menubar.demo.m.mac
	$w.menubar.demo.m add cascade -label {Towers of Hanoi} \
		-menu $w.menubar.demo.m.toh
	$w.menubar.demo.m add cascade -label {Water jug} \
		-menu $w.menubar.demo.m.wj
	
	## allow users to source default rules too
	$w.menubar.demo.m add separator
	$w.menubar.demo.m add cascade -label {Default Rules} \
		-font $tsiConfig(boldFont) -menu $w.menubar.demo.m.def
	
	if [winfo exists $w.menubar.demo.m.def] {destroy $w.menubar.demo.m.def}
	menu $w.menubar.demo.m.def -tearoff 0
	$w.menubar.demo.m.def add command -label {Readme} \
		-font $tsiConfig(boldFont) \
		-command {ShowFile \
		"[file join $soar_library .. demos default readme.txt]" 0}
	$w.menubar.demo.m.def add command -label {Simple} \
		-command {tsiLoadSoar8DefaultProds simple.soar}
	$w.menubar.demo.m.def add command -label {Selection} \
		-command {tsiLoadSoar8DefaultProds selection.soar}
	#$w.menubar.demo.m.def add command -label {Operator Subgoaling} \
	#	-command {tsiLoadSoar8DefaultProds operator-subgoaling.soar}


	if [winfo exists $w.menubar.demo.m.bw] {destroy $w.menubar.demo.m.bw}
	menu $w.menubar.demo.m.bw -tearoff 0
	$w.menubar.demo.m.bw add command -label {load Blocks World} \
		-command {tsiLoadSoar8Demo blocks-world blocks-world.soar}
	$w.menubar.demo.m.bw add command -label {load Blocks World Operator Subgoaling} \
		-command {tsiLoadSoar8Demo blocks-world blocks-world-operator-subgoaling.soar}
	$w.menubar.demo.m.bw add command -label {load Blocks World Look Ahead} \
		-command {tsiLoadSoar8Demo blocks-world blocks-world-look-ahead.soar}
	
	if [winfo exists $w.menubar.demo.m.ep] {destroy $w.menubar.demo.m.ep}
	menu $w.menubar.demo.m.ep -tearoff 0
	$w.menubar.demo.m.ep add command -label {Readme} \
		-font $tsiConfig(boldFont) \
		-command {ShowFile \
		"[file join $soar_library .. demos eight-puzzle readme.txt]" 0}
	$w.menubar.demo.m.ep add command -label {load eight-puzzle} \
		-command {tsiLoadSoar8Demo eight-puzzle eight-puzzle.soar}
	$w.menubar.demo.m.ep add command -label {load fifteen-puzzle} \
		-command {tsiLoadSoar8Demo eight-puzzle fifteen-puzzle.soar}
	
	if [winfo exists $w.menubar.demo.m.mac] {destroy $w.menubar.demo.m.mac}
	menu $w.menubar.demo.m.mac -tearoff 0
	$w.menubar.demo.m.mac add command -label {load Mac} \
		-command {tsiLoadSoar8Demo mac mac.soar}
	$w.menubar.demo.m.mac add command -label {load Mac Planning} \
		-command {tsiLoadSoar8Demo mac mac-planning.soar}
	
	if [winfo exists $w.menubar.demo.m.toh] {destroy $w.menubar.demo.m.toh}
	menu $w.menubar.demo.m.toh -tearoff 0
	$w.menubar.demo.m.toh add command -label {load Towers of Hanoi} \
		-command {tsiLoadSoar8Demo towers-of-hanoi towers-of-hanoi.soar}
	$w.menubar.demo.m.toh add command -label {load Towers of Hanoi Recursive} \
		-command {tsiLoadSoar8Demo towers-of-hanoi towers-of-hanoi-recursive.soar}
	
	if [winfo exists $w.menubar.demo.m.wj] {destroy $w.menubar.demo.m.wj}
	menu $w.menubar.demo.m.wj -tearoff 0
	$w.menubar.demo.m.wj add command -label {Readme} \
		-font $tsiConfig(boldFont) \
		-command {ShowFile \
		"[file join $soar_library .. demos water-jug readme]" 0}
	$w.menubar.demo.m.wj add command -label {load Water jug} \
		-command {tsiLoadSoar8Demo water-jug water-jug.soar}
	$w.menubar.demo.m.wj add command -label {load Water jug Look Ahead} \
		-command {tsiLoadSoar8Demo water-jug water-jug-look-ahead.soar}
	
	if [winfo exists $w.menubar.demo.m.wjgui] {destroy $w.menubar.demo.m.wjgui}
	menu $w.menubar.demo.m.wjgui -tearoff 0
	$w.menubar.demo.m.wjgui add command -label {Readme} \
		-font $tsiConfig(boldFont) \
		-command {ShowFile \
		"[file join $soar_library .. demos water-jug readme]" 0}
	$w.menubar.demo.m.wjgui add command -label {load Water Jug} \
		-command {tsiLoadSoar8Demo water-jug water-jug.tcl}
	$w.menubar.demo.m.wjgui add command -label {load Water Jug Look Ahead} \
		-command {tsiLoadSoar8Demo water-jug water-jug-look-ahead.tcl}

	
    } else {
	# this is either Soar 7 or soar8 mode is OFF
	set soar_demos [file join [file dirname $soar_library] [tsiSetDemoDir]]
	
	if $tsiConfig(expertise) {
	    $w.menubar.demo.m add command -label {GUI Demos} \
		    -font $tsiConfig(boldFont)
	    $w.menubar.demo.m add command -label {Critter World} \
		    -command {RunGUIDemo critter_world}
	    $w.menubar.demo.m add command -label {Eight Puzzle} \
		    -command {RunGUIDemo eight_puzzle}
	    $w.menubar.demo.m add separator
	}

	## the Soar7 "no-gui" demos ... (submenus defined after TSI demos)
	$w.menubar.demo.m add cascade -font $tsiConfig(boldFont) \
		-label {No GUI} -menu $w.menubar.demo.m.nogui
	$w.menubar.demo.m add separator
	
	##  the TSI demos for Soar 7
	$w.menubar.demo.m add command -label {TSI Demos} \
		-font $tsiConfig(boldFont)
	$w.menubar.demo.m add command -label {ht.s7} \
		-command {loadTSIFile ht.s7}
	$w.menubar.demo.m add command -label {ht2.s7} \
		-command {loadTSIFile ht2.s7}
	$w.menubar.demo.m add command -label {analogy.s7} \
		-command {loadTSIFile analogy.s7}
	$w.menubar.demo.m add command -label {analogy.toptwo.s7} \
		-command {loadTSIFile analogy.toptwo.s7}
	$w.menubar.demo.m add command -label {analogy.topspace.s7} \
		-command {loadTSIFile analogy.topspace.s7}
	$w.menubar.demo.m add command -label {default.s7} \
		-command {loadTSIFile default.s7}
	
	### the submenus for the no-gui demos
	
	if [winfo exists $w.menubar.demo.m.nogui] {
	    destroy $w.menubar.demo.m.nogui
	}
	menu $w.menubar.demo.m.nogui -tearoff 0
	$w.menubar.demo.m.nogui add command -label {Blocks World} \
		-command {RunNonGUIDemo blocks-world}
	$w.menubar.demo.m.nogui add command -label {Eight Puzzle} \
		-command {RunNonGUIDemo eight_puzzle}
	$w.menubar.demo.m.nogui add command -label {Farmer Puzzle} \
		-command {RunNonGUIDemo farmer}
	$w.menubar.demo.m.nogui add command -label {Keys and Boxes} \
		-command {RunNonGUIDemo kab}
	$w.menubar.demo.m.nogui add command -label {MissionariesandCannibals} \
		-command {RunNonGUIDemo missionaries}
	$w.menubar.demo.m.nogui add cascade -label {Safe Stack} \
		-menu $w.menubar.demo.m.nogui.sts
	$w.menubar.demo.m.nogui add cascade -label {Towers of Hanoi} \
		-menu $w.menubar.demo.m.nogui.toh
	$w.menubar.demo.m.nogui add command -label {Water Jug} \
		-command {RunNonGUIDemo water-jug}
    
	 menu $w.menubar.demo.m.nogui.sts -tearoff 0
	 $w.menubar.demo.m.nogui.sts add command -label {Initial Problem} \
		 -command {RunNonGUIProbDemo safe-stack \
		 [file join probs prob.1]}
	 $w.menubar.demo.m.nogui.sts add command -label {Volume(Box) known} \
		 -command {RunNonGUIProbDemo safe-stack \
		 [file join probs prob.2]}
	 $w.menubar.demo.m.nogui.sts add command \
		 -label {Lighter(Box, EndTable) known} \
		 -command {RunNonGUIProbDemo safe-stack \
		 [file join probs prob.3]}
	 $w.menubar.demo.m.nogui.sts add command \
		 -label {Lighter(Box, EndTable) and Weight(Box) = 0.1 known} \
		 -command {RunNonGUIProbDemo safe-stack \
		 [file join probs prob.4]}
	 $w.menubar.demo.m.nogui.sts add command \
		 -label {Initial Problem with different numbers} \
		 -command {RunNonGUIProbDemo safe-stack \
		 [file join probs prob.5]}
	 $w.menubar.demo.m.nogui.sts add command \
		 -label {Lighter(Box, EndTable) and Fragile(EndTable) = NO known } \
		 -command {RunNonGUIProbDemo safe-stack \
		 [file join probs prob.6]}
	 
	 menu $w.menubar.demo.m.nogui.toh -tearoff 0
	 set tower_notice [file join [file dirname $soar_library] \
		 [tsiSetDemoDir] no-gui towers-of-hanoi tn-toh.s7]
	 if {[file exists $tower_notice]} {
	     $w.menubar.demo.m.nogui.toh add command -label {Tower Noticing} \
		     -command {tsiDisplayAndSendCommand "source  \
		     [file join [file dirname $soar_library] \
		     [tsiSetDemoDir] no-gui towers-of-hanoi tn-toh.s7]"}
	     $w.menubar.demo.m.nogui.toh add separator
	 }
	 $w.menubar.demo.m.nogui.toh add command \
		 -label {3 Disks, 1 Goal Stack} \
		 -command {RunNonGUIProbDemo towers-of-hanoi \
		 [file join probs prob.3d.1]}
	 $w.menubar.demo.m.nogui.toh add command \
		 -label {3 Disks, 2 Goal Stacks} \
		 -command {RunNonGUIProbDemo towers-of-hanoi \
		 [file join probs prob.3d.2]}
       $w.menubar.demo.m.nogui.toh add command -label {4 Disks, 1 Goal Stack}\
		 -command {RunNonGUIProbDemo towers-of-hanoi \
		 [file join probs prob.4d.1]}
       $w.menubar.demo.m.nogui.toh add command -label {5 Disks, 1 Goal Stack}\
		 -command {RunNonGUIProbDemo towers-of-hanoi \
		 [file join probs prob.5d.1]}
       $w.menubar.demo.m.nogui.toh add command -label {5 Disks, 2 Goal Stacks}\
		 -command {RunNonGUIProbDemo towers-of-hanoi \
		 [file join probs prob.5d.2]}
	 
     }
     
     pack $w.menubar.demo -side right -padx 3
 }


proc tsiDisplayAndSendCommand {command} {
global tsiConfig

   if [winfo exists .tsw] {
      termTextSendCommand $tsiConfig(tswFrame).t $command
   } else {
      puts $command
      puts [$command]
   }
   
   focus .tsw
}

#
# add the Soar help Menu
#

proc addTSIHelpMenu {w} {
  global soar_library tcl_platform soar_doc_dir tsiConfig tsiSuppressMenus

  if {[info exists tsiSuppressMenus(help)]} {
      return
  }
  if $tsiConfig(debug) {puts "\n>>> starting AddTSIHelpMenu"}
  if [winfo exists $w.menubar.help.m] {
    destroy $w.menubar.help.m
  } else {
    menubutton $w.menubar.help -text {Help} -menu $w.menubar.help.m
  }

  menu $w.menubar.help.m -tearoff 0
  pack $w.menubar.help -side right -padx 5

  set menuSuppressionCount 0
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(help,$menuSuppressionCount)]} {
     $w.menubar.help.m add command -label {Show TSI help} -command {TSIHelp}
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(help,$menuSuppressionCount)]} {
     $w.menubar.help.m add command -label {help -all} \
         -command {tsiDisplayAndSendCommand {help -all}}
  }
  $w.menubar.help.m add separator
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(help,$menuSuppressionCount)]} {
      $w.menubar.help.m add cascade -label {On Soar} -menu $w.menubar.help.m.soar

      # Cascade menu for the "On Soar" item above.

        menu $w.menubar.help.m.soar -tearoff 0
      if [info exists soar_doc_dir] {
            set docDir $soar_doc_dir
      } else {
         set docDir [file join [file dirname $soar_library] doc]
      }
      $w.menubar.help.m.soar add command -label {Man Pages (Soar Commands)} \
             -command "ShowManPage \"[file join $docDir cat]\" 0"
      $w.menubar.help.m.soar add command -label {List All Commands} \
             -command {tsiDisplayAndSendCommand {help -all}}
        ### Do we need these, because they don't currently seem to exist. RMJ
      #$w.menubar.help.m.soar add command -label {Change Log} \
      #           -command "ShowFile \"[file join $docDir ChangeLog]\" "
      $w.menubar.help.m.soar add command -label {Release Notes} \
	     -command "ShowFile \"[file join $docDir RelNotes.txt]\" "
      $w.menubar.help.m.soar add command -label {Tcl/Tk Init File} \
             -command "ShowFile \"[file join $soar_library soar.tcl]\" "
      $w.menubar.help.m.soar add command -label {News} \
             -command {tsiDisplayAndSendCommand soarnews}
      $w.menubar.help.m.soar add command -label {Version} \
             -command {tsiDisplayAndSendCommand version}
      if {$tcl_platform(platform) == {unix}} {
         $w.menubar.help.m.soar add separator
         $w.menubar.help.m.soar add cascade -label {WWW} \
                  -menu $w.menubar.help.m.soar.www

         # Subcascade for {On Soar/WWW}

         menu $w.menubar.help.m.soar.www -tearoff 0
         $w.menubar.help.m.soar.www add command -label {Soar} \
              -command {viewWWW http://www.isi.edu/soar/soar.html}
         $w.menubar.help.m.soar.www add command -label {Soar Archive} -command \
            {viewWWW http://www.isi.edu/soar/soar-archive-homepage.html}
         $w.menubar.help.m.soar.www add command -label {Soar User's Manual} \
            -command {viewWWW http://www.isi.edu/soar/users-manual/html/soar6-manual.info.Top.html}
         $w.menubar.help.m.soar.www add command -label {Soar Online Man. Pages} \
            -command {viewWWW file:$soar_library/../doc/html/soar_man.html &}
         $w.menubar.help.m.soar.www add command -label {Soar FAQ} \
            -command  {viewWWW http://www.psychology.nottingham.ac.uk/users/ritter/soar-faq.html &}
      }
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(help,$menuSuppressionCount)]} {
     $w.menubar.help.m add cascade -label {On Tcl} -menu $w.menubar.help.m.tcl

     # Cascade menubar for the "On Tcl" item above.

     menu $w.menubar.help.m.tcl -tearoff 0
     $w.menubar.help.m.tcl add command -label {Init File} \
           -command {ShowFile [file join [info library] init.tcl]}
     $w.menubar.help.m.tcl add command -label {Library Location} \
           -command {tsiDisplayAndSendCommand {info library}}
     $w.menubar.help.m.tcl add command -label {Man. Pages} \
           -command {ShowManPage "$tsiConfig(tcl_man_dir)"}
     $w.menubar.help.m.tcl add command -label {Version} \
           -command {tsiDisplayAndSendCommand {info tclversion}}
     if {$tcl_platform(platform) == {unix}} {
        $w.menubar.help.m.tcl add separator
        $w.menubar.help.m.tcl add cascade -label {WWW} \
                     -menu $w.menubar.help.m.tcl.www

        # Subcascade for "On Tcl/WWW"

        menu $w.menubar.help.m.tcl.www -tearoff 0
        $w.menubar.help.m.tcl.www add command -label {General} \
             -command {viewWWW http://www.isi.edu/soar/schwamb/tcl.html}
        $w.menubar.help.m.tcl.www add command -label {Manual Pages} \
              -command \
          {viewWWW http://www.cis.upenn.edu/~ioi/tix/doc/tcltkman/tcltkman.html}
        $w.menubar.help.m.tcl.www add command -label {SunScript} \
              -command {viewWWW http://sunscript.sun.com/}
     }
  }
  incr menuSuppressionCount
  if {![info exists tsiSuppressMenus(help,$menuSuppressionCount)]} {
     $w.menubar.help.m add cascade -label {On Tk} -menu $w.menubar.help.m.tk

     # Cascade menu for the "On Tk" item above.
     menu $w.menubar.help.m.tk -tearoff 0
     $w.menubar.help.m.tk add command -label {Init File} \
          -command {ShowFile [file join [uplevel #0 {set tk_library}] \
                                                 tk.tcl]}
     $w.menubar.help.m.tk add command -label {Library Location} \
         -command {tsiDisplayAndSendCommand \
                     {uplevel #0 {set tk_library}}}
     $w.menubar.help.m.tk add command -label {Manual Pages} \
          -command {ShowManPage "$tsiConfig(tcl_man_dir)"}
     #     -command {set temp [file dirname [uplevel #0 {set tk_library}]]; \
     #               ShowManPage [file join $temp doc]}
     $w.menubar.help.m.tk add command -label {Version} \
       -command {tsiDisplayAndSendCommand {uplevel #0 {set tk_version}}}
     $w.menubar.help.m.tk add separator
     $w.menubar.help.m.tk add cascade -label {WWW} \
          -menu $w.menubar.help.m.tk.www

     # Subcascade for "On Tk/WWW"
     menu $w.menubar.help.m.tk.www -tearoff 0
     $w.menubar.help.m.tk.www add command -label {General} \
          -command {viewWWW http://www.isi.edu/soar/schwamb/tcl.html}
     $w.menubar.help.m.tk.www add command -label {Man. Pages} \
          -command \
         {viewWWW http://www.cis.upenn.edu/~ioi/tix/doc/tcltkman/tcltkman.html}
     $w.menubar.help.m.tk.www add command -label {SunScript} \
          -command {viewWWW http://sunscript.sun.com/}
   }
  if $tsiConfig(debug) {puts "\n>>> ending AddTSIHelpMenu"}
}



###
###	VIII.	True utility functions
###

## Prints an array!
## from p. 84 from outsterhout
proc parray name {
  upvar $name a
  foreach el [lsort [array names a]] {
      puts "$el = $a($el)"}}




###
###	IX.	Force an operator selection
###

  # change -FER  new function
## should be packed better
## Operator name:  box
## attribute   value
## ^ attribute value
  

proc CreateOpDialog {{w .create_op_dialog}} {
    global required_op op_arg_name op_arg_value
    catch {destroy $w}

    toplevel $w
    wm title $w {Propose Operator}
    wm iconname $w {Propose Operator}

    frame $w.frame -borderwidth 4
    label $w.frame.label -text {Operator name: }
    entry $w.frame.name -width 20 -relief sunken -textvariable required_op

    frame $w.frame2 -borderwidth 4
    label $w.frame2.label -text {Arg name ^}
    entry $w.frame2.name -width 20 -relief sunken -textvariable op_arg_name

    frame $w.frame3 -borderwidth 4
    label $w.frame3.label -text {}
    entry $w.frame3.name -width 20 -relief sunken -textvariable op_arg_value

    button $w.doit -text {Require operator} -command {require-op $required_op \
         $op_arg_name $op_arg_value; \
          destroy .create_op_dialog; init-soar; d 1}

    button $w.undoit -text {Clear requirement} -command {unrequire-op; \
          destroy .create_op_dialog; }

    button $w.ok -text Cancel -command {destroy $w}

    pack $w.frame.label -side top -pady 2m
    pack $w.frame.name -side left -padx 1m -pady 0m

    pack $w.frame2.label -side top -pady 0m
    pack $w.frame2.name -side top -pady 0m

    pack $w.frame3.label -side top -pady 0m
    pack $w.frame3.name -side top -padx 0m

    pack $w.frame -side top
    pack $w.frame2 -side top
    pack $w.frame3 -side top
    pack $w.doit -side left
    pack $w.undoit -side left
    pack $w.ok -side right
}


## need to have format-watch fixed -fer
## make op required as well
## rename roarg to roattr
## later, make roattr an aray/list

# needs to be made require preference, nt just best
proc require-op {rop roarg roval} {
  puts "requiring $rop with ^$roarg $roval"
  if {$roarg == {}} {set roarg {require-op-dattr}}
  if {$roval == {}} {set roval {require-op-dval}}
 # want expansion, so use quotes not braces
 sp "ror
  (state <s> ^superstate nil)
  -->
   (<s> ^operator <o> +)
   (<s> ^operator <o> >)
   (<o> ^name $rop ^$roarg $roval)"
  # trace the value here, whihc is broken, use ifdef as well
## format-watch -object -add o $rop  {%id ($rop $roarg: %v{$roarg}}

  puts {end req}
}

proc unrequire-op {} {
  puts {unrequiring op}
 excise ror
  puts {end req}
}

proc askCallback { args } {
	global askCallbackChoice w
	
	set w [toplevel .w]
	set l [listbox $w.l -selectmode browse]
	wm title $w "Ask"
	wm geometry $w +200+200
	
	pack $w.l
	
	bind $w.l <Return> {
		global askCallbackChoice w
		set askCallbackChoice [$w.l curselection]
		destroy $w
	}
	
	bind $w.l <Double-1> {
		global askCallbackChoice w
		set askCallbackChoice [$w.l curselection]
		destroy $w
	}
	
	foreach x $args {
		$l insert end "Operator $x"
	}
	$l insert end "First"
	$l insert end "Last"
	$l insert end "Random"
	
	focus $w.l
	grab $w.l
	tkwait window $w.l
	
	return $askCallbackChoice
}

proc askCallback2 { args } {
	echo "\nPlease choose one of the following:\n"
	set numargs [llength $args]

	for {set i 0} {$i < $numargs} {incr i 1} {
		echo "$i: [lindex $args $i]\n"
	}
	
	echo "Or choose one of the following to change the user-select mode\n"
	echo "to something else:  $numargs (first), [expr $numargs+1] (last), [expr $numargs+2] (random)\n"
	
	echo "Enter selection (1-[expr $numargs+2]): "
	
	set choice [gets stdin]
	
	return $choice
}

proc askCallback2 { args } {
	puts "\nPlease choose one of the following:\n"
	set numargs [llength $args]

	for {set i 0} {$i < $numargs} {incr i 1} {
		puts "$i: [lindex $args $i]\n"
	}
	
	puts "Or choose one of the following to change the user-select mode\n"
	puts "to something else:  $numargs (first), [expr $numargs+1] (last), [expr $numargs+2] (random)\n"
	
	puts "Enter selection (1-[expr $numargs+2]): "
	
	set choice [gets stdin]
	
	return $choice
}
