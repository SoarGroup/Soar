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
# Code for representing productions as graphs
#
# A Production is represented as a graph. Each vertex has the following
# properties:
#  name - Name of vertex (state, name, etc)
#  type - Type of vertex (identifier, string, float, etc)
#  value - Value of vertex (only meaningful for string, float, etc)
#  special - 
#  isState - true if the structure under this vertex is a state
#  isVariablized - true if this vertex represents a variablized attribute
#  valRelOps - Relational ops that we're used in the test of the value
#              e.g., not_equal, less, equal, etc.
#  side - which side of the production it came from (L, R, or B).
#
# Requires:
#   parser.tcl
#   util.tcl
#   graph.tcl
#   soarutil.tcl

namespace eval Production {
   # Properties of each node
   proc NodeProps {} { return { 
      name 
      type 
      value
      special
      isState 
      isVariablized
      valRelOps
      side
   } }
   # Property defaults values
   proc NodePropDefaults {} { return {
      name                 "unknown" 
      type                 unknown 
      value                {}
      special              {}
      isState              0
      isVariablized        0
      valRelOps            {}
      side                 B
   } }

   # Possible values for 'type' node property
   #  DR - removed enum 7/25/02
   proc NodeTypes {} { return { unknown identifier string int float } }
 
   # Possible values for 'special' node property
   # Should ProblemSpace be removed? I'm not sure if it's ever actually
   # used. partition.tcl appears to just use ProblemSpaceSpec.
   proc SpecialTypes {} { return { 
      ProblemSpace 
      ProblemSpaceSpec
      Operator 
      OperatorSpec
      TopState 
      SuperState 
   } }

   proc StateSpec        {} { return state }

   variable psSpec ;# internal problem-space spec list

   # Use this for TAS-style (state.problem-space.name)
   set psSpec { problem-space name } 
   # Use this for Visual Soar-style (state.name)
   #set psSpec { name } 

   ##
   # PS spec 'constant'. Set the value by calling with a single
   # argument.
   proc ProblemSpaceSpec { { s {} } } { 
      variable psSpec
      if { [llength $s] > 0 } {
         set psSpec $s
      }
      return $psSpec
   }

   proc OperatorSpec     {} { return { operator name } }
   proc TopStateSpec     {} { return { top-state } }
   proc SuperStateSpec   {} { return { superstate } }

   ##
   # Parse a production
   #
   # @param name Name of production to parse
   # @returns The graph of the production
   proc Parse { name } {
      set graph [parse $name]

      return $graph
   }

   ##
   # Return all nodes with isState property set to true
   #
   # @param graph Production graph returned by Parse
   # @returns List of node tags
   proc GetStates { graph } {
      set r {}
      foreach v [$graph GetVertices] {
         if [$graph Get $v isState] {
            lappend r $v
         }
      }
      return $r
   }

   ##
   # Return all nodes with 'special' attribute set to ProblemSpace
   #
   # @param graph Production graph
   # @param state State node to start search at
   # @returns List of node tags
   proc GetProblemSpaces { graph state } {
      set r {}
      foreach v [$graph GetVertices] {
         if { [$graph Get $v special] == "ProblemSpace" } {
            lappend r $v
         }
      }
      return $r
   }

   ##
   # Return all nodes with 'special' attribute set to Operator
   #
   # @param graph Production graph
   # @returns List of node tags
   proc GetOperators { graph } {
      set r {}
      foreach v [$graph GetVertices] {
         if { [$graph Get $v special] == "Operator" } {
            lappend r $v
         }
      }
      return $r
   }

   ##
   # Parse a loaded Soar production and return its graph
   proc parse { name } {
      set buf [SoarUtil::GetSoarPrintInternal $name]
      if { $buf == {} } {
         puts "No loaded production: $name"
         return {}
      }

      #  Parse the production into WMEs
      set wmes [parse_production $buf]
      if {$wmes == 0} {
         puts "Could not parse production $name"
         puts $buf
         return {}
      }
      return [buildProduction $wmes]
   }

   proc sidesAreCompatible { s0 s1 } {
      if {$s0 == $s1 } {
         return 1
      } elseif { $s0 == "B" || $s1 == "B" } {
         return 1
      }
      return 0
   }

   proc MergeSideValues { s0 s1 } {
      if { $s0 != $s1 } {
         return B
      } else {
         return $s0
      }
   }

   ##
   # Builds a production graph given the list returned b parse_production
   #
   # Rewritten for file version 1.9 to handle LHS/RHS asymmetry
   #
   # @param list of wmes { Side Id AttrName Value }
   proc buildProduction { wmes } {
      set wmes [lunique $wmes] ;# Remove repeats

#      foreach s $wmes {
#         puts $s
#      }

      set graph [Graph::Create [NodeProps]]
      array set props [NodePropDefaults]
      set props(name) TOP
      set props(side) L
      # Insert a fake TOP node to start from
      set top [$graph InsertVertex [array get props]]

      set tids($top) TOP           ;# maps nodes to id values (<*>)
      set tIdToNodes(TOP) $top     ;# maps ids to nodes (multi-map)
      set tNodeReroutes($top) $top ;# lookup table rerouting node when they are
                                   ;# replaced because they're a duplicate
      
      set states {}
      foreach s $wmes {
         set side [lindex $s 0]
         set id [lindex $s 1]
         set attr [lindex $s 2]
         set val [lindex $s 3]

         set attrRelOps [lindex $s 4]
         set valRelOps [lindex $s 5]

         unset props
         array set props [NodePropDefaults]
         set props(name) $attr
         set props(side) $side
         set props(valRelOps) $valRelOps
         set props(isVariablized) [isVariable $attr]
         if { [string compare $attr "state"] == 0 && [string compare $id "TOP"] == 0 } {
            set props(isState) 1
         }

         set n [$graph InsertVertex [array get props]]
         set tNodeReroutes($n) $n ;# Add identity mapping to reroute table for now

         # Accumulate list of state node for later...
         if $props(isState) {
            lappend states $n
         }

         # Is this a terminal node?
         if { [isVariable $val] } then { ;# non-terminal
            set tids($n) $val
            if ![info exists tIdToNodes($val)] {
               set tIdToNodes($val) {}
            }
            set tIdToNodes($val) [lunion $tIdToNodes($val) $n]
         } else { ;# terminal, just store the value
            $graph Set $n value $val
         }
         set tparents($n) $id
      }
#      parray tparents
#      parray tIdToNodes
      # Now the graph has all of its vertices. Let's go through and set
      # parents making sure we take which side of the production the 
      # node was found on.
      foreach n [$graph GetVertices] {
         if ![info exists tparents($n)] { ;# no parents
            continue
         }
         # We don't need to merge states, just leave them...
         if { [string compare $tparents($n) TOP] == 0 } {
            $graph AddEdge $top $n
            continue
         }
         set side [$graph Get $n side]
         set name [$graph Get $n name]

         # Get all the nodes that match this parent id
         if { ![info exists tIdToNodes($tparents($n))] } {
            LogWarning "No node for parent $tparents($n), skipping"
            continue
         }
         set potParents $tIdToNodes($tparents($n)) ;# potential parents
#         puts "$name - $n"
#         puts "pot = $potParents"
         set mySideParents {}    ;# parents on the same side as me
         set otherSideParents {} ;# parents on the other side from me
         foreach p $potParents {
            if { $p != $n  && $p < $n } { ;# prevent self-loops
               # Which side is the potential parent on relative to me?
               # Use the reroute table in case the parent has moved...
               if [sidesAreCompatible $side [$graph Get $p side]] {
                  lappend mySideParents $tNodeReroutes($p)
               } else {
                  lappend otherSideParents $tNodeReroutes($p)
               }
            }
         }
#         puts "my = $mySideParents"
#         puts "other = $otherSideParents"
         # We give priority to parents that are on the same side as
         # the node.
         set parentsFound 0
         set realParents {}
         if { [llength $mySideParents] > 0 } {
            set realParents $mySideParents
            set parentsFound 1
         } elseif { [llength $otherSideParents] > 0 } {
            set realParents $otherSideParents
            set parentsFound 1
         } 
         if { $parentsFound } {
            # now reparent node under each of its true parents
            foreach p $realParents {
#               puts "parenting under $p"
               # Does this parent already have a child by the same name?
               set sibs [$graph GetOutAdjacencies $p]
               set fnode ""
               foreach s $sibs {
                  if { [$graph Get $s name] == $name } { ;# Yes!
                     set fnode $s
                  }
               }
               # If there is a sibling with the same name, merge our properties
               # in with those of the sibling and don't reparent ourself...
               if { $fnode != "" } {
                  $graph Set $fnode value [lunion [$graph Get $fnode value] [$graph Get $n value]]
                  # Now when are children try to find their parent they'll find
                  # this replacement instead...
                  set tNodeReroutes($n) $fnode
                  # other stuff?
                  $graph Set $fnode side [MergeSideValues [$graph Get $fnode side] $side]
               } else {
                  $graph AddEdge $p $n ;# just set the node's parent
               }
            }
         } else {
            puts "No parents found!"
         }
      }
      # Find all of the nodes that were duplicates and prune them from the
      # graph...
      foreach n [$graph GetVertices] {
         if { $n != $top && [llength [$graph GetInAdjacencies $n]] == 0 } {
            $graph DeleteVertex $n
         }
      }
      # Determine special field for all nodes...
      foreach n [$graph GetVertices] {
         set sfVisitTable($n) 0
      }
      foreach s $states {
         setSpecialFields $graph $top $s {} sfVisitTable
      }
      # Try to guess types of all vertices...
      $graph Map Production::setTypeField

      return $graph
   }

   ##
   # Determine any special attributes of a node such as problem-space, state,
   # operator, etc.
   #
   # @param g The graph
   # @param parents List of parents of this node
   # @param n The node
   # @param path The node name path (list of names)
   # @param visitTableName A hashtable to indicate when a node has already
   #                       been visited.
   proc setSpecialFields { g parents n path visitTableName } {
      upvar $visitTableName visitedTable
      set visitedTable($n) 1 ;# mark this node as visited
      set name [$g Get $n name]
      set parent [lindex $parents end]
      set nParents [llength $parents]
      set p [concat $path $name]
      set notfound 1

      # This mess is mostly to handle finding problem-spaces no matter how
      # they're specified in the productions (e.g. ^problem-space.name for TAS,
      # ^name for Visual Soar code, etc). The code below for OperatorSpec et al 
      # should also look like this, but ^operator.name is more consistent so
      # we'll worry about that when the time comes...

      # Let's see if we've found a problems space specifier 
      # (e.g. ^problem-space.name)
      set psSpec [ProblemSpaceSpec]    ;# The spec
      set psSpecLen [llength $psSpec]  ;# length of spec
      # Does the current path look like the spec?
      if { [lcompare [ltail $p $psSpecLen] $psSpec] } {
         # ok, is the parent of the top of the spec a state?
         # i.e., we have to have state.problem-space.name or
         # superstate.problem-space.name. This is crucial if the spec is just
         # ^name as in the case of visual soar generated code.
         set specParent [lindex $parents [expr $nParents - $psSpecLen]]
         if [$g Get $specParent isState] {
            $g Set $n special ProblemSpaceSpec
            $g Set $parent special ProblemSpace
            set notfound 0
         }
      }

      if { $notfound && [lcompare [ltail $p [llength [OperatorSpec]]] [OperatorSpec]] } {
         $g Set $n special OperatorSpec
         $g Set $parent special Operator
      } elseif { [lcompare [ltail $p [llength [TopStateSpec]]] [TopStateSpec]] } {
         $g Set $n special TopState
         $g Set $n isState 1
      } elseif { [lcompare [ltail $p [llength [SuperStateSpec]]] [SuperStateSpec]] } {
         $g Set $n special SuperState
         $g Set $n isState 1
      }

      # depth first left-to-right traversal
      foreach c [$g GetOutAdjacencies $n] {
         if { !$visitedTable($c) } {
            setSpecialFields $g [concat $parents $n] $c $p visitedTable
         }
      }
   }

   ##
   # Is the string in 'val' a Soar variable, i.e. <*>
   proc isVariable { val } {
      return [string match <*> $val]
   }
   ##
   # Get list of variables from 'list'
   proc getVarList { list } {
      set olist {}
      foreach v $list {
         if [isVariable $v] {
            lappend olist $v
         }
      }
      return $olist
   }
   ##
   # Get list of non-variables from 'list'
   proc getValueList { list } {
      set olist {}
      foreach v $list {
         if ![isVariable $v] {
            lappend olist $v
         }
      }
      return $olist
   }

   proc extractAttrName { s } {
      set e [expr [string last "," $s] + 1]
      return [string range $s $e end]
   }
   proc extractVarName { s } {
      set e [expr [string first "," $s] - 1]
      return [string range $s 0 $e]
   }
   ##
   # Tries to guess the type of a node.
   proc setTypeField { graph v } {
      set name [$graph Get $v name]

      if [llength [$graph GetOutAdjacencies $v]] {
         $graph Set $v type identifier
         return
      }
      set val [$graph Get $v value]
      set len [llength $val]
      if { $len == 0 } {
         $graph Set $v type unknown
      } else {
         $graph Set $v type [SoarUtil::GuessSoarTypeFromList $val]
      }
   }

} ;# namespace Production

IfStandAlone { ;# Some test code
   lappend auto_path $env(SOAR_LIBRARY)
   package require Soar

   sp {test0
      (state <s> ^operator <o>
                 ^problem-space.name jim
                 ^judy <name>)
      (<o> ^name bob)
      (state <s2> ^operator <o2>
                 ^problem-space.name jim2)
      (<o2> ^name bob2)
      (<o2> ^{ <vars> << var1 var2 var3 >> } <var>)
   -->
      (write (crlf) |Judy message from | <name>)
   }

   set g [Production::Parse test0]
   set t [$g FindVertex name TOP]
   $g MapBreadthFirst $t Graph::Print

   set states [Production::GetStates $g]
   puts "States = $states" 
   foreach s $states {
      puts "state $s"
      puts "   ProblemSpaces = [Production::GetProblemSpaces $g $s]" 
      puts "   Operators = [Production::GetOperators $g $s]" 
   }

} ;# End test code
