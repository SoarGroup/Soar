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

    # This loop will escape any existing quotes in the string
    if { $name == "sp" } {
      set i [string first "\"" $arg]
      while {$i >= 0 } {
        set arg [string replace $arg $i $i "\\\""]
        set i [string first "\"" $arg [expr $i + 2]]
      }
    }

    # Now wrap the argument with quotes (if it has a space in it)
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

####
# sp
# @param args - the production being sourced
# We define a custom procedure to handle sp commands
# If a production is replaced (duplicate name), it prints out the rule's name
# This is partly because the command 'source filename -v' doesn't work
# (Tcl source can't accept command line arguments)
#######

proc sp { args } {
    global output_buffer
    set argstr [buildExecuteCommandArgumentString "sp" $args]
    set r [executeCommandLine $argstr]
    if { $r == "#*" } {
        set rule [lindex $argstr 1]
        set rulename  [string range $rule 0 [string first " " $rule]]
        appendOutputBuffer "\nREPLACING RULE $rulename\n"
    } elseif {$r != "*"} {
        appendOutputBuffer "\n$r\n"
    } else {
        appendOutputBuffer $r
    }
    return ""
}


##
# Create TCL procs for all soar commands
# Some commands like source, alias and unalias have special implementations
# below. Dir commands also have tcl implementations: cd dirs popd pushd pwd
defineSoarCommands [set allSoarCommands {
  alias
  chunk
  debug
  decide
  echo
  epmem
  explain
  gp
  help
  load
  output
  preferences
  print
  production
  rl
  run
  save
  smem
  soar
  #sp # handled by custom procedure above
  srand
  stats
  svs
  trace
  visualize
  wm
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
    puts "ERROR sourcing File $file"
    puts $errorMessage
    popd
    error $errorMessage
  }
  
  popd
  return ""
}

proc initializeSlave {} {
  reconfigureOutput
}
