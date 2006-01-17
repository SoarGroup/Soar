
#puts "loading soar.tcl for version 8.2"

#
# The default productions
#

set default [file join $soar_library default.soar]


watch 0
watch 1

#
# When an alias is defined, a new procedure is created at the
# top level whose name matches the alias.  The string defining
# the alias is used as the prefix of the actual command issued.
# If the alias command is invoked with additional arguments
# these are post-pended onto the command prefix.  Note the
# argument "args" is treated like Common Lisp's &rest argument.
#

global defined_aliases
set defined_aliases {}

global print_alias_switch
set print_alias_switch "off"

proc alias {{name ""} args} {
  global defined_aliases print_alias_switch

  if {[string compare $name ""] == 0} {
      lsort [set defined_aliases]
  } elseif {[string compare $args ""] == 0} {
      set position [lsearch -exact $defined_aliases $name]
      if {$position >= 0} {
	  echo "$name: [info body $name]"
      } else {
	  echo "Error: There is no alias named \"$name\". "
      }
  } elseif {[string compare $args "-off"] == 0} {
      set position [lsearch -exact $defined_aliases $name]
      if {$position >= 0} {
	  set defined_aliases [lreplace $defined_aliases $position $position]
	  rename $name {}
	  if {[string compare $print_alias_switch "on"] == 0} {
	      echo "Removing alias \"$name\"."
	  }
      } else {
	  echo Error: There is no alias named \"$name\".
      }
  } else {
      if {[lsearch -exact [info commands] $name] >= 0 | \
	      [lsearch -exact $defined_aliases $name] >= 0 } {
	  echo "Error: \"$name\" already exists as a previously defined alias or command.\nNew alias not created."
      } else {
	  if {[string compare $print_alias_switch "on"] == 0} {
	      echo "Defining alias \"$name\" as \"$args\""
	  }
	  set defined_aliases [linsert $defined_aliases 0 $name]

	  uplevel #0 "proc $name {args} {
	      if {\$args == \"\"} {
		  $args 
	      } else {
		  eval $args \$args
	      }
	  }"
      }
  }
}
	  
proc unalias {name} {
    global defined_aliases
    set position [lsearch -exact $defined_aliases $name]
    if {$position >= 0} {
	set defined_aliases [lreplace $defined_aliases $position $position]
	rename $name {}
	if {[string compare $print_alias_switch "on"] == 0} {
	    echo "Removing alias \"$name\"."
	}
    } else {
	echo Error: There is no alias named \"$name\".
    }
}

#---------------------------------------------------------------

# These alias definitions arise from Soar 6 command syntax
#
global documented_aliases
set documented_aliases "? wmes"

alias ?     help
alias wmes  print -depth 0 -internal
#alias quit  exit
    
# More complex aliases can be defined using proc.  Note that
# the argument syntax {<arg> <initial value if unspecified>}
# is treated like Common Lisp's &optional argument.

proc d {{num 1}} {
	run d $num 
}

proc e {{num 1}} {
	run e $num 
}

#  the TSI has a "step" button, so we should also allow users to type it in
alias step run 1 d


# this was originally local for umich, since we have an annoying
# conflict.  But other sites have reported it also, so we
# will include it here since "go" was used in recent versions.
alias go    echo go is not a Soar7 command. Use run instead.


### Description : Extended aliases for Soar7.  Derived from commonly used and 
### psychologically supported rules, computed to save users time and errors.
### Based on the keystroke model and designed to be learned by a command
### name rule. This absorbed much of the standard set above and expanded upon
### it. Aliases are added and the Tcl variable documented_aliases is updated.

source [file join $soar_library soar7.2km-aliases.tcl]



#---------------------------------------------------------------
#
# Help procedures

global pager_cache
set pager_cache ""

global help_no_args_string
set help_no_args_string "
Commonly used Soar commands:
  cd		Change to another directory
  excise	Remove productions from Soar's memory
  init-soar	Reinitialize Soar
  learn		Turn learning on and off
  log		Save a Soar session to a file
  matches	Print info about the match set and partial matches
  preferences	Display items in preference memory
  print		Display productions or working memory elements
  pwd		Display the current working directory
  quit		Exit Soar
  run		Run the Soar decision cycle
  soarnews	Display information such as where to report bugs
  source	Load a file into Soar
  sp		Define a Soar production
  version	Display the version number of Soar
  watch		Set the amount of information displayed as Soar runs
  wmes		alias to display working memory elements

For a list of ALL available help topics, type \"help -all\"
For help on a specific command, type \"help\" followed by the command name.
"

proc help args {
    global soar_library
    global pager_cache env help_no_args_string 
    global documented_aliases tcl_man_dir tk_man_dir

    if {([llength $args] == 0) || ([lindex $args 0] == "-beginner")} {
	return $help_no_args_string
    }

    if [info exists tcl_man_dir] {
	set tcl_doc [file join $tcl_man_dir mann]
    } else {
	set tcl_doc ""
    }
    if [info exists tk_man_dir] {
	set tk_doc [file join $tk_man_dir mann]
    } else {
	set tk_doc ""
    }
    if {[lindex $args 0] == "-all"} {
	set d [pwd]
	cd [file join $soar_library ../doc/man]
	set topics_string "\nSoar commands:\n\n"
	set topics [glob -nocomplain -- *.n]
        set names ""
        foreach f $topics {
            set names [concat $names [file rootname $f]]
        }
	set names [lsort $names]
	set linelength 0
	foreach item $names {
	    set topics_string "$topics_string $item"
	    incr linelength [string length $item]
	    if {$linelength > 59} {
		#puts "item = $item, length = $linelength"
		set topics_string [format "$topics_string\n"]
		set linelength 0
	    }
	}
	
	if [file isdirectory $tcl_doc] {
	    #puts "in help: tcl_doc = $tcl_doc\n"
	    cd $tcl_doc
	    #puts "\t in dir: [pwd]\n"
	    set topics_string "$topics_string \n\nTcl built-in commands:\n\n"
	    set topics [glob -nocomplain -- *.n]
	    set names ""
	    foreach f $topics {
		set names [concat $names [file rootname $f]]
	    }
	    set names [lsort $names]
	    set linelength 0
	    foreach item $names {
		set topics_string "$topics_string $item"
		incr linelength [string length $item]
		if {$linelength > 58} {
		    #puts "item = $item, length = $linelength"
		    set topics_string [format "$topics_string\n"]
		    set linelength 0
		}
	    }
	} else {
	    set topics_string [format "$topics_string\n\nFor help on Tcl/Tk commands:\n  on Windows, open the tcl80.hlp file, usually in \n              \"C:/Program Files/Tcl/doc\" \n  on Unix, try \"man tcl\" or ask your system mgr\n              where the Tcl Man pages are. "]
	}
	
	if [file isdirectory $tk_doc] {
	    cd $tk_doc
            set topics_string "$topics_string \n\nTk commands:\n\n"
	    set topics [glob -nocomplain -- *.n]
            set names ""
            foreach f $topics {
                set names [concat $names [file rootname $f]]
	    }
	    set names [lsort $names]
	    set linelength 0
	    foreach item $names {
		set topics_string "$topics_string $item"
		incr linelength [string length $item]
		if {$linelength > 60} {
		    #puts "item = $item, length = $linelength"
		    set topics_string [format "$topics_string\n"]
		    set linelength 0
		}
	    }
	} 
	
	set topics_string [format "$topics_string\n"]
	cd $d
	return $topics_string
    }
    
    set unix_man_page 0

    # the next 3 lines allow -usage to appear anywhere on line...
    if {[lsearch $args "-usage"] > -1} {
	set index [lsearch $args "-usage"]
	set args  [lreplace $args $index $index]
	if {[llength $args] > 0} {
	    set topic [lindex $args 0]
            ## Soar-Bugs 127 - Test for unambiguous partial command
            if {([llength [info commands ${topic} ]] != 1) &&
                ([llength [info commands ${topic}*]] == 1)   } {
              set topic [info commands ${topic}*]
            }
	    if {[file exists [file join $soar_library ../doc/man/$topic.n]]} {
		set dir [file join $soar_library ../doc/man]
	    } elseif {[file exists [file join $tcl_doc $topic.n]]} {
		set dir $tcl_doc
	    } elseif {[file exists [file join $tk_doc  $topic.n]]} {
		set dir $tk_doc
	    } else {

		# See if its an alias
		
                if {[lsearch $documented_aliases $topic] >= 0} {
		    return "\"$topic\" is a documented alias, not a builtin command.\n Try \"help predefined-aliases\" "
		} else {

		    # One last try.  See if its in a man page.

                    echo "\"$topic\" is not a Soar, Tk, or Tcl command."
                    echo "Checking to see if it is a Unix command..."
                    set text [exec man $topic]
                    if [regexp "SYNOPSIS" $text] {
		        set unix_man_page 1
		    } else {
			return "No help is available for the topic \"$topic\".  \"help -all\" will list\nall available help topics and \"man\" gives help on Unix commands."
		    }
		}
	    }
	    set d [pwd]
	    if {!$unix_man_page} {
		cd $dir
		set f [open "| nroff -man $topic.n" r]
	    } else {
		set f [open "| man $topic" r]
	    }
	    while {![eof $f]} {
		gets $f line
		if [regexp {^[ ]*(S.)*S(Y.)*Y(N.)*N(O.)*O(P.)*P(S.)*S(I.)*I(S.)*S} \
			"$line"] {
		    gets $f line
		    while {   ![regexp {___} "$line"] 
                           && ![string match "" "$line"]
		           && ![eof $f]} {
			echo $line
			gets $f line
		    }
		    break
		}
	    }
	    catch "close $f"
	    cd $d
	    return
	} else {
	    return "help -usage: No topic given."
	}
    }

    set topic [lindex $args 0]

    ## Soar-Bugs 127 - Test for unambiguous partial command
    if {([llength [info commands ${topic} ]] != 1) &&
        ([llength [info commands ${topic}*]] == 1)   } {
      set topic [info commands ${topic}*]
    }

    if {[lsearch $documented_aliases $topic] >= 0} {
        set dir [file join $soar_library ../doc/man]
        set topic predefined-aliases
    } elseif {[file exists [file join $soar_library ../doc/man/$topic.n]]} {
	set dir [file join $soar_library ../doc/man]
    } elseif {[file exists [file join $tcl_doc $topic.n]]} {
	set dir $tcl_doc
    } elseif {[file exists [file join $tk_doc  $topic.n]]} {
	set dir $tk_doc
    } else {

# One last try.  See if its in a man page.

        echo "\"$topic\" is not a Soar, Tk, or Tcl command."
        echo "Checking to see if it is a Unix command..."
        set text [exec man $topic]
        if [regexp "S.*Y.*N.*O" $text] {
	    set unix_man_page 1
	} else {
	    return "No help is available for the topic \"$topic\".  \"help -all\" will list\nall available help topics and \"man\" gives help on Unix commands."
	}
    }

    if {!$unix_man_page} {
	set d [pwd]
	cd $dir
    }

#   Find pager to use    

    set pager $pager_cache
    if {$pager == ""} {
	catch {set pager $env(PAGER)}
	if {$pager == ""} {
	    catch {exec more} msg
	    if [regexp {couldn't find} $msg] {
		catch {exec page} msg
		if [regexp {couldn't find} $msg] {
		    catch {exec less} msg
		    if [regexp {couldn't find} $msg] {
		    } else {
			set pager less
		    }			
		} else {
		    set pager page
		}
	    } else {
		set pager more
	    }
	}
    }

    if {$pager != ""} {
	set pager_cache $pager
	set tempfile /tmp/soartk.out[pid]
	if {$unix_man_page} {
	    exec man $topic >$tempfile
	} else {
	    exec nroff -man $topic.n >$tempfile
	}
	uplevel #0 $pager $tempfile
    } else {
	echo "Cannot find pager to show help.  Please set the env. var. PAGER."
    }

    if {!$unix_man_page} {
	cd $d
    }

    return
}

#---------------------------------------------------------------
#
# Include pushd, popd, and dirs commands from the TclX extension.

source [file join $soar_library pushd.tcl]

#---------------------------------------------------------------

# Redefine the default prompt:
#
#global tcl_prompt1
#
#if [string match $interp_type agent] {
#    if [string match $interp_name soar] {
#	set tcl_prompt1 "puts -nonewline stdout {soar> }; log -add {soar> }"
#    } else {
#	set tcl_prompt1 "puts -nonewline stdout \"$interp_type $interp_name> \"; log -add \"$interp_type $interp_name> \""
#    }
#} else {
#    set tcl_prompt1 "puts -nonewline stdout \"$interp_type $interp_name> \""
#}


#---------------------------------------------------------------

# Helpful procedures.

# Dump the printed output and results of a command to a file

proc command-to-file {command arg1 {arg2 ""}} {
	if {[string match $arg2 "" ]} {
	    set file $arg1
	    set mode w
	} elseif {[string match $arg1 "-new"]} {
	    set file $arg2
	    set mode w
	} elseif {[string match $arg2 "-new"]} {
	    set file $arg1
	    set mode w
	} elseif {[string match $arg1 "-existing"]} {
	    set file $arg2
	    set mode a
	} elseif {[string match $arg2 "-existing"]} {
	    set file $arg1
	    set mode a
	} else {
	    error "command-to-file: Unrecognized arguments: $arg1 $arg2"
	}
	set f [open $file $mode]
	output-strings-destination -push -channel $f
	uplevel #0 [echo [eval $command]]
	output-strings-destination -pop
	close $f
}


# Send output to the "stdout" channel by default
output-strings-destination -push -channel stdout
