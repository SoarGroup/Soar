#
# Monitor demo
#
# This demo shows the result of a command being shown in
# a window as Soar executes.  This is a recreation of the
# functionality of the previous (pre-Tcl) multi-agent
# (with X) monitor command.
#

proc mkMonitor {{w .mon} {c {print -stack -operator}}} {
    catch {destroy $w}

    set monitor_name [monitor -add after-decision-cycle \
                              [list ShowInMonitor $w $c]]

    toplevel $w
    wm title $w "Monitor $monitor_name"
    wm iconname $w "Monitor $monitor_name"
    wm minsize $w 1 1

    text $w.text -font -adobe-courier-bold-o-normal--14-*-*-*-*\
         -width 60 -height 10 -bd 2 -yscrollcommand "$w.scroll set"
    scrollbar $w.scroll -command "$w.text yview"
    button $w.ok -text Dismiss \
     -command "destroy $w; monitor -delete after-decision-cycle $monitor_name"
    
    pack $w.ok -side bottom -fill x
    pack $w.scroll -side right -fill y
    pack $w.text -side left -expand 1 -fill both
}

proc ShowInMonitor {w command} {
    output-strings-destination -push -append-to-result
    $w.text delete 1.0 end
    $w.text insert end [eval $command]
    output-strings-destination -pop
    update
}

mkMonitor