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
# BWidget utilities
#
# A set of utilities for dealing with widgets, mostly BWidgets.
package require BWidget

namespace eval BwUtil {

# Expand all the nodes in a BWidget tree widget.
proc ExpandBWidgetTree { t } {
   set ch [$t nodes root 0 end]
   foreach n $ch {
      $t opentree $n
   }
}

# Given a graph and a start vertex, fills in a BWidget tree with
# using the value of $field as the label of each node.
# The tag of each tree node is the same as its corresponding vertex in the
# graph.
# The -data property of each tree node is filled with a 'struct' with the
# following values:
#     BackEdges - List of adjacencies to this vertex that could not
#        be connected because they would have formed a cycle (not easy
#        to display with a tree :)
# Here's an example of getting at this data:
#     array set data [$tree itemcget $node -data]
#     puts "Backedges = $data(Backedges)"
#
# @param treeName Tree widget
# @param graph The source graph
# @param vert The starting vertex in the graph (only child of root)
# @param field The name of the vertex field that should be used as node
#              label. (e.g, name)
proc NewBWidgetTreeFromGraph { treeName graph vert field } {
   set w [Tree $treeName]
   foreach v [$graph GetVertices] {
      set visited($v) 0
   }
   set visited($vert) 1

   set NV [Graph::NullVertex]
   set L [list [list $vert $NV]]

   while { [llength $L] > 0 } {
      set vl [lindex $L 0]        ;# Get head of queue
      set L [lrange $L 1 end]    ;# pop head of queue
      
      set c [lindex $vl 0] ;#child
      set p [lindex $vl 1] ;#parent
      if { $p == $NV } { set p root }
      
      $w insert end $p $c -text [$graph Get $c $field]
      set be {} ;# accumulate back edges
      foreach v [$graph GetOutAdjacencies $c] {
         if { !$visited($v) } {
            set visited($v) 1
            lappend L [list $v $c]
         } else {
            puts "backedge!"
            lappend be $v
         }
      }
      $w itemconfigure $c -data [list BackEdges $be]
   }
   return $w
}

# Returns the selected text of a combo box.
proc GetComboBoxText { cb } {
   return [lindex [$cb cget -values] [$cb getvalue]]
}

# Sets a combo box selection from a text value (rather than an index)
#
# @param cb Widget path
# @param v  Text value that should be selected.
# @returns The index of the selection, or -1 if it wasn't there.
proc SetComboBoxValue { cb v } {
   set i [lsearch -exact [$cb cget -values] $v]
   if { $i != -1 } {
      $cb setvalue @$i ;# who knows why you need the @.
   }
   return $i
}

# Set the text of an entry box
proc SetEntryText { e s } {
   set old [$e get]
   $e delete 0 [expr [string length $old] + 1]
   $e insert 0 $s
   return $old
}

# Get the text from a text widget.
proc GetTextBoxText { t } {
   return [$t get 0.0 "end -1 chars"]
}

# Set the text in a text widget.
proc SetTextBoxText { e s } {
   set old [GetTextBoxText $e]
   $e delete 0.0 "end + 1 chars"
   $e delete 0.0 ;# Get rid of last extra new-line
   $e insert 0.0 $s
   return $old
}

} ;# namespace eval BwUtil
