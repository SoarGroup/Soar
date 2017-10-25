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

proc hasProcedure { procName } {
	if { [info commands $procName] == "" } {
		return "false"
	} else {
		return "true"
	}
}

##
# Defines tcl procedures for soar commands.
#
# This file is source within an agent's interpreter.
#

# Define proc for executing Soar commands through SML
# Will throw an error if the ExecuteCommandLine command failed
proc executeCommandLine { command } {
  global _agent _kernel

  set result [$_agent ExecuteCommandLine $command false true]
  set ok [$_agent GetLastCommandLineResult]
  if { $ok == 0 } {
	  error $result
  }

  return $result
  #return [$_agent ExecuteCommandLine $command false true]
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
    global output_buffer last_file verbose

    set argstr [buildExecuteCommandArgumentString "sp" $args]
	set result [executeCommandLine $argstr]
	if { $verbose == "true" && $result == "#*" } {
        set rule [lindex $argstr 1]
        set rulename  [string range $rule 0 [string first "\n" $rule]]
        appendOutputBuffer "\nReplacing production $rulename"
	} else {
        appendOutputBuffer $result
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
proc source {fname args} {
  global _agentName last_file verbose
  set dir [file dir $fname]
  set file [file tail $fname]
  set last_file $file

  if { $args != "" && [string first "-v" $args] != -1 } {
	  set verbose "true"
  }
  
  pushd $dir

  # Source the file in the global scope and catch any errors so
  # we can properly clean up the directory stack with popd
  if { [catch {uplevel #0 builtInSource $file} errorMessage] } {
    popd
	if { [string first "\nError in file" $errorMessage] == 0} {
		error "$errorMessage"
	} else {
		error "\nError in file [pwd]/$file: \n$errorMessage"
	}
  }
  
  popd
  return ""
}

proc initializeSlave {} {
  global verbose
  set verbose "false"

  reconfigureOutput
}

# The following is a workaround to deal with a limitation of the Tcl
# interface.  Alias issues from the command line go through the normal
# Soar CLI alias mechanism, but aliases used in sourced files do not seem
# to be, which breaks backwards compatibility.  The following code creates 
# Tcl-based aliases that parallel the real ones defined in the cli code.  

global defined_aliases
set defined_aliases {}

proc myalias {{name ""} args} {
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

myalias a alias
myalias aw wm add
myalias chdir cd
myalias ctf output command-to-file
myalias d run -d
myalias dir ls
myalias e run -e
myalias ex production excise
myalias fc production firing-counts
myalias gds_print print --gds
myalias inds decide indifferent-selection
myalias init soar init
myalias interrupt soar stop
myalias is soar init
myalias man help
myalias p print
myalias pc print --chunks
myalias ps print --stack
myalias pw production watch
myalias quit exit
myalias r run
myalias rn load rete-network
myalias rw wm remove
myalias s run 1
myalias set-default-depth output print-depth
myalias ss soar stop
myalias st stats
myalias step run -d
myalias stop soar stop
#myalias topd pwd
myalias un alias -r
myalias varprint print -v -d 100
myalias w trace
myalias wmes print -depth 0 -internal

myalias unalias alias -r
myalias indifferent-selection decide indifferent-selection
myalias numeric-indifferent-mode decide numeric-indifferent-mode
myalias predict decide predict
myalias select decide select
#myalias srand decide srand

myalias replay-input load percepts
myalias rete-net load rete-network
myalias load-library load library
# myalias source load file
myalias capture-input save percepts

myalias pbreak production break
myalias excise production excise
myalias production-find production find
myalias firing-counts production firing-counts
myalias matches production matches
myalias memories production memory-usage
myalias multi-attributes production optimize-attribute
myalias pwatch production watch

myalias add-wme wm add
myalias wma wm activation
myalias remove-wme wm remove
myalias watch-wmes wm watch

myalias allocate debug allocate
myalias internal-symbols debug internal-symbols
myalias port debug port
#myalias time debug time

myalias init-soar soar init
myalias stop-soar soar stop
myalias gp-max soar max-gp
myalias max-dc-time soar max-dc-time
myalias max-elaborations soar max-elaborations
myalias max-goal-depth soar max-goal-depth
myalias max-memory-usage soar max-memory-usage
myalias max-nil-output-cycles soar max-nil-output-cycles
myalias set-stop-phase soar stop-phase
myalias soarnews soar
myalias cli soar tcl
myalias tcl soar tcl
myalias timers soar timers
myalias version soar version
myalias waitsnc soar wait-snc

myalias chunk-name-format chunk naming-style
myalias max-chunks chunk max-chunks

myalias clog output log
myalias command-to-file output command-to-file
myalias default-wme-depth output print-depth
myalias echo-commands output echo-commands
myalias verbose trace -A
myalias warnings output warnings

myalias watch trace
