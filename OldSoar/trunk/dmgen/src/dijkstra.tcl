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
# Implementation of Dijkstra's single-source shortest paths algorithm.
#
# requires:
#  graph.tcl

namespace eval Dijkstra {

   variable d
   set d([Graph::NullVertex]) {}
   variable pi
   set pi([Graph::NullVertex]) {}
   
   proc Inf { } { return 99999999 }

   ##
   # Calculate single source shortest paths on a graph using Dijkstra's
   # algorithm.
   # Returns an adjacency array.
   #
   # @param g Graph to process
   # @param w Edge weight function. Takes two vertices and returns the weight
   #           of the edge between them. See UnitWeight below.
   # @param s Source vertex
   # @returns An adjacency array. See test code below.
   proc ShortestPaths { g w s } {
      variable d
      variable pi
      unset d
      unset pi

      initSingleSource $g $s
      set s {}
      set q [$g GetVertices]
      while { [llength $q] > 0 } {
         set u [getQMin q]
         set s [lunion $s $u]
         foreach v [$g GetOutAdjacencies $u] {
            relax $u $v $w
         }
      }
      return [array get pi]
   }
   ##
   # Get path to a vertex after a shortest path calculation
   # The source vertex is implicit in the rpi table that come
   # from ShortestPaths.
   #
   # @param v Vertex we want the path to
   # @param rpi Name of an array initialized from running Dijkstra
   #            ShortestPaths on $g $s.
   # @returns A list of vertices starting at s and ending at v
   proc GetVertexPath { rpi v } {
      upvar $rpi pi

      # the path is built in reverse order
      set path $v
      set v $pi($v)
      set NV [Graph::NullVertex]
      while { $v != $NV } {
         set path [concat $v $path]
         set v $pi($v)
      }
      return $path
   }

   ##
   # A weight function that returns a unit weight for all edges.
   proc UnitWeight { u v } { return 1 }

   ##
   # Algorithm initialization function. Called internally 
   proc initSingleSource { g s } {
      variable d
      variable pi
      foreach v [$g GetVertices] {
         set d($v) [Inf]
         set pi($v) [Graph::NullVertex]
      }
      set d($s) 0
   }

   ##
   # Vertex 'relaxation' used internally.
   proc relax { u v w } {
      variable d 
      variable pi
      if { $d($v) > $d($u) } {
         set d($v) [expr $d($u) + [$w $u $v]]
         set pi($v) $u
      }
   }

   proc getQMin { qref } {
      upvar 1 $qref q 
      variable d
      variable pi

      set mv [lindex $q 0]
      if { [llength $q] == 1 } {
         set q {}
         return $mv
      }
      foreach v [lrange $q 1 end] {
         if { $d($v) < $d($mv) } {
            set mv $v
         }
      }
      set q [ldelete $q $mv]
      return $mv
   }

   proc test { } { ;# Some test code...
      set g [Graph::Create { name }] ;# Create a graph where each vertex has a 'name' field
      set n1 [$g InsertVertex { name 1 }]
      set n2 [$g InsertVertex { name 2 }]
      set n3 [$g InsertVertex { name 3 }]
      set n4 [$g InsertVertex { name 4 }]
      set n5 [$g InsertVertex { name 5 }]

      $g AddEdge $n1 $n2
      $g AddEdge $n2 $n3
      $g AddEdge $n2 $n4
      $g AddEdge $n4 $n5
      $g AddEdge $n5 $n3
#
# Something like this:
#    2--3
#   / \  \
#  1   4--5
#
      parray Graph::$g
      # calculate shortest paths to all nodes from 1
      array set pi [ShortestPaths $g UnitWeight $n1]

      # print out paths to each node
      puts [join [$g Get [GetVertexPath pi $n1] name] " -> "]
      puts [join [$g Get [GetVertexPath pi $n2] name] " -> "]
      puts [join [$g Get [GetVertexPath pi $n3] name] " -> "]
      puts [join [$g Get [GetVertexPath pi $n4] name] " -> "]
      puts [join [$g Get [GetVertexPath pi $n5] name] " -> "]
   }
}

IfStandAlone { ;# Some test code
   Dijkstra::test
}
