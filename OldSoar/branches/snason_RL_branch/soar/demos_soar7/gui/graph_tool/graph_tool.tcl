#
# Graph demo
#

global soar_library graphtool_dir graphtool graphtool_path blt_app

set graphtool_dir $soar_library/../demos/gui/graph_tool
set graphtool soargraph
set graphtool_path $graphtool_dir/$graphtool
set blt_app $soar_library/../../blt-1.8/blt_wish

proc test-shell {} {
    global default graphtool_dir
    excise -all
    source $default
    source $graphtool_dir/farmer.soar
    init-soar
    init-graph
    monitor -add after-decision-cycle show-data
    d
}


proc show-data {} {
    global graphtool
    set x     [stats -system -dc-count]
    set wme   [stats -memory -pool wme -used]
    set chunk [stats -system -chunk-count]

    send -async $graphtool "line_add wmes $x $wme ; line_add chunks $x $chunk"
}


proc init-graph {} {
    global graphtool
    send -async $graphtool "option add *graph.xTitle \"Decision Cycle\"
                            option add *graph.yTitle \"Y\"
                            option add *graph.title \"SoarGraph\" "
    init-line wmes   circle 1
    init-line chunks plus   3
}


proc init-line {line symbol linewidth} {
    global graphtool
    set names [send $graphtool {$graph element names}]
    if {[lsearch $names $line] >= 0} {
	send -async $graphtool "\$graph element delete $line"
    }
    send -async $graphtool "set ${line}(x) {}
                            set ${line}(y) {} 
                            \$graph element create $line \
	                            -xdata \$${line}(x)  \
                                    -ydata \$${line}(y)  \
                                    -symbol $symbol      \
                                    -linewidth $linewidth "
}

proc terminate-graphing-tool {} {
    global graphtool
    if {[lsearch [winfo interps] $graphtool] >= 0} {
        echo "Terminating graphing tool ..."
	send -async $graphtool exit
        while (1) {
	    if {[lsearch [winfo interps] $graphtool] < 0} {
		break
	    }
	}
    }
}

proc fire-up-graphing-tool {} {
    global graphtool graphtool_path blt_app
    terminate-graphing-tool

    exec $blt_app -f $graphtool_path &

    echo "Starting up graphing tool ..."
    while (1) {
	set interps [winfo interps]
	if {[lsearch $interps $graphtool] >= 0} {
	    break
	}
    }
}


proc mkGraphToolDemo {{w .graphtool}} {
    global blt_app
    if ![file exists $blt_app] {
	tk_dialog .dlg "TkSoar Notification" \
              "Sorry, the Graph Tool Demo needs the BLT application which cannot be found." \
              error 0 {Dismiss}
      return
    }
    catch {destroy $w}
    toplevel $w
    wm title $w "Graph Tool Demonstation"
    wm iconname $w "Graph Tool"

    label $w.msg -font -Adobe-times-medium-r-normal--*-180* \
            -text "Graph Tool Demonstration" -padx 10 -pady 10

    frame $w.frame -borderwidth 10 

    button $w.frame.run -text "Run Graph Demo" -command {test-shell}
    button $w.frame.stop -text "Stop" -command "stop-soar"
    button $w.frame.go -text "Continue" -command "d"
    button $w.frame.restart -text "Restart Graphing Tool" \
	    -command {fire-up-graphing-tool}

    button $w.ok -text Dismiss \
           -command "destroy $w; terminate-graphing-tool; monitor -clear"

    pack $w.frame.run $w.frame.stop $w.frame.go $w.frame.restart -side top
    pack $w.msg $w.frame $w.ok -side top

    fire-up-graphing-tool

    monitor -clear
    monitor -add after-decision-cycle update
}

mkGraphToolDemo












