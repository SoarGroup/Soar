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
# Generic utility functions

##
# Create a globally accessible constant, e.g.
#  Constant Pi 3.1415926535
#  puts "Pi = [pi]"
#
#  @param name Name of constant
#  @param value Value of constant
proc Constant { name value } {
   proc $name { } [list return $value]
}

##
# Create a proc static variable
#  @param List of names of static variables to create
proc Static { args } {
   set procName [lindex [info level -1] 0]
   foreach varName $args {
      uplevel 1 "upvar #0 staticvars($procName:$varName) $varName"
   }
}

##
# Simple assertion. remove with 'proc Assert {x} { }'
proc Assert {condition} {
   if {[catch {uplevel [list expr $condition]} n] || $n == "" || $n == 0} {
      puts "Assertion failed (result $n), in:"
      set prefix ""
      for {set i [info level]} {$i} {incr i -1} {
         append prefix "  "
            puts "$prefix'[info level $i]'"
      }
      # try to call a failure handler to collect more info
      if {![catch ::AssertionFailureHandler msg] && $msg != ""} {
         append condition " ($msg)"
      }
      #error "Assertion failed: $condition"
      puts "Assertion failed: $condition"
      exit
   }
} 

##
# Add code that is run if the source file is run standalone.
#
# @param body Code to execute when script is run standalone.
proc IfStandAlone body {
   global argv0
   # If the script file is the same as "this" file, run body.
   if { [info exists argv0] && \
         ![string compare [file tail [info script]] [file tail $argv0]] } {
      catch {console show}
      uplevel $body
   }
 }

proc Max {a args} {foreach i $args {if {$i>$a} {set a $i}};return $a}
proc Min {a args} {foreach i $args {if {$i<$a} {set a $i}};return $a}


##
# Generate a unique "tag" for a particular namespace. Subsequent calls with
# the same namespace will return new unique tags.
#
# @param nspace Name of tag
# @param int  If true, then the tag is simply an integer, otherwise it is a 
#             string.
proc GetTag { nspace { int 0 } } {
   set idx __nextNum$nspace ;# A global "static" variable with the next tag
   global $idx
   if ![info exists $idx] { ;# initialize on first call
      set $idx 0
   }
   incr $idx 
   if { $int } {
      return [set $idx]
   }
   return "__tag$nspace[set $idx]"
}

##
# Join a list of path elements with the system path separator.
# 
# @param pathList List of path elements
proc JoinPathList { pathList } {
   set r ""
   foreach p $pathList {
      set r [file join $r $p]
   }
   return $r
}

##
# Get an absolute path to the currently running script.
#
# This function is only accurate if it's called from a top-level
# script that hasn't changed the current directory.
#
# @returns a tuple (path to script, name of script)
proc GetPathToScript { } {
   set relPath [info script]
   set scriptName [file tail $relPath]
   set dirName [file dirname $relPath]

   set pathType [file pathtype $dirName]
   if { $pathType == "absolute" } {
      return [list $dirName $scriptName]
   } elseif { $pathType == "relative" } {
      set cur [file split [pwd]]
      set parts [file split $dirName]
      set end [llength $cur]
      for { set i 0 } { $i < [llength $parts] } { incr i } {
         set p [lindex $parts $i]
         if { $p == ".." } {
            incr end -1
         } elseif { $p != "." } {
            break
         }
      }
      set full [concat [lrange $cur 0 [expr $end - 1]] \
                       [lrange $parts $i end]]
      return [list [JoinPathList $full] $scriptName]
   } else { ;# volumerelative
      return [list $dirName $scriptName]
   }
}

##
# Returns true if a list is empty
proc lempty { list } {
   return [expr [llength $list] == 0]
}

##
#  Returns a new list that is 'list' with the first instance of 'value'
#  removed.
#
#  @param list Input list
#  @param value Value of item to remove from list
#  @retursn New list
proc ldelete { list value } {
   set ix [lsearch -exact $list $value]
   if { $ix >= 0 } {
      return [lreplace $list $ix $ix]
   } else {
      return $list
   }
}

##
# Remove duplicates from a list and return a new list
#
# @param list List to process
# @returns Copy of list with duplicates removed
proc lunique {list} {
   # ulist is the uniq-ed list
   set ulist {}

   # The main loop
   foreach e $list {
      if {![info exist vec($e)]} {
         set vec($e) {}
         lappend ulist $e
      }
   }

   return [lrange $ulist 0 end]
}

proc lcompare {a b} {
   set la [llength $a]
   set lb [llength $b]

   if { $la != $lb } { return 0 }
   for { set i 0 } { $i < $la } { incr i } {
      if { [lindex $a $i] != [lindex $b $i] } {
         return 0
      }
   }
   return 1
}
proc ltail { L i } {
   set l [llength $L]
   if { $l < $i } {
      return $L
   }
   return [lrange $L [expr $l - $i] end]
}

##
# Create the union of two lists, i.e. all elements from both with no duplicates
#
# @param a First list
# @param b Second list
# @returns A new list that is the a U b.
proc lunion { a b } {
   return [lunique [concat $a $b]]
}
##
# Calculate set intersection of lists a and b and return it.
proc lintersect { a b } {
   set i {}
   foreach v $a {
      if { [lsearch -exact $b $v] >= 0 } {
         lappend i $v
      }
   }
   return $i
}
##
# Calculate the set difference a - b of lists a and b and return it
proc ldiff { a b } {
   set d $a
   foreach v $b {
      set d [ldelete $d $v]
   }
   return $d
}

##
# Repeatedly print a string to the console.
#
# @param lvl Number of repititions of string
# @param s String to repeat
proc PrintTabs { lvl { s  "  " } } {
   for { set i 0 } { $i < $lvl } { incr i } {
      puts -nonewline $s
   }
}

proc UnwindDirStack {} {
   while { ![catch { popd }] } { }
}

proc ReplaceChar { s ic oc } {
   set l [string length $s]
   set r ""
   for { set i 0 } { $i != $l } { incr i } {
      set c [string index $s $i]
      if { [string compare $c $ic] == 0 } {
         set c $oc
      }
      set r [concat $r $c]
   }
   return [join $r ""]
}

