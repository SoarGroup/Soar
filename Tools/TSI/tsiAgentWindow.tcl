#### -*- Mode: Emacs-Lisp -*- 
###############################################################################
####              
#### File            : tsiAgentWindow.tcl
#### Authors         : Randy Jones, etc.
#### Created On      : not known
#### Last Modified By: Frank Ritter
#### Last Modified On: 16-Feb-97
#### Update Count    : 
#### 
#### PURPOSE
#### This script implements a GUI for a Soar application.
####
#### TABLE OF CONTENTS
####
####	i.	Source File INCLUDEs and globals
####	I.	Interaction window Definition
#### 	II.	Support PROCEDURES
####
###############################################################################
global tsiConfig

###
###	I.	Interaction window Definition
###

proc makeSoarInteractionWindow {{x 0} {y 30} } {
  global tsiConfig auto_path tsi_library
  global wwwViewer soar_library interp_name
  global _agent _kernel

  if $tsiConfig(debug) {puts "\n>>> starting makeSoar interaction window1"}

  if $tsiConfig(debug) {
    puts "\n>>> starting makeSoar interaction window2 $interp_name"
  }

  if [catch registerWithController] {
      error "Failed to register new agent with controller"
  }

  if $tsiConfig(debug) {puts "\n>>> starting makeSoar interaction window3"}
  # if the window already exists, just quit

  if {[winfo exists .tsw] != 0} { return }

  if $tsiConfig(debug) {puts "\n>>> starting makeSoar interaction window2"}
  # Otherwise build a new top-level window

  toplevel .tsw
  wm title .tsw "$interp_name Agent Interaction Window"
  wm iconname .tsw $interp_name

  ##set x [expr {$x + 40 * [expr int([llength [tsiListAgents]]) - 1 ] } ]
  if {$x >= 0} { set x +$x }
  if {$y >= 0} { set y +$y }
  wm geometry .tsw $x$y

  # Create menus that will be attached to menu bar.  We start by defining
  # a frame to contain all the menu buttons.

  frame .tsw.menubar -relief raised -bd 2
  pack .tsw.menubar -side top -fill x

  #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  # The following menus are listed in the order they appear in the menu
  # bar (except for the "Demos" menu since it is packed to the right
  # instead of the left).  Within menu sections, commands are listed
  # in the order of the menu entries.  Cascades and sub-cascades are
  # listed separately.

  addTSIFileMenu .tsw

  addTSIShowMenu .tsw

  addTSIMemoryMenu .tsw

  #if $tsiConfig(debug) {puts "\n>>> starting makeSoar interaction window-productions"}
  #addTSIProductionMenu .tsw

  if $tsiConfig(debug) {puts "\n>>> starting makeSoar interaction window-watch"}
  addTSIWatchMenu .tsw

  if $tsiConfig(debug) {puts "\n>>> starting makeSoar interaction window-view"}
  addTSIViewMenu .tsw

  if $tsiConfig(debug) {puts "\n>>> starting makeSoar interaction window-commands"}
  addTSICommandsMenu .tsw

  if $tsiConfig(debug) {puts "\n>>> starting makeSoar interaction window-help"}
  addTSIHelpMenu .tsw

  if $tsiConfig(debug) {puts "\n>>> starting makeSoar interaction window-demo"}
  addTSIDemoMenu .tsw

  #- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  # Create TEXT WINDOW and scrollbar

  if $tsiConfig(debug) {puts "\n>>> starting makeSoar interaction window-term"}

  setUpSoarTerm $tsiConfig(tswFrame)
  $tsiConfig(tswFrame) configure -font $tsiConfig(normalFont)
  $tsiConfig(tswFrame) configure -yscrollcommand {.tsw.s set}
  scrollbar .tsw.s -relief flat -command {$tsiConfig(tswFrame) yview}

  # QUICK ACTION BUTTONS

  frame .tsw.frame -borderwidth 0
  label .tsw.frame.environmentLabel  -text "Evironment:"
  button .tsw.frame.run  -text {Run} -command {tsiDisplayAndSendCommand run}
  button .tsw.frame.step -text {Step} -command {tsiDisplayAndSendCommand {run 1}} 
  if ([info exists tsiConfig(hasEnvironment)] && $tsiConfig(hasEnvironment) != 0) {
     button .tsw.frame.stop -text {Stop} -command {tsiDisplayAndSendCommand {stop}}
  } else {
     button .tsw.frame.stop -text {Stop} -command {tsiDisplayAndSendCommand {stop-soar}}
  }
  label .tsw.frame.soarLabel  -text "This Agent:"
  button .tsw.frame.myelab -text {Elaboration} -command {tsiDisplayAndSendCommand {run-soar 1 e -self}}
  button .tsw.frame.mydecision -text {Decision} -command {tsiDisplayAndSendCommand {run-soar 1 d -self}}
  button .tsw.frame.myphase -text {phase} -command {tsiDisplayAndSendCommand {run-soar 1 p -self}}
  button .tsw.frame.search -text {Search} -command {searchWindow .tsw}

  # Pack up the GUI

  pack .tsw.s -side right -fill both
  pack $tsiConfig(tswFrame) -side top -expand 1 -fill both

  pack .tsw.frame.environmentLabel .tsw.frame.step .tsw.frame.run .tsw.frame.stop \
       .tsw.frame.soarLabel .tsw.frame.myphase .tsw.frame.myelab .tsw.frame.mydecision -side left
  pack .tsw.frame.search -side right
  pack .tsw.frame -side bottom -fill x

  #--------------------------------------------------------------------------

  # BINDINGS

  # Don't like "click to focus"
  # bind .tsw <Enter> { focus $tsiConfig(tswFrame) }
  #--------------------------------------------------------------------------
  # Startup INITIALIZATIONS:

  # Redirect all printed output (from stdout, usually) to the text widget

  ## currently, 1-Feb-97, the mac can't handle the extra widget, so keep 
  ## output at default location
  ##global tcl_platform
  ##if {$tcl_platform(platform) != {mac}} {
  ##}
  # This better work in Soar 7.1 or we are in trouble RMJ
  #output-strings-destination -push -text-widget $tsiConfig(tswFrame)
  output-strings-destination -push -procedure tsiOutputToWidget

  # We add a monitor to make sure that long running commands like
  # "go" do not stop X events from being processed.

  monitor -add after-decision-cycle update

  eval $tsiConfig(tswFrame) tag configure prompt $tsiConfig(promptConfigure)
  eval $tsiConfig(tswFrame) tag configure oldUserText $tsiConfig(userTextConfigure)
  eval $tsiConfig(tswFrame) tag configure searchFind $tsiConfig(searchTextConfigure)

  # Initialize focus of keyboard events
  focus $tsiConfig(tswFrame)
}

if $tsiConfig(debug) {puts "\n>>> tsiAgentWindow1"}

#--------------------------------------------------------------------------



###
### 	II.	Support PROCEDURES
### 
proc tsiOutputToWidget {x} {
global tsiConfig

   $tsiConfig(tswFrame).t insert end $x
   $tsiConfig(tswFrame).t see end
}

######## Set the watch options from a menu
proc resetWatch {attribute {on ""} {off remove}} {
   if $Debugger::watchValue($attribute) {
      tsiDisplayAndSendCommand [concat watch $attribute $on]
   } else {
      tsiDisplayAndSendCommand [concat watch $attribute $off]
   }
}

proc tsiSetWMELevel {} {
   switch $Debugger::watchValue(WMElevel) {
      1 {tsiDisplayAndSendCommand {watch --nowmes}}
      2 {tsiDisplayAndSendCommand {watch --timetags}}
      3 {tsiDisplayAndSendCommand {watch --fullwmes}}
   }
}

## Set up the directories for later use in soar.soar
## Notes: different paths, can't use ::: in the midst of a cd, just ::
## different types of files as well, and can't spawn html viewer as easily
## get an error:>> invalid command name ".showfile1.text"<<

######## New definition of help

### Currently, the "help" command in Soar sends its output directly to
### the "console" (for lack of a better term, that is wherever Soar was
### run from), with no way to redirect it (to a text widget, for example).
### So for the graphical interface, if someone types "help" we need to
### trap the command and do our own version where we load the help file
### into a text widget ourselves.

### The regular soar "help" command is renamed in tsiInitAgent
proc help args {
    global soar_library soar_help tcl_version soar_doc_dir soarConfig tsiConfig
    ### If the user types "help" with no arguments, or with the "-usage"
    ### argument, we can simply pass the command on to soar (using
    ### tsiInternalHelp, and the output will be directed to the text widget.
    ### The "help -usage" output has formatting commands in it, so we will
    ### pass that through the ScanTextAndHighlight procedure.
    if {($args == {}) || ([string index [lindex $args 0] 0] == {-})} {
        if {[lindex $args 0] == {-usage}} {
            scan [$tsiConfig(tswFrame).t index insert] %d start}
        termTextProcessCommand $tsiConfig(tswFrame).t [concat tsiInternalHelp $args]
        if {[lindex $args 0] == {-usage}} {
            scan [$tsiConfig(tswFrame).t index end] %d finish
            ScanTextAndHighlight $tsiConfig(tswFrame).t $start $finish }
        return
    }
    ### If we get this far, the user has asked for help on a specific topic,
    ### so we handle that case ourselves by displaying the topic file in a
    ### separate window.
    set topic [lindex $args 0]   
    ## Soar-Bugs 127 - Test for unambiguous partial command
	 if {([llength [info commands ${topic} ]] != 1)} {
       # topic is not a complete unique commandname
	    if {([llength [info commands ${topic}*]] == 1)} {
		    set topic [info commands ${topic}*]
		 } elseif {([llength [info commands ${topic}*]] > 1)} {
			 $tsiConfig(tswFrame).t insert insert \
			       "Ambiguous Help topic: [info commands ${topic}*]"
			 return
       }
    }
    if [info exists soar_doc_dir] {
       set fullTopic [file join $soar_doc_dir cat $topic.n]
    } else {
       set fullTopic [file join $soar_library .. doc cat $topic.n]
    }
    if {[file isfile $fullTopic] == 1} {
        ShowFile "$fullTopic" 0
    } else {
        $tsiConfig(tswFrame).t insert insert "Sorry, there is no help on \"$topic\".\n"
    }
}

# These procedures are used to run the demos which do not have a
# GUI.

if $tsiConfig(debug) {puts "\n>>> tsiAgentWindow2"}

proc tsiSetDemoDir {} {
   if {[version] < 8.0} {return "demos"}
   if {([version] >= 8.0) && [regexp {.*ON} [soar8]]} {
	   return "demos"
	} else {
		return "demos_soar7"
	}
}

# Can be changed to work with mac and unix versions.
# 26-Mar-97 -FER
proc loadTSIFile {demo} {
    global tsi_library
    if [catch "pushd [file join $tsi_library demos]"] {
       set old_dir [pwd]
       cd [file join $tsi_library demos]
    }
    tsiDisplayAndSendCommand {excise -all}
    tsiDisplayAndSendCommand "source $demo"
    if [info exists old_dir] {
       if [catch "cd $old_dir"] {
          puts "Warning: Couldn't return to directory $old_dir"
       }
    } elseif [catch popd] {
       puts {Warning: Couldn't return to old directory}
    }
}

proc tsiLoadSoar8Demo {demo filename} {
    global soar_library 

    set demodir [tsiSetDemoDir]
    if [catch "pushd [file join $soar_library .. $demodir $demo]"] {
       set old_dir [pwd]
       cd [file join $soar_library .. $demodir $demo]
    }
    tsiDisplayAndSendCommand {excise -all}
    tsiDisplayAndSendCommand "source $filename"
    if [info exists old_dir] {
       if [catch "cd $old_dir"] {
          puts "Warning: Couldn't return to directory $old_dir"
       }
    } elseif [catch popd] {
       puts {Warning: Couldn't return to old directory}
    }
}
proc tsiLoadSoar8DefaultProds {filename} {
    global soar_library 
### IMPORTANT:  must only be called from Soar version >8.0
###  hmm...maybe not demos/default also exists for Soar7 demos...
    if [catch "pushd [file join $soar_library .. demos default]"] {
       set old_dir [pwd]
       cd [file join $soar_library .. demos default]
    }
    tsiDisplayAndSendCommand "source $filename"
    if [info exists old_dir] {
       if [catch "cd $old_dir"] {
          puts "Warning: Couldn't return to directory $old_dir"
       }
    } elseif [catch popd] {
       puts {Warning: Couldn't return to old directory}
    }
}

proc RunGUIDemo {demo} {
    global default soar_library 

	 set demodir [tsiSetDemoDir]
    tsiDisplayAndSendCommand {excise -all}
    tsiDisplayAndSendCommand "source \"[file join $soar_library default.soar]\" "
    tsiDisplayAndSendCommand "uplevel #0 [list pushd \
        \"[file join [file dirname $soar_library] $demodir gui \
                                  $demo]\"]"
    tsiDisplayAndSendCommand "uplevel #0 [list source $demo.tcl]"
    tsiDisplayAndSendCommand "uplevel #0 popd"
}

proc RunNonGUIDemo {demo} {
    global default soar_library
	 set demodir [tsiSetDemoDir]
    tsiDisplayAndSendCommand {excise -all}
    tsiDisplayAndSendCommand "source $default"
    tsiDisplayAndSendCommand "source \
        [file join [file dirname $soar_library] $demodir no-gui \
                                  $demo $demo.soar]"
}

proc RunNonGUIProbDemo {demo prob} {
    global default soar_library

	 set demodir [tsiSetDemoDir]
    tsiDisplayAndSendCommand {excise -all}
    tsiDisplayAndSendCommand "source $default"
    tsiDisplayAndSendCommand "source \
        [file join [file dirname $soar_library] $demodir no-gui \
                                  $demo $demo.soar]"
    tsiDisplayAndSendCommand "source \
        [file join [file dirname $soar_library] $demodir no-gui \
                                  $demo $prob]"
}

### Use the information we know about mosaic and netscape WWW viewers
### to view web pages in a friendly way (i.e., use an existing viewer
### process if one is available, otherwise start the viewer up).
proc viewWWW {url} {
   global wwwViewer env

   if ![info exists wwwViewer] {
      tk_dialog .error {Error} "WWW viewer not set.\nSet wwwViewer to mosaic, \
xmosaic, or netscape\n(other viewers can be tried but may not work)." error 0 Dismiss
      return
   }
   # mosaic/xmosaic
   if {([string first mosaic $wwwViewer] == 0) || \
       ([string first xmosaic $wwwViewer] == 0)} {
      if {![catch "set mp [open [file join $env(HOME) .mosaicpid] r]"] && \
          ![catch "gets $mp pid"]} {
         close $mp
         if ![catch "set mp [open /tmp/Mosaic.$pid w]"] {
            puts $mp newwindow
            puts $mp $url
            close $mp
            if ![catch "exec kill -USR1 $pid"] {
               return
            }
         }
      }
      if [catch "exec $wwwViewer $url &"] {
         tk_dialog .error {Error} "Cannot launch $wwwViewer WWW viewer" \
                   error 0 Dismiss
      }
      return
   }
   # netscape
   if {[string first netscape $wwwViewer] == 0} {
      if [catch "exec $wwwViewer -remote openURL($url)"] {
         if [catch "exec $wwwViewer $url"] {
         tk_dialog .error {Error} "Cannot launch $wwwViewer WWW viewer" \
                   error 0 Dismiss
         }
      }
      return
   }
   # Tcl/Tk-based viewers?
   #
   # all others
   if [catch "exec $wwwViewer $url"] {
      tk_dialog .error {Error} "Cannot launch $wwwViewer WWW viewer" \
                error 0 Dismiss
   }
}

