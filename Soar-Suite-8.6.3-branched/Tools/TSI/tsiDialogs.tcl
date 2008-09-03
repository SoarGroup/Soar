##########################################################################
# File         : tsiDialogs.tcl
# Author       : Gordon D. Baxter
# Date	       : 10/06/96
# Description 
#
# This file implements a simple set of dialog boxes for general use.
#   - an information/warning dialog with only an OK button
#   - a warning dialog with an Ok/Cancel option
#   - a dialog to allow a single variable to be modified
# The dialogs are essentially the same in appearance, with a coloured
# background to highlight their appearance on the display.
# The dialogs are based around the one in Brent Welch's text book
# "Practical Programming in Tcl and Tk".
#
##########################################################################
#
# A simple proc for information dialogs (with Dismiss button)
#
# Unlike tk_dialog, this window does not grab the mouse, so you can
# leave it up while you do other things.

proc tsiInfoWindow {dlg promptText {ttl {Info}} {txtFont 0}} {
  global tsiConfig

  if {$txtFont==0} {
    set txtFont $tsiConfig(dialogDefaultTextFont)
  }

  # make a toplevel frame for the dialog

  if [winfo exists $dlg] {
    destroy $dlg
  }

  toplevel $dlg -borderwidth 10 -bg bisque2
  wm title $dlg $ttl

  # make sure it comes near the middle of the screen

  set centreX [expr [winfo screenwidth $dlg] / 2]
  set centreX [expr $centreX - 250]
  set centreY [expr [winfo screenheight $dlg] / 2]
  set centreY [expr $centreY - 120]

  wm geometry $dlg +$centreX+$centreY

  # Add a message widget to show the prompt, after calculating a
  # reasonable width in pixels

  set promptLen [string length $promptText]
  if {$promptLen > 40} {
    set promptLen [expr $promptLen / 2]
  }

  message $dlg.msg -text $promptText \
                   -width [expr $promptLen * 12] \
                   -justify center \
                   -bg bisque2 \
                   -font $txtFont

  # add a frame for the buttons.

  set b [frame $dlg.buttons -bd 10 -bg bisque2]
  pack $dlg.msg $dlg.buttons -side top

  # add the OK button

  button $b.ok -text {Dismiss} -command "destroy $dlg" \
                        -width 6 -font $tsiConfig(dialogFont)
  pack $b.ok -anchor center
}

#
# Simple dialog for getting a single value from the user
# Takes as arguments the text used as a prompt for the value,
# and the current value.  Returns the new value.  Called as follows:
#
#   set pr {Enter new print depth value}
#   set printDepth getOneValueDialog $pr $printDepth
#
#   set printDepth [getOneValueDialog \
#                   {Enter new print depth value} $printDepth]
#
# NOTE: If the user deletes the current value and does not enter a new
#       one, the value is left unchanged, i.e., the old value is returned.
#       It is also assumed that the variable that forms the second
#       argument DOES exist.
#


proc getOneValueDialog {promptText currentVal} {

  # Set up a global to be used for holding results as an array structure
  global prompt tsiConfig

  # Make the frame for the dialog
  set dlg [toplevel .prompt -borderwidth 10 -bg bisque2]
  wm title .prompt {Enter New Value}

  # make sure it comes near the middle of the screen
  set centreX [expr [winfo screenwidth $dlg] / 2]
  set centreX [expr $centreX - 250]
  set centreY [expr [winfo screenheight $dlg] / 2]
  set centreY [expr $centreY - 120]

  wm geometry $dlg +$centreX+$centreY

  # Put a message box on the dialog
  set promptLen [string length $promptText]
  if {$promptLen > 40} {
    set promptLen [expr $promptLen / 2]
  }

  message $dlg.msg -text $promptText \
                   -width [expr $promptLen * 12] \
                   -justify center \
                   -bg bisque2 \
                   -font $tsiConfig(dialogFont)

  # Add an entry widget, which shows the old value by default

  set prompt(result) $currentVal
  entry $dlg.entry -textvariable prompt(result) -font $tsiConfig(dialogFont)

  # set up a widget for the buttons

  set b [frame $dlg.buttons -bd 10 -bg bisque2 -width $promptLen]
  pack $dlg.msg $dlg.entry $dlg.buttons -side top

  # add the ok and cancel buttons, making them the same size

  set bWidth [string length {Cancel}]

  button $b.ok -text Ok -command {set prompt(ok) 1} \
                        -width $bWidth \
                        -font $tsiConfig(dialogFont)
  button $b.cancel -text Cancel -command {set prompt(ok) 0} \
                   -width $bWidth \
                   -font $tsiConfig(dialogFont)
  pack $b.ok $b.cancel -side left -anchor center

  # stick the cursor on the entry widget, wait for a button to be pressed,
  # then remove the dialog

  focus $dlg.entry
  grab $dlg
  tkwait variable prompt(ok)
  grab release $dlg
  destroy $dlg

  # sort out what to return - if the OK button is pressed and a new value
  # was entered, return that; otherwise return the old value.

  if {$prompt(ok)} {
    if {[string compare $prompt(result) {}] != 0} {
      return $prompt(result)
    } else {
      return $currentVal
    }
  } else {
    return $currentVal
  }
}

# The AddWmeDialog enables the user to enter a series of wmes
# or wme patterns so they can be added to an agent's working
# memory.  This dialog contains fields to enter the object
# attribute, value items.  There is also a checkbutton for
# acceptable preferences.  The attribute and value fields are
# initialized to "*" the acceptable wildcard pattern.  A few
# buttons trigger the add-wme command and termination of the
# dialog.

proc AddWmeDialog {{w .add_wme_dialog}} {
    global add_wme_info
    catch {destroy $w}

    set add_wme_info(id)   {}
    set add_wme_info(attr) {*}
    set add_wme_info(val)  {*}
    set add_wme_info(pref) 0

    toplevel $w
    wm title $w {Add Working Memory Element}
    wm iconname $w {Add WME}

    frame $w.frame -borderwidth 4
    entry $w.frame.id -width 5 -relief sunken -textvariable add_wme_info(id)
    label $w.frame.label -text {^}
    entry $w.frame.attr -width 20 -relief sunken -textvariable \
                                                      add_wme_info(attr)
    entry $w.frame.val  -width 20 -relief sunken -textvariable add_wme_info(val)
    checkbutton $w.frame.pref -text {+} -variable add_wme_info(pref)

    button $w.doit -text {Add WME Now} \
      -command "addWmeGraphically $w \$add_wme_info(id) \$add_wme_info(attr)
                                     \$add_wme_info(val) \$add_wme_info(pref)"

    text $w.result -height 1 -width 20 -relief flat -state disabled

    button $w.ok -text Dismiss -command "destroy $w"

    pack $w.frame.id -side left -padx 1m -pady 2m
    pack $w.frame.label -side left -pady 2m
    pack $w.frame.attr $w.frame.val $w.frame.pref \
         -side left -padx 1m -pady 2m
    pack $w.frame -side top
    pack $w.doit -side left
    pack $w.result -side left
    pack $w.ok -side right
}

proc addWmeGraphically {w id attr val pref} {
#   $w.result configure -state normal
#   $w.result delete 1.0 end
#   output-strings-destination -push -text-widget $w.result
   if {$pref == 1} {
      tsiDisplayAndSendCommand "add-wme $id $attr $val +"
   } else {
      tsiDisplayAndSendCommand "add-wme $id $attr $val"
   }
#   output-strings-destination -pop
#   $w.result see 1.0
#   $w.result configure -state disabled
}

##### Search facility

proc searchWindow {w} {
   global interp_name tsiConfig
   if [winfo exists $w.search] {
      wm deiconify $w.search
      raise $w.search
      return
   }
   toplevel $w.search
   wm title $w.search "Text Search: Agent $interp_name"
   wm iconname $w.search "$interp_name search"

   frame $w.search.text
   entry $w.search.text.t -width 20 -relief sunken -bd 2 \
         -font $tsiConfig(searchTextFont) \
         -textvariable searchString($w.t) \
         -xscrollcommand "$w.search.text.s set"
   scrollbar $w.search.text.s -relief flat -orient horizontal \
             -command "$w.search.text.t xview"
   pack $w.search.text.t $w.search.text.s -fill x

   frame $w.search.buttons
   frame $w.search.buttons.left
   frame $w.search.buttons.right
   button $w.search.buttons.left.restart -font $tsiConfig(searchButtonsFont) \
          -text {Restart} \
          -command "clearSearch $w.t"
   button $w.search.buttons.left.clear -font $tsiConfig(searchButtonsFont) \
          -text {Clear} \
          -command "set searchString($w.t) \"\"; clearSearch $w.t"
   button $w.search.buttons.right.up -font $tsiConfig(searchButtonsFont) \
          -text {Search Up} \
          -command "findNext $w.t -1"
   button $w.search.buttons.right.down -font $tsiConfig(searchButtonsFont) \
          -text {Search Down} \
          -command "findNext $w.t +1"

   pack $w.search.buttons.left.restart $w.search.buttons.left.clear -fill x
   pack $w.search.buttons.right.up $w.search.buttons.right.down -fill x

   pack $w.search.buttons.left $w.search.buttons.right -side left

   frame $w.search.cbuttons
   button $w.search.cbuttons.hide -text Hide -command "wm withdraw $w.search"
   button $w.search.cbuttons.dismiss -text Dismiss -command "destroy $w.search"
   pack $w.search.cbuttons.hide $w.search.cbuttons.dismiss -side left

   pack $w.search.cbuttons $w.search.text $w.search.buttons -side bottom
  # No click to focus
   bind $w.search <Enter> "
       focus $w.search.text.t
   "
}

