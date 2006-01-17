##########################################################################
# File         : termText.tcl
# Author       : Randolph M. Jones
# Date         : 9-29-96 US, 29-9-96 non-US
# Description  :
#
# This file consists of modifications to the standard Tk Text widget.
# Thus, portions of the code have been borrowed heavily from that widget,
# and are subject to the following copyrights, as well as the rules
# on usage and redistribution found in the file license.terms included
# in the Tk distribution.
#
# Copyright (c) 1992-1994 The Regents of the University of California.
# Copyright (c) 1994-1995 Sun Microsystems, Inc.
#
# The main idea of these modifications is to divide the text window
# into two sections, divided by the ``current prompt''.  All text
# before (and including) the current prompt is read-only.  All text
# after the prompt is fully editable by the user.  If new text arrives
# to the widget from somewhere other then the user, the current prompt
# becomes an ``old'' prompt, and a new ``current'' prompt is created
# the next time the user types something.  When the new prompt is created,
# a new copy of the user's text typed so far (since the old current prompt)
# will be generated.  Thus, the user can keep working on a command line
# without any nasty interactions with other text that is getting added
# to the text window by other processes.
#
# There are also a number of mostly cosmetic changes to the standard
# text widget.
#
##########################################################################

# Usage: termtext <widget-id> [flags to be passed along to the text widget]
# Make sure you set the prompt you want with termTextSetPrompt *before*
# creating the termtext widget with this procedure call.
proc termtext {w args} {
    set ret [eval "text $w $args"]
    termTextBindings $w
    if [catch {termTextSetPrompt $w}] {
        termTextSetPrompt $w {% }
    }
    termTextPrompt $w
    return $ret
}

proc termTextSetPrompt {w args} {
    global termTextPromptValue
    if {$args == {}} {
        return termTextPromptValue($w)
    }
    return [eval set termTextPromptValue($w) $args]
}

# Special bindings for termText widgets, that are different from the
# normal text widget (most of them are different).  We have to put
# "break" at the end of each of these, because in tk4.0 and later,
# the TAGged bindings will get called first and then the WIDGET bindings
# will get called second, but we don't want the normal text widget
# bindings to get called at all.
proc termTextBindings w {
global tk_version

############################
### Mouse bindings

# These button-1 bindings have been set up to act like the standard
# button-1 bindings for an X terminal (at least this is the way they
# behave in my environment with the MWM window manager).

bind $w <1> {
    set ::tk::Priv(selectMode) char
    set ::tk::Priv(mouseMoved) 0
    set ::tk::Priv(x) %x
    set ::tk::Priv(y) %y
    %W tag remove sel 1.0 end
    focus %W
    break
}
bind $w <B1-Motion> {
    if !$::tk::Priv(mouseMoved) {
       %W mark set anchor @$::tk::Priv(x),$::tk::Priv(y)
    }
    if {(%x != $::tk::Priv(x)) || (%y != $::tk::Priv(y))} {
        set ::tk::Priv(mouseMoved) 1
        set ::tk::Priv(x) %x
        set ::tk::Priv(y) %y
        termTextSelectTo %W %x %y
    }
    break
}

# Here's what I have observed (at least in my environment...hopefully this
# behavior isn't platform dependent, but is uniform in Tk):
# Clicking a button once calls the regular single-click binding (e.g., <1>
# for mouse button 1)
# Clicking twice quickly DOES NOT call the single-click binding, but
# does call the Double binding (e.g., <Double-1)
# Clicking three or more times quickly FIRST calls the Double- binding
# (after the second click) and then calls the Triple- binding FOR EACH
# SUBSEQUENT CLICK

# The following bindings depend on this behavior to implement the
# xterm-like actions of button one (similarly with button 3).
bind $w <Double-1> {
    %W mark set anchor @$::tk::Priv(x),$::tk::Priv(y)
    set ::tk::Priv(mouseMoved) 1
    set ::tk::Priv(selectMode) word
    termTextSelectTo %W %x %y
    break
}
bind $w <Triple-1> {
    %W mark set anchor @$::tk::Priv(x),$::tk::Priv(y)
    set ::tk::Priv(mouseMoved) 1
    set ::tk::Priv(selectMode) [termTextStepUpSelectMode $::tk::Priv(selectMode)]
    termTextSelectTo %W %x %y
    break
}
bind $w <Shift-1> {
#delete normal text binding
    break
}
bind $w <Double-Shift-1>	{
#delete normal text binding
    break
}
bind $w <Triple-Shift-1>	{
#delete normal text binding
    break
}
bind $w <ButtonRelease-1> {
#    tkCancelRepeat
    ::tk::CancelRepeat

    if !$::tk::Priv(mouseMoved) {
        termTextMaybeSetCursor %W %x %y
    }
    break
}
bind $w <Control-1> {
    termTextMaybeSetCursor %W %x %y
    break
}

# These are just like the normal text bindings, but we want them loaded
# regardless of whether tk_strictMotif is set, because we like them.
bind $w <2> {
    %W scan mark %x %y
    set ::tk::Priv(x) %x
    set ::tk::Priv(y) %y
    set ::tk::Priv(mouseMoved) 0
    break
}
bind $w <B2-Motion> {
    if {(%x != $::tk::Priv(x)) || (%y != $::tk::Priv(y))} {
        set ::tk::Priv(mouseMoved) 1
    }
    if $::tk::Priv(mouseMoved) {
        %W scan dragto %x %y
    }
    break
}
# ButtonRelease-2 differs from the standard text binding because it
# uses termTextInsertByLines when pasting text (because pasted text
# might have multiple commands in it)
bind $w <ButtonRelease-2> {
    if !$::tk::Priv(mouseMoved) {
        catch {termTextInsertByLines %W [selection get -displayof %W]}
    }
    break
}

# These bindings have been set up to make button 3 act like the standard
# behaviors for button 3 in an Xterm (that is, they control extending
# the current selection)
bind $w <3> {
    ::tk::TextResetAnchor %W @%x,%y
    termTextSelectTo %W %x %y
    break
}
bind $w <B3-Motion> {
    termTextSelectTo %W %x %y
    break
}
bind $w <Double-3> {
    set ::tk::Priv(selectMode) [termTextStepUpSelectMode $::tk::Priv(selectMode)]
    termTextSelectTo %W %x %y
    break
}
bind $w <Triple-3> {
    set ::tk::Priv(selectMode) [termTextStepUpSelectMode $::tk::Priv(selectMode)]
    termTextSelectTo %W %x %y
    break
}

############################
### Keyboard bindings

# All the bindings that can set the cursor position, or insert or delete
# text must also call termTextRewriteUserText to rewrite the prompt and current
# input text in case some terminal output has come in since we last typed.
bind $w <Left> {
    termTextRewriteUserText %W
    termTextSetCursor %W [%W index {insert - 1c}]
    break
}
bind $w <Right> {
    termTextRewriteUserText %W
    termTextSetCursor %W [%W index {insert + 1c}]
    break
}

# this variable implements the interactive history, it is normally set
# by the termTextProcessCommand procedure
uplevel #0 set currentHistory 0

bind $w <Up> {
    %W delete {insert linestart} {insert lineend}
    termTextRewriteUserText %W
    termTextSetCursor %W [termTextUpDownLine %W -1]

    if { $currentHistory > 1 } {

        uplevel #0 set currentHistory [expr $currentHistory - 1]
    }
    if { $currentHistory > 0 } {
        termTextInsert %W [history event $currentHistory]
    }
    break
#    termTextRewriteUserText %W
#    termTextSetCursor %W [termTextUpDownLine %W -1]
#    break
}
bind $w <Down> {
    %W delete {insert linestart} {insert lineend}
    termTextRewriteUserText %W
    termTextSetCursor %W [termTextUpDownLine %W 1]
    if { $currentHistory > 0 } {
        if { $currentHistory < [history nextid] } {
            uplevel #0 set currentHistory [expr $currentHistory + 1]
            if { $currentHistory < [history nextid] } {
                termTextInsert %W [history event $currentHistory]
            }
        }
    }
    break
#    termTextRewriteUserText %W
#    termTextSetCursor %W [termTextUpDownLine %W 1]
#    break
}

bind $w <Control-Left> {
    termTextRewriteUserText %W
    termTextSetCursor %W [%W index {insert - 1c wordstart}]
    break
}
bind $w <Control-Right> {
    termTextRewriteUserText %W
    termTextSetCursor %W [%W index {insert wordend}]
    break
}
bind $w <Control-Up> {
    termTextRewriteUserText %W
    termTextSetCursor %W [::tk::TextPrevPara %W insert]
    break
}
bind $w <Control-Down> {
    termTextRewriteUserText %W
    termTextSetCursor %W [::tk::TextNextPara %W insert]
    break
}
bind $w <Prior> {
   %W yview scroll -1 pages
    break
}
bind $w <Next> {
   %W yview scroll 1 pages
    break
}

bind $w <Home> {
    termTextRewriteUserText %W
    termTextSetCursor %W {insert linestart}
    break
}
bind $w <End> {
    termTextRewriteUserText %W
    termTextSetCursor %W {insert lineend}
    break
}
bind $w <Control-Home> {
    termTextRewriteUserText %W
    termTextSetCursor %W 1.0
    break
}
bind $w <Control-End> {
    termTextRewriteUserText %W
    termTextSetCursor %W {end - 1 char}
    break
}

bind $w <Tab> {
    termTextRewriteUserText %W
    termTextDeleteSelectedText %W
    termTextInsert %W \t
    break
}
bind $w <Control-i> {
    termTextRewriteUserText %W
    termTextDeleteSelectedText %W
    termTextInsert %W \t
    break
}

# If some text is selected, delete and backspace simply delete the selected
# text, but it's not allowed to delete any selected text that is before
# the end of the prompt.
bind $w <Delete> {
    termTextRewriteUserText %W
    if {[termTextDeleteSelectedText %W] == 0} {
	%W delete insert
	%W see insert
    }
    break
}
bind $w <BackSpace> {
    termTextRewriteUserText %W
    if {[termTextDeleteSelectedText %W] == 0} {
        if [%W compare insert != {promptEnd + 1 chars}] {
	    %W delete insert-1c
	    %W see insert
        }
    }
    break
}
bind $w <Control-h> {
    termTextRewriteUserText %W
    if {[termTextDeleteSelectedText %W] == 0} {
        if [%W compare insert != {promptEnd + 1 chars}] {
	    %W delete insert-1c
	    %W see insert
        }
    }
    break
}
bind $w <Control-u> {
    termTextRewriteUserText %W
    %W delete {promptEnd + 1 chars} insert
    %W see insert
    break
}

bind $w <Insert> {
    termTextRewriteUserText %W
    catch {termTextInsertByLines %W [selection get -displayof %W]}
    break
}

# The return key gets to be special, because it causes the possible
# invocation of the terminal process, by sending the current user-input
# line as a command
bind $w <Return> {
    termTextRewriteUserText %W
    termTextInsert %W \n
    termTextInvoke %W
    break
}

# Any other text just gets inserted, possibly causing the prompt-line
# to be rewritten
bind $w <KeyPress> {
    if {"%A" != "{}"} {
       termTextRewriteUserText %W
       termTextDeleteSelectedText %W
       termTextInsert %W %A
    }
    break
}

# Ignore all Alt, Meta, and Control keypresses unless explicitly bound.
# Otherwise, if a widget binding for one of these is defined, the
# <KeyPress> class binding will also fire and insert the character,
# which is wrong.  Ditto for <Escape>.

bind $w <Alt-KeyPress> {# nothing }
bind $w <Meta-KeyPress> {# nothing}
bind $w <Control-KeyPress> {# nothing}
bind $w <Escape> {# nothing}
bind $w <KP_Enter> {# nothing}

# We don't allow any of the bindings that do selection with keystrokes
# in the normal text widget, because that doesn't interact well with
# not allowing the cursor to get set to a point before the current
# prompt
bind $w <Shift-Left> {
    break
}
bind $w <Shift-Right> {
    break
}
bind $w <Shift-Up> {
    break
}
bind $w <Shift-Down> {
    break
}
bind $w <Shift-Control-Left> {
    break
}
bind $w <Shift-Control-Right> {
    break
}
bind $w <Shift-Control-Up> {
    break
}
bind $w <Shift-Control-Down> {
    break
}
bind $w <Shift-Prior> {
    break
}
bind $w <Shift-Next> {
    break
}
bind $w <Shift-Home> {
    break
}
bind $w <Shift-End> {
    break
}
bind $w <Control-Shift-Home> {
    break
}
bind $w <Control-Shift-End> {
    break
}
bind $w <Control-Shift-space> {
    break
}
bind $w <Shift-Select> {
    break
}

# Additional emacs-like bindings, many similar to the ones provided in
# the normal text widget:
bind $w <Control-a> {
    termTextRewriteUserText %W
    termTextSetCursor %W {insert linestart}
    break
}
bind $w <Control-b> {
    termTextRewriteUserText %W
    termTextSetCursor %W insert-1c
    break
}
bind $w <Control-d> {
    termTextRewriteUserText %W
    %W delete insert
    break
}
bind $w <Control-e> {
    termTextRewriteUserText %W
    termTextSetCursor %W {insert lineend}
    break
}
bind $w <Control-f> {
    termTextRewriteUserText %W
    termTextSetCursor %W insert+1c
    break
}
bind $w <Control-k> {
    termTextRewriteUserText %W
    if [%W compare insert == {insert lineend}] {
        %W delete insert
    } else {
        %W delete insert {insert lineend}
    }
    break
}
bind $w <Control-n> {
    %W delete {insert linestart} {insert lineend}
    termTextRewriteUserText %W
    termTextSetCursor %W [termTextUpDownLine %W 1]
    if { $currentHistory > 0 } {
        if { $currentHistory < [history nextid] } {
            uplevel #0 set currentHistory [expr $currentHistory + 1]
            if { $currentHistory < [history nextid] } {
                termTextInsert %W [history event $currentHistory]
            }
        }
    }
    break
#    termTextRewriteUserText %W
#    termTextSetCursor %W [termTextUpDownLine %W 1]
#    break
}
bind $w <Control-o> {
# emacs control-O just doesn't work well when \n is supposed to invoke
# a command, so we won't allow it
    break
}
bind $w <Control-p> {
    %W delete {insert linestart} {insert lineend}
    termTextRewriteUserText %W
    termTextSetCursor %W [termTextUpDownLine %W -1]
    if { $currentHistory > 1 } {
        uplevel #0 set currentHistory [expr $currentHistory - 1]
    }
    if { $currentHistory > 0 } {
        termTextInsert %W [history event $currentHistory]
    }
    break
#    termTextRewriteUserText %W
#    termTextSetCursor %W [termTextUpDownLine %W -1]
#    break
}
bind $w <Control-t> {
    termTextRewriteUserText %W
    ::tk::TextTranspose %W
    break
}
bind $w <Control-v> {
    %W yview scroll -1 pages
    break
}
# Inserted by Mazin for his personal copy to override
# control-v to be paste command and control-c to by copy

bind $w <Control-v> {
    termTextPaste %W
    break
}

# Will want to remove this for release or at least remap to quit-soar
bind $w <Control-q> {
    exit
    break
}

bind $w <Control-C> {
	clipboard clear
	clipboard append {[selection get]}
    break
}

bind $w <Meta-v> {
    %W yview scroll 1 pages
    break
}
bind $w <Meta-b> {
    termTextRewriteUserText %W
    termTextSetCursor %W {insert - 1c wordstart}
    break
}
bind $w <Meta-d> {
    termTextRewriteUserText %W
    %W delete insert {insert wordend}
    break
}
bind $w <Meta-f> {
    termTextRewriteUserText %W
    termTextSetCursor %W {insert wordend}
    break
}
bind $w <Meta-less> {
    termTextRewriteUserText %W
    termTextSetCursor %W 1.0
    break
}
bind $w <Meta-greater> {
    termTextRewriteUserText %W
    termTextSetCursor %W end-1c
    break
}
bind $w <Meta-BackSpace> {
    termTextRewriteUserText %W
    %W delete {insert -1c wordstart} insert
    break
}
bind $w <Meta-Delete> {
    termTextRewriteUserText %W
    %W delete {insert -1c wordstart} insert
    break
}

bind Text <<Cut>> {
    termTextCut %W
    break
}

bind Text <<Paste>> {
    termTextPaste %W
    break
}

### Don't allow clearing for now
bind Text <<Clear>> {
    break
}

### END OF proc termTextBindings
}

# This is very similar to tkTextSelectTo, but it does not check whether
# the mouse has moved.  The test comes in the mouse button bindings now.
proc termTextSelectTo {w x y} {
    global ::tk::Priv

    set cur [$w index @$x,$y]
    if [catch {$w index anchor}] {
	$w mark set anchor $cur
    }
    set anchor [$w index anchor]
    switch $::tk::Priv(selectMode) {
	char {
	    if [$w compare $cur < anchor] {
		set first $cur
		set last anchor
	    } else {
		set first anchor
		set last [$w index "$cur + 1c"]
	    }
	}
	word {
	    if [$w compare $cur < anchor] {
		set first [$w index "$cur wordstart"]
		set last [$w index {anchor - 1c wordend}]
	    } else {
		set first [$w index {anchor wordstart}]
		set last [$w index "$cur wordend"]
	    }
	}
	line {
	    if [$w compare $cur < anchor] {
		set first [$w index "$cur linestart"]
		set last [$w index {anchor - 1c lineend + 1c}]
	    } else {
		set first [$w index {anchor linestart}]
		set last [$w index "$cur lineend + 1c"]
	    }
	}
    }
    if {$::tk::Priv(mouseMoved) || ($::tk::Priv(selectMode) != {char})} {
	$w tag remove sel 1.0 $first
	$w tag add sel $first $last
	$w tag remove sel $last end
	update idletasks
    }
}

# This procedure is very similar to tkTextSetCursor, except that it does
# not allow the cursor to go left of (or above) the end of the current
# prompt.  It also sets the special "userCursor" mark, to keep track
# of where the user is in the prompt-input line, in case the user text
# has to be rewritten (by termTextRewriteUserText).
# Finally, tkTextSetCursor clears the current selection, but we don't
# bother to do that, because we have disabled keystroke selection.
proc termTextSetCursor {w pos} {
    global ::tk::Priv

    if [$w compare $pos >= end] {
	set pos {end - 1 chars}
    }

    if [$w compare $pos <= promptEnd] {
        set pos {promptEnd + 1 chars}
    }

    $w mark set insert $pos
    $w mark set userCursor insert
    $w see insert
}

# This is similar to tkTextInsert for normal text widgets, but it does
# not allow selected text before the end of the prompt to be deleted.
# In addition, inserting "\n" should always occur at the end of the
# text instead of at the current insert point, because "\n" signals
# a possible invocation of the terminal's process.
proc termTextInsert {w s} {
    if {($s == {}) || ([$w cget -state] == {disabled})} {
	return
    }
    if {[$w tag nextrange sel {promptEnd + 1 chars} end] != {}} {
        $w mark set insert sel.last
        if [$w compare sel.first > promptEnd] {
	    $w delete sel.first sel.last
        } else {
            $w delete {promptEnd + 1 chars} sel.last
        }
    }
    if {$s == "\n"} {
        $w mark set insert end
        termTextAddText $w $s userText
    } else {
        termTextAddText $w $s userText
    }
    $w mark set userCursor insert
    $w see insert
}

# termTextInsertByLines -- (RMJ)
# Insert text line by line, calling termTextInvoke after each line, just in
# case the line completes a command.
# This procedure should be called by actions that paste text, because
# pasted text might contain multiple commands on different lines.
# Regular typed text should be caught as soon as the user types "\n",
# so it doesn't have to pass through this procedure, but can go directly
# to termTextInsert
proc termTextInsertByLines {w s} {
    if {($s == {}) || ([$w cget -state] == {disabled})} {
        return
    }
    for {set i [string first \n $s]} {$i >= 0} {set i [string first \n $s]} {
        set cmd [string range $s 0 $i]
        set s [string range $s [expr $i + 1] [string length $s]]
        termTextRewriteUserText $w
        termTextInsert $w $cmd
        termTextInvoke $w
    }
    if {$s != {}} {
        termTextRewriteUserText $w
        termTextInsert $w $s
    }
}

# This is very similar to tkTextUpDownLine, returning a cursor position
# above or below the current position, but keeping track of the position
# on short lines (so you can maintain the column with a series of up and
# down moves).  The difference in this procedure is that it adjusts the
# cursor on the first line of the user text, to take the length of the
# prompt into account.  So the first position in a line after the prompt
# line corresponds to the first position AFTER THE PROMPT on the prompt
# line
proc termTextUpDownLine {w n} {
    global ::tk::Priv

    set i [$w index promptEnd]
    scan $i "%d.%d" pline pchar
    set i [$w index insert]
    scan $i "%d.%d" line char
    if {[string compare $::tk::Priv(prevPos) $i] != 0} {
        if {$pline == $line} {
            set ::tk::Priv(char) [expr $char - $pchar - 1]
        } else {
	    set ::tk::Priv(char) $char
        }
    }
    if {[expr $line + $n] == $pline} {
        set new [$w index [expr $line + $n].[expr $::tk::Priv(char) + $pchar + 1]]
    } else {
        set new [$w index [expr $line + $n].$::tk::Priv(char)]
    }
    if {[$w compare $new == end] || [$w compare $new == {insert linestart}] ||
        [$w compare $new <= promptEnd]} {
	set new $i
    }
    set ::tk::Priv(prevPos) $new
    return $new
}

# tkTextTranspose --
# This procedure implements the "transpose" function for text widgets.
# It tranposes the characters on either side of the insertion cursor,
# unless the cursor is at the end of the line.  In this case it
# transposes the two characters to the left of the cursor.  In either
# case, the cursor ends up to the right of the transposed characters.
#
# Arguments:
# w -		Text window in which to transpose.

#    if [$w compare "$pos - 1 char" == 1.0] {}
proc ::tk::TextTranspose w {
    set pos insert
    if [$w compare $pos == {promptEnd + 1 char}] {
	set pos [$w index "$pos + 1 char"]
    }
    if [$w compare $pos != "$pos lineend"] {
	set pos [$w index "$pos + 1 char"]
    }
    set new [$w get "$pos - 1 char"][$w get  "$pos - 2 char"]
    if [$w compare "$pos - 1 char" <= {promptEnd + 1 chars}] {
	return
    }
    $w delete "$pos - 2 char" $pos
    $w insert insert $new userText
    $w mark set insert {insert - 1 char}
    $w see insert
}


proc termTextMaybeSetCursor {w x y} {
    if [$w compare @$x,$y > promptEnd] {
        $w mark set insert @$x,$y
    }
}

proc termTextStepUpSelectMode {mode} {
    global ::tk::Priv
    if [info exists ::tk::Priv(selectMode)] {
        switch $::tk::Priv(selectMode) {
	    char {return word}
            word {return line}
            line {return char}
        }
    }
    return char
}

# termTextRewriteUserText (RMJ)
# If the last character in the widget is not tagged as userText, it means
# we have received output from somewhere else, so we need to rewrite the
# prompt and any user text that has been typed, to make things look clean
# again
proc termTextRewriteUserText {w} {
    # If we have done this right, there will only be one range in bounds
    set bounds [$w tag ranges userText]
    # If there's no user text yet, check where the end of the prompt is,
    # otherwise, use the bounds of the existing user text
    if {$bounds == {}} {
        set b1 [$w index {promptEnd + 1 char}]
        set b2 $b1
    } else {
        set b1 [lindex $bounds 0]
        set b2 [lindex $bounds 1]
    }
    # The last character tagged with "userText" is the last real character
    if [$w compare $b2 != {end - 1 chars}] {
        # store a copy of the user text
        set t [$w get $b1 $b2]
        # remember the relative position of the cursor
        if [$w compare userCursor == insert] {
            # If the userCursor was at the end of the userText, it will
            # be follwing the insert mark, so we need to set it back to
            # the end of the userText
            scan $b2 "%d.%d" userline userchar
        } else {
            set i [$w index userCursor]
            scan $i "%d.%d" userline userchar
        }
        scan $b1 "%d.%d" startline startchar
        set relline [expr $userline - $startline]
        set relchar [expr $userchar - $startchar]
        # print a new prompt
        termTextPrompt $w
        # insert a copy of the text
        $w mark set insert end
        termTextAddText $w $t
        # put the cursor in the right place
        $w mark set insert "promptEnd + $relline lines + $relchar chars + 1 chars"
        $w mark set userCursor insert
        # tag the new text
        $w tag add userText {promptEnd + 1 chars} {end - 1 chars}
    }
}

## Delete any of the selected text that is appears AFTER the prompt.
## This usually occurs when a key is pressed.  Return 1 if anything is
## deleted and 0 otherwise.  The returned information is used for the
## special cases of backspace and delete keys.
proc termTextDeleteSelectedText {w} {
    if {[$w tag nextrange sel {promptEnd + 1 chars} end] == {}} {
        return 0
    } else {
        $w mark set insert sel.last
        if [$w compare sel.first > promptEnd] {
            $w delete sel.first sel.last
        } else {
            $w delete {promptEnd + 1 chars} sel.last
        }
        return 1
    }
}

##############
## Invoke the term's process when a "\n" is typed at the keyboard
## Do this by attempting to process the command (which checks to see
## if it is a complete command), underlining the command text when it is
## complete, and the setting up a new prompt
proc termTextInvoke {w} {
    if [catch {set cmd [$w get userText.first userText.last]}] {
        set cmd "\n"
    }

    if { ![string compare [string trim $cmd] "!!"] } {
		  set cmd [history event [expr [history nextid] -1]]
		  termTextInsert $w "$cmd\n"
    }
    if [regexp {^!(.+)$} [string trim $cmd] dummy event] {
		  set cmd [history event $event]
		  termTextInsert $w "$cmd\n"
    }
    if [regexp {^\^([^^]*)\^([^^]*)\^?$} $cmd dummy old new] {
		  regsub -all $old [history event [expr [history nextid] -1]] \
					 [string trim $new] cmd
		  termTextInsert $w "$cmd\n"
    }

    if [termTextProcessCommand $w $cmd] {
        $w tag add oldUserText userText.first userText.last
        termTextPrompt $w
    }
}

##############
# You can also invoke the term's process by sending it a command vias
# a menu, button, send, or anything other than user interaction with the
# termText widget.  In order for things to work properly, you have to
# send these "interrupting" commands via the termTextSendCommand
# procedure. 
# This procedure remembers whatever was being typed on the regular
# prompt line, but "interrupts" it by printing out a new "fake"
# prompt, followed by the command (this is just so the user can have
# some idea of what's going on when one of these commands is invoked).
# It then sets up a new prompt with the user's prompt-line text, so
# typing can take up from wherever it left off when it was interrupted.

# Note: this procedure assumes that the command argument is a SINGLE,
# COMPLETE command (which can be a TCL script, however, that includes
# multiple commands)
proc termTextSendCommand {w command} {
    global termTextPromptValue
    $w mark set insert end
    # Put in a fake prompt if we are not already at a prompt
    if [$w compare insert > {promptEnd + 1 char}] {
       termTextAddText $w "\n$termTextPromptValue($w)"
       $w tag add prompt {insert linestart} {insert - 1 chars}
    }
    # print out the command
    $w mark set commandStart {insert - 1 chars}
    termTextAddText $w $command
    $w tag add oldUserText {commandStart + 1 chars} insert
    $w mark unset commandStart
    termTextAddText $w "\n"
    termTextProcessCommand $w $command
    termTextRewriteUserText $w
}

# The procedure below is used to print out a prompt at the insertion
# point.
# If the insertion point is not at the beginning
# of a line, a carriage return is added so the prompt will always
# appear at the left.  If we are logging, then we dump the prompt
# to the log file as well.  Finally, highlight the prompt by
# marking it as bold text and set the promptEnd mark so the cannot
# backspace over the prompt.  The font used by the bold tag is
# defined in the initialization section.

proc termTextPrompt {w} {
    global termTextPromptValue
    $w mark set insert end
    if [$w compare insert != {insert linestart}] {
        termTextAddText $w "\n"
    }
    termTextAddText $w $termTextPromptValue($w)
    $w mark set promptEnd insert-1c
    $w tag add prompt {promptEnd linestart} promptEnd
    $w tag remove userText 1.0 end
    $w mark set userCursor insert
    ## "see" the end to make sure we scroll along with text being inserted
    ## by the output of a command.
    ## $w see insert
    $w see end
}

proc termTextProcessCommand {w cmd} {
    # First thing we do is make sure things start getting inserted at
    # the end
    $w mark set insert end

    if [info complete $cmd] {

        set result [catch {uplevel #0 [list history add $cmd exec]} msg]
        uplevel #0 set currentHistory [history nextid] 

	if {$msg != {} } {
            termTextAddText $w "$msg\n"
        }
        return 1
    }
    # did not process command, because it was not complete
    return 0
}

# We use this procedure to add user text to the termtext widget, so
# individual applications can redefine it if they want (for example, to
# add logging to a file)
proc termTextAddText {w t {tags {}}} {
    if {$tags == {}} {
        $w insert insert $t
    } else {
        $w insert insert $t $tags
    }
}

### This acts like tk_textCut, but only cuts the portion of the selection
### that is after the prompt.
proc termTextCut w {
    if {[selection own -displayof $w] == "$w"} {
        clipboard clear -displayof $w
        catch {
            clipboard append -displayof $w [selection get -displayof $w]
            termTextRewriteUserText $w
            if {[termTextDeleteSelectedText $w] == 0} {
	        $w delete insert
	        $w see insert
            }
        }
    }
}

### This acts like tk_textPaste, but does an insertByLines
proc termTextPaste w {
   #puts "in termTextPaste"
    catch {
        termTextInsertByLines $w [selection get -displayof $w \
                -selection CLIPBOARD]
    }
}

