#--------------------------------------------------------------------------

proc setUpSoarTerm w {
    setSoarPrompt $w
    termtext $w -relief raised -bd 2 -setgrid false

    bind $w <Escape> {tsiDisplayAndSendCommand stop}

    $w tag configure searchFind -background lightblue

    soarPopUp $w
}

proc setSoarPrompt {w} {
    global interp_name interp_type
#    termTextSetPrompt $w "$interp_type $interp_name> "
    termTextSetPrompt $w "$interp_name> "
}

# Our redefinition of the termTextAddText procedure logs the text to
# the current log file if logging is on
proc termTextAddText {w t {tags {}}} {
    if {$tags == {}} {
        $w insert insert $t
    } else {
        $w insert insert $t $tags
    }
    if {[log --query] == {open}} {
        log --add $t
    }
}

### Written by Karl Schwamb for the original Soar prototype GUI
### Tweaked a little bit by Randolph M. Jones for the Tcl/Tk Soar Interface
# This procedure converts the text in a text widget from a raw nroff
# output form to a prettier form.  The text in the widget is scanned
# for nroff output format controls such as the following:
#
#   Underscored character: <underscore><backspace><char>
#   Bolded character: <char><backspace><char><backspace><char><backspace><char>
#   Trash: <escape>9
#
# Since this scanning process can be slow, we update idletasks as we
# process the lines.  The basic process is to find one of the above
# sequences, delete the control characters, and highlight the character
# according to its intended format by using text widget tags.
#
# This procedure needs to be made faster.  This could be done by 
# modifying a line for all changes before writting characters back
# into the text widget.  This requires that we generate a table mapping
# character indices to formatting tags and then using that to adjust
# the text widget.

proc ScanTextAndHighlight {w {start 1} {finish -1}} {
    global tsiConfig
    $w config -cursor watch
 
    if {$finish == -1} {
        scan [$w index end] %d finish
    }
    set begin 0

    for {set i $start} {$i < $finish} {incr i} {
	set text [$w get $i.$begin "$i.$begin lineend"]  
        set check_line 1

        # this could take a while - so keep up to date on events!
        update 

        while {$check_line} {

	    set check_line 0

	    if {[regexp -indices -nocase "\x1b\[9\]" \
		    [string range $text $begin end] indices] == 1} {
                set left [expr $begin + [lindex $indices 0]]
                set right [expr $begin + [lindex $indices 1]]
                $w delete $i.$left $i.$right+1c
		
                set text [$w get $i.$begin "$i.$begin lineend"] 
	        set check_line 1
            }

	    if {[regexp -indices -nocase "(.)\x08(.)\x08(.)\x08(.)" \
		    [string range $text $begin end] indices one two three four] == 1} {
		set one   [string range $text [lindex $one 0] [lindex $one 1]]
		set two   [string range $text [lindex $two 0] [lindex $two 1]]
		set three [string range $text [lindex $three 0] [lindex $three 1]]
		set four  [string range $text [lindex $four 0] [lindex $four 1]]
		if {([string compare $one $two] == 0) &&   \
			([string compare $one $three] == 0) && \
			([string compare $one $four] == 0)} {
		    set left [expr $begin + [lindex $indices 0]]
		    set right [expr $begin + [lindex $indices 1]]
		    $w delete $i.$left $i.$right

		    $w tag add bold "$i.$left" "$i.$left+1c"

		    set text [$w get $i.$begin "$i.$begin lineend"] 
		    set check_line 1
		}
	    }

	    if {[regexp -indices -nocase "_\x08." \
		    [string range $text $begin end] indices] == 1} {
		set left [expr $begin + [lindex $indices 0]]
		set right [expr $begin + [lindex $indices 1]]
		$w delete $i.$left $i.$right
		
		$w tag add italic "$i.$left" "$i.$left+1c"

		set text [$w get $i.$begin "$i.$begin lineend"] 
		set check_line 1
	    }
	}

    }

     $w tag configure bold -font $tsiConfig(boldFont)
     $w tag configure italic -font $tsiConfig(italicFont)

     $w config -cursor xterm
}

# The ShowFile procedure shows a file to the user by creating a
# new text window and placing the contents of the file in the 
# window.  If an additional argument is given, it indicates
# whether a nroff man_page is being shown or not.  If not, then 
# we just dump the file contents into the window.  If so, then
# we first run nroff on the file and remove backspaces and 
# escapes.

proc ShowFile {file {man_page 0}} {
    global fileWindowCounter tsiConfig
    if {[file isfile $file] == 1} {
        if {[info exists fileWindowCounter] == 0} {
            set fileWindowCounter 0
        }
	set w ".showfile$fileWindowCounter"
	incr fileWindowCounter

	catch {destroy $w}

	toplevel $w 
        wm title $w $file
	wm iconname $w $file
	wm minsize $w 0 0

	text $w.text -width 85 -setgrid 1 -bd 2 \
                     -yscrollcommand "$w.scroll set"  \
                     -font $tsiConfig(normalFont)
	scrollbar $w.scroll -command "$w.text yview"
	button $w.ok -text Dismiss -command "destroy $w"

	pack $w.ok -side bottom -fill x
	pack $w.scroll -side right -fill y
	pack $w.text -side left -expand 1 -fill both

        bind $w.text <space> {
           tkTextSetCursor %W [tkTextScrollPages %W 1]
        }
        bind $w.text <Return> "
            $w.ok invoke
            break
        "
        bind $w <Enter> "
           focus $w.text
        "

	$w.text delete 1.0 end
	if {$man_page == 0} {
	    set f [open $file]
	    while {![eof $f]} {
		$w.text insert end [read $f 1000]
	    }
	    close $f
	    ScanTextAndHighlight $w.text
	} else {
	    cd [file dirname $file]
	    #regsub -all (\x08|(\x1b)9) [exec nroff -man $file] {} page
	    #$w.text insert end $page
	    $w.text insert end [exec nroff -man $file]
	    ScanTextAndHighlight $w.text
	}
        $w.text configure -state disabled
    }
}

# This procedure asks the user for a man page to display and 
# then shows it to the user in a separate window.

proc ShowManPage {dir {use_nroff 1}} {
    set file [tk_getOpenFile -initialdir $dir -title {Show Man Page:}]
    if {$file != {}} {
	ShowFile "$file" $use_nroff
    }
}

proc findNext {w dir} {
    global searchString searchPoint
    if {$searchString($w) == {}} return
    if {$dir < 0} {
        if {[info exists searchPoint($w)] == 0} {
            set searchPoint($w) [$w index {end - 1 chars}]
            set i [$w search -backwards -nocase $searchString($w) \
                              $searchPoint($w) 1.0]
        } else {
            set i [$w search -backwards -nocase $searchString($w) \
                              "$searchPoint($w) - 1 chars" 1.0]
        }
    } else {
        if {[info exists searchPoint($w)] == 0} {
            set searchPoint($w) [$w index 1.0]
            set i [$w search -nocase $searchString($w) \
                              $searchPoint($w) end]
        } else {
            set i [$w search -nocase $searchString($w) \
                              "$searchPoint($w) + 1 chars" end]
        }
    }
    if {$i != {}} {
        set searchPoint($w) $i
        $w tag remove searchFind 1.0 end
        $w tag add searchFind $i "$i + [string length $searchString($w)] chars"
        $w see $i
    }
}

proc clearSearch w {
    global searchPoint
    catch {unset searchPoint($w)}
    $w tag remove searchFind 1.0 end
}

