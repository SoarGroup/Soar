##########################################################################
# File         : soar-menu.tcl
# Author       : Frank E. Ritter (with help from GDB)
# Date	       : 30-Oct-96
# Last Update  : 01-Apr-97 by Thomas Head
# Description  : 
#
# Takes two real arguments, window title and list of items to choose.
##########################################################################
#

proc soar-menu {{title "Soar-menu"} {menuitems {1 2 3}} \
        {w .soarmenu} {maxw 0}} {
    global soarmenuResult
    set soarmenuResult ""
    catch {destroy $w}

    toplevel $w
    wm title $w $title
    wm iconname $w $title
    set maxw [string length $title]
    incr maxw 2
    foreach item $menuitems {
        if {[string length $item] > $maxw} {
           set maxw [string length $item]}}

    frame $w.frame -borderwidth 4
    foreach item $menuitems {
      eval "\
      button .soarmenu.$item -text $item -width $maxw \
             -command {destroy .soarmenu; set soarmenuResult $item}"
      pack .soarmenu.$item -side top}

  if {"$soarmenuResult" == ""} {
    # wait for the box to be destroyed
    update idletask
    grab .soarmenu
    tkwait window .soarmenu}
  ## echo ">>" $soarmenuResult "<<"
  return $soarmenuResult}

