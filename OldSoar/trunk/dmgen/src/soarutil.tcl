###
# Copyright 1995-2004 Soar Technology, Inc., University of Michigan. All 
# rights reserved.
# 
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are 
# met:
# 
#    1.	Redistributions of source code must retain the above copyright 
#       notice, this list of conditions and the following disclaimer. 
# 
#    2.	Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in 
#       the documentation and/or other materials provided with the 
#       distribution. 
# 
# THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND 
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SOAR 
# CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
# DAMAGE.
# 
# The views and conclusions contained in the software and documentation 
# are those of the authors and should not be interpreted as representing 
# official policies, either expressed or implied, of Soar Technology, Inc., 
# the University of Michigan, or the Soar consortium.
### 

##
# Various utility functions for dealing with Soar.
#
# Requires:
#     Soar
#     production.tcl (Production::NodeTypes)

namespace eval SoarUtil {

##
# Strip the bars off a Soar string if they're both there.
#
# @param s The String to process
# @returns If s has bars, returns s with the bars removed. Otherwise,
#          returns s
proc StripSoarBars { s } {
   set n [string length $s]
   if { $n > 1 && \
        [string range $s 0 0 ] == "|" && \
        [string range $s end end ] == "|" } {
      return [string range $s 1 [expr $n - 2]]
   }
   return $s
}

##
# Run a soar command and return its output.
#
# @param cmd Command to run
# @returns Result of command
proc RunSoarCommand { cmd } {
   output-strings-destination -push -append-to-result
   catch {
   set buf [eval $cmd]
   }
   output-strings-destination -pop
   return $buf
}

##
# Get the result of "print -internal" for the specified production
#
# @param name Name of production
# @returns {} if an error occured, the results otherwise.
proc GetSoarPrintInternal { name } {
   set buf [RunSoarCommand "print -internal $name"]
   if { [string range $buf 0 1] != "sp" } {
      return {}
   }
   return $buf
}

##
# Get the result of "print" for the specified production
#
# @param name Name of production
# @returns {} if an error occured, the results otherwise.
proc GetSoarPrint { name } {
   set buf [RunSoarCommand "print $name"]
   if { [string range $buf 0 1] != "sp" } {
      return {}
   }
   return $buf
}

##
# Get a list of all loaded productions
proc GetSoarProductions { } {
   set L {}
   set b [split [RunSoarCommand "print -all"] " \n"]
   foreach p $b {
      if { [llength $p] > 0 } {
         lappend L [StripSoarBars $p]
      }
   }
   return $L
}

##
# Get the name of the file from which a production was loaded.
#
# @param name Name of production
# @returns Name of file.
proc GetSoarProductionFile { name } {
   return [RunSoarCommand "print -filename $name"]
}

##
# Try to guess the type of a single value (int, float, or string)
#
proc GuessSoarType { s } {
   set t {}
   set s [string trim $s]
   set int {^([+-])*[0-9]+$}
   set float1 {^([+-])*[0-9]*\.?[0-9]+$}  ;# normal float
   set float2 {^([+-])*[0-9]+\.$}         ;# no digits after '.'
   if [regexp $int $s] {
      set t int
   } elseif [regexp $float1 $s] {
      set t float
   } elseif [regexp $float2 $s] {
      set t float
   } else {
      set t string
   }
   return $t
}

##
# Try to guess a Soar type given a list of values...
proc GuessSoarTypeFromList { s } {
   set ls [llength $s]
   if { $ls == 0 } { ;# no value, no guess (could be identifier?)
      return unknown 
   }
   if { $ls == 1 } { ;#single value, one guess
      return [GuessSoarType $s]
   }

   # initialize bitmap indexed by typename
   foreach i [Production::NodeTypes] { set rlu($i) 0 }

   # accumulate individual guesses in a bitmap indexed by typename
   set lr 0 ;# count of number of different guesses encountered
   foreach v $s {
      set t [GuessSoarType $v]
      if { $rlu($t) == 0 } { incr lr }
      set rlu($t) 1
   }

   if { $lr == 1 } { ;# single type guessed. yay.
      if { $rlu(float) } {
         set r float
      } elseif { $rlu(int) } {
         set r int
      } else {
         #set r enum
         set r string
      }
   } elseif { $lr > 0 } { ;# multiple, conflicting guesses
      # possibilities are string int float...

      # If only float and int have been guessed, go with float.
      if { $lr == 2 && $rlu(float) && $rlu(int) } {
         set r float
      } else { ;# mix of strings, ints, and floats, go with enum
         #set r enum
         set r string
      }
   }
   return $r
}

} ;# end namespace SoarUtil
