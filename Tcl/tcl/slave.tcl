global output_buffer
set output_buffer ""

proc getOutputBuffer {} {
  global output_buffer
  return $output_buffer
}

proc clearOutputBuffer {} {
  global output_buffer
  set output_buffer ""
}
proc appendOutputBuffer {newOutput} {
  global output_buffer
  append output_buffer $newOutput
}

##
# Defines tcl procedures for soar commands.
#
# This file is source within an agent's interpreter.
#

# Define proc for executing Soar commands through SML
proc executeCommandLine { command } {
  global _agent
  return [$_agent ExecuteCommandLine $command false true]
}

##
# Takes a Soar command name and argument list (from Tcl) and builds a
# single string for the command making sure to use quotes around arguments
# with spaces. If we don't do this, then the arguments end up with braces
# around them, which screws up SML commands that take file names

proc buildExecuteCommandArgumentString { name argList } {
  set result $name
  foreach arg $argList {
    append result " "
    if { [string first " " $arg] >= 0 } {
      append result "\"$arg\""
    } else {
      append result $arg
    }
  }
  return $result
}

##
# Given a list of command names, create Tcl procs for them that forward the
# commands to SML
#
# @param commands List of Soar command names
proc defineSoarCommands { commands } {
  global printCommandResults

  foreach name $commands {
    proc $name {args} "
         global output_buffer
         set r \[executeCommandLine \[buildExecuteCommandArgumentString $name \$args\]\]
         appendOutputBuffer \$r
         return \"\"
      "
  }
}

##
# Create TCL procs for all soar commands
# Some commands like source, alias and unalias have special implementations
# below. Dir commands also have tcl implementations: cd dirs popd pushd pwd

defineSoarCommands [set allSoarCommands {
   add-wme
   allocate
   capture-input
   chunk-name-format
   cli
   clog
   command-to-file
   debug
   default-wme-depth
   echo
   echo-commands
   edit-production
   epmem
   explain-backtraces
   excise
   firing-counts
   gds-print
   gp
   gp-max
   help
   indifferent-selection
   init-soar
   internal-symbols
   learn
   load-library
   ls
   matches
   max-chunks
   max-dc-time
   max-elaborations
   max-goal-depth
   max-memory-usage
   max-nil-output-cycles
   memories
   multi-attributes
   numeric-indifferent-mode
   o-support-mode
   port
   predict
   preferences
   print
   production-find
   pwatch
   rand
   remove-wme
   replay-input
   rete-net
   rl
   run
   save-backtraces
   select
   set-library-location
   set-stop-phase
   smem
   soar
   soarnews
   sp
   srand
   stats
   stop-soar
   time
   timers
   verbose
   version
   waitsnc
   warnings
   watch
   watch-wmes
   wma
}]

global outputStringsStack currentOutputStringsProc

# Current stack of pushed destinations
set outputStringsStack {}

# The current procedure to call for output
set currentOutputStringsProc ""

# The -discard output procedure
proc discardOutputStringsProc {message} {}

##
# Implementation of output-strings-destination command
proc output-strings-destination {args} {
  global outputStringsStack currentOutputStringsProc
  global _agentName

  set n [llength $args]
  if { $n == 0 } {
    error "Not enough arguments to output-strings-destination"
  }

  set arg0 [lindex $args 0]
  if { $arg0 == "-push" } {
    set arg1 [lindex $args 1]
    if { $arg1 == "-procedure" } {
      set newProc [lindex $args 2]
    } elseif {$arg1 == "-channel"} {
      set newProc [list puts [lindex $args 2]]
    } elseif {$arg1 == "-append-to-result"} {
      set newProc discardOutputStringsProc
    } elseif {$arg1 == "-discard"} {
      set newProc discardOutputStringsProc
    } else {
      error "Unsupported option $arg1"
    }
    lappend outputStringsStack $currentOutputStringsProc
    set currentOutputStringsProc $newProc
  } elseif { $arg0 == "-pop" } {
    set stackSize [llength outputStringsStack]
    if {$stackSize != 0} {
      set currentOutputStringsProc [lindex $outputStringsStack end]
      set outputStringsStack [lrange $outputStringsStack 0 end-1]
    } else {
      error "output-strings-destination stack is empty"
    }

  } else {
    error "Unrecognized argument '$arg0'"
  }

  #puts "stack is $outputStringsStack"
  #puts "proc is $outputStringsStack"
}

proc printCallback { message } {
  global currentOutputStringsProc

  # Since currentOutputStringsProc may be a list, we have to append
  # message to the end of it. We can't use concat because, if message
  # has spaces, then it will be treated as a list rather than a single
  # argument. Stupid tcl.
  set command $currentOutputStringsProc
  lappend command $message
  global _agentName
  eval $command
}


# We can use this technique to intercept "puts" and send the output somewhere else
# of our choosing.  In this case we'll return the output string as the result of the
# call which means it'll be passed back as the result of the "puts" command and be
# printed in the debugger (if issued from there).
proc reconfigureOutput {} {
  if {[info proc puts] != "puts"} {
    rename puts builtInPuts

    proc puts {args} {
      # We need to see if there are any parameters to puts and handle
      # according.  We'll ignore any channel arguments.  Following logic
      # may seem weird, but the parameter rules are a bit particular:
      # - Must have at least one argument
      # - 1 argument
      #   - Param 1:  string to output
      # - 2 arguments
      #   - Case 1:
      #     - Param 1: -no-newline
      #     - Param 2: string to output
      #   - Case 2:
      #     - Param 1: channel
      #     - Param 2: string to print
      # - 3 arguments
      #   - Param 1: -no-newline
      #   - Param 2: channel
      #   - Param 3: string to output

      set n [llength $args]
      set addNewLine 1
      set strIndex 0
      if { $n > 1 } {
        if { [lindex $args 0] == "-nonewline" } {
          set addNewLine 0
          if {$n == 3} {
            # puts -nonewline channel string
            set strIndex 2
          } else {
            # puts -nonewline string
            set strIndex 1
          }
        } else {
          # puts channel string
          set strIndex 1
        }
      }

      set putOutput [join [lindex $args $strIndex]]
      if ($addNewLine) {
        append putOutput "\n"
      }
      appendOutputBuffer $putOutput
      return ""
    }
  }
}

##
# This proc is called to source a Soar file when creating a new agent. Its purpose
# is to perform the proper directory change when sourcing the file which is not
# done by sml::Agent::LoadProductions.
rename source builtInSource
proc source {arg} {
  set dir [file dir $arg]
  set file [file tail $arg]
  global _agentName

  pushd $dir

  # Source the file in the global scope and catch any errors so
  # we can properly clean up the directory stack with popd
  if { [catch {uplevel #0 builtInSource $file} errorMessage] } {
    popd
    error $errorMessage
  }

  popd
  return ""
}

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
set print_alias_switch "on"

proc alias {{name ""} args} {
  global defined_aliases print_alias_switch

  if {[string compare $name ""] == 0} {
    puts [lsort [set defined_aliases]]
  } elseif {[string compare $args ""] == 0} {
    set position [lsearch -exact $defined_aliases $name]
    if {$position >= 0} {
      puts "$name: [info body $name]"
    } else {
      puts "Error: There is no alias named \"$name\". "
    }
  } elseif {[string compare $args "-off"] == 0} {a
    set position [lsearch -exact $defined_aliases $name]
    if {$position >= 0} {
      set defined_aliases [lreplace $defined_aliases $position $position]
      rename $name {}
      if {[string compare $print_alias_switch "on"] == 0} {
        puts "Removing alias \"$name\"."
      }
    } else {
      puts "Error: There is no alias named \"$name\"."
    }
  } else {
    if {[lsearch -exact [info commands] $name] >= 0 | \
      [lsearch -exact $defined_aliases $name] >= 0 } {
      puts "Alias Error: \"$name\" already exists as a previously defined alias or command.\nNew alias not created.\n"
    } else {
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
  global defined_aliases print_alias_switch
  set position [lsearch -exact $defined_aliases $name]
  if {$position >= 0} {
    set defined_aliases [lreplace $defined_aliases $position $position]
    rename $name {}
    if {[string compare $print_alias_switch "on"] == 0} {
      puts "Removing alias \"$name\"."
    }
  } else {
    puts "Error: There is no alias named \"$name\"."
  }
}

proc initializeSlave {} {
  reconfigureOutput
}
