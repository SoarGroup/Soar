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
# A graph data structure
#
# The graph is stored as a table (array)
# Each vertex is given a unique tag. 
#
# graph(__vertices__) = List of tags of all vertices in graph.
# graph($tag) = list of tags of adjacencent vertices, if any
# graph(__fields__) = List of user-specified fields for each vertex
#                    These fields are automatically unset when a vertex is
#                    deleted.
# $graph($tag,field) = Value of vertex field if set
#
# Sample: See test-code at end of file
#
# Requires:
#  util.tcl

namespace eval Graph {

   ##
   # Constant tag used to indicate an invalid vertex tag.
   proc NullVertex {} { return -1 }

##
# Create a new graph
#
# To create a graph:
#     set T [Graph::Create [list name type value]]
#
# @param List of user field names used for each vertex
# @returns Name of graph access proc
proc Create { fields } {
   set graphTag [GetTag Graph]
   variable $graphTag
   upvar 0 $graphTag graph

   # Initialize the basic graph structure
   set graph(__tag__) $graphTag
   set graph(__fields__) $fields
   set graph(__vertices__) {}

   # Add a global procedure for accessing this graph
   # Using eval forces the variable argument list to be expanded so we can pass
   # it to the proc.
   proc ::$graphTag { cmd args } "return \[eval Graph::\$cmd $graphTag \$args\]"

   return $graphTag
}

##
# Destroy a graph returned by Create.
#
# Graph::Destroy $T
proc Destroy { graph } {
   unset Graph::$graph   ;# Remove the table
   rename $graph {}      ;# Remove the access procedure
}


##
# Make a copy of the graph and return it.
#
# @param graphName Tree to copy
# @param fields list of Fields of new graph
# @param copycmd Command to execute to copy fields
proc Copy { graphName { fields "" } { copycmd Graph::BasicVertexCopy } } {
   if { $fields == "" } {
      set fields [GetFields $graphName]
   }
   set new [Create $fields]

   upvar 0 $graphName graph
   upvar 0 Graph::$new newGraph

   set graphTag $graph(__tag__)
   foreach n $graph(__vertices__) {
      lappend newGraph(__vertices__) $n
      set newGraph($n) $graph($n) ;# Copy adjacencies
      $copycmd $new $n $graphTag $n
   }
   return $new
}

##
# Insert a new vertex in the graph.
#
# @param graphName Name of graph
# #param fields List of field/value pairs to initialize the vertex (optional)
# @returns Tag of new vertex
proc InsertVertex { graphName { fields { } } } {
   upvar 0 Graph::$graphName graph

   set tag [GetTag GraphVertices 1]     ;# Get new tag for vertex
   set graph($tag) { }
   lappend graph(__vertices__) $tag    ;# Add to vertex list

   SetAll $graphName $tag $fields ;# Set fields of vertex

   return $tag
}
proc AddEdge { graphName start end } {
   upvar 0 Graph::$graphName graph

   set i [lsearch -exact $graph($start) $end]
   if { $i < 0 } { ;# If the edge doesn't already exist
      lappend graph($start) $end
   }
}
proc RemoveEdge { graphName start end } {
   upvar 0 Graph::$graphName graph

   set graph($start) [ldelete $graph($start) $end]
}
# Set a single field of a vertex
proc Set { graphName vertex name value } {
   upvar 0 Graph::$graphName graph
   set graph($vertex,$name) $value
}
proc SetL { graphName vertex values } {
   upvar 0 Graph::$graphName graph
   foreach { n v } $values {
      set graph($vertex,$n) $v
   }
}
proc SetAll { graphName vertex values } {
   upvar 0 Graph::$graphName graph

   array set avals $values
   foreach { f } $graph(__fields__) {
      if [info exists avals($f)] { ;# Only copy valid fields
         set graph($vertex,$f) $avals($f)
      }
   }
}

##
# Get a field value for a vertex or set of vertices.
#
# If vertex is a single vertex:
#    if name is "" returns a table of field values
#    else returns the field value
# If vertex is a list of vertices, then
#    if name is "" returns a list of tables of field values
#    else raises an error (name must not be empty)
proc Get { graphName vertex { name "" } } {
   upvar 0 Graph::$graphName graph
   if { [llength $vertex] == 1 } {
      if { $name != "" } {
         return $graph($vertex,$name)
      }
      
      set r {}
      foreach f $graph(__fields__) {
         if [info exists graph($vertex,$f)] {
            lappend r $f $graph($vertex,$f)
         } else {
            lappend r $f {}
         }
      }
      return $r
   } else {
      set r {}
      if { $name != "" } {
         foreach v $vertex {
            lappend r $graph($v,$name)
         }
         return $r
      } else {
         error "name parameter to Graph::Get must not be emtpy when\
                mutliple vertices are specified"
      }
   }
}

##
# Delete a vertex from the graph. 
# Removes vertex and all adjacencies
#
# @param graphName Name of graph
# @param tag Tag of vertex to remove
proc DeleteVertex { graphName tag } {
   upvar 0 Graph::$graphName graph

   if ![info exists graph($tag)] {
      return
   }

   # Remove incoming adjacencies
   foreach n [GetInAdjacencies $graphName $tag] {
      RemoveEdge $graphName $n $tag
   }
   unset graph($tag) ;# remove outgoing adjacencies

   # Eliminate the vertex's user fields if set
   foreach f $graph(__fields__) {
      if [info exists graph($tag,$f)] {
         unset graph($tag,$f)
      }
   }
   # Remove vertex from vertex list
   set graph(__vertices__) [ldelete $graph(__vertices__) $tag]

}


# Returns list of user fields in graph
proc GetFields { graphName } {
   upvar 0 Graph::$graphName graph
   return $graph(__fields__)
}
# Returns list of vertex in graph
proc GetVertices { graphName } {
   upvar 0 Graph::$graphName graph
   return $graph(__vertices__)
}

proc EdgeExists { graphName start end } {
   upvar 0 Graph::$graphName graph

   return [expr \
              [lsearch -exact \
                  [GetOutAdjacencies $graphName $start] $end] \
              >= 0]
}
# Returns adjacency list of vertex
proc GetOutAdjacencies { graphName tag } {
   upvar 0 Graph::$graphName graph

   return $graph($tag)
}
proc GetInAdjacencies { graphName tag } {
   upvar 0 Graph::$graphName graph
   set r { }
   foreach n $graph(__vertices__) {
      if { [EdgeExists $graphName $n $tag] } {
         lappend r $n
      }
   }
   return $r
}

##
# Visit vertices in order that they were inserted
# 
# @param graphName Name of graph
# @param cmd Command to invoke at each vertex. graph and vertex tag will
#            be appended to command each invokation
# @param rval If true, a list of return values is constructed
# @returns List of return values of command invokations
proc Map { graphName cmd { rval 1 } } {
   if [IsEmpty $graphName] {
      return
   }
   upvar 0 Graph::$graphName graph
   if { $rval } {
      set o {}
      foreach { t } $graph(__vertices__) {
         lappend o [$cmd $graph(__tag__) $t]
      }
      return $o
   }

   foreach { t } $graph(__vertices__) {
      $cmd $graph(__tag__) $t
   }
}

##
# Visit vertices from a start vertex in a depth first manner.
#
# @param graphName Name of graph
# @param cmd Command to invoke at each vertex. graphName and vertex tag
#            will be append to command each invokation
# @param tag Start vertex. This vertex is not visited so you'll
#            have to expclicity visit it yourself prior to invkoing this
#            proc.
proc MapDepthFirst { graphName cmd { tag TOP } { lvl -1 } } {
   upvar 0 Graph::$graphName graph
   incr lvl
   foreach n $graph(__vertices__) {
      set visited($n) 0
   }
   set visited($tag) 1
   foreach { t } $graph($tag) {
      $cmd $graph(__tag__) $t $lvl
      MapDepthFirst $graphName $cmd $t $lvl
   }
}

##
# Visit vertices from a start vertex in a breadth first manner.
#
# @param graphName Name of graph
# @param tag Start vertex. This vertex is not visited so you'll
#            have to expclicity visit it yourself prior to invkoing this
#            proc.
# @param cmd Command to invoke at each vertex. graphName and vertex tag
#            will be append to command each invokation
proc MapBreadthFirst { graphName tag cmd} {
   upvar 0 Graph::$graphName graph
   
   foreach n $graph(__vertices__) {
      set visited($n) 0
   }
   set visited($tag) 1

   set L [list $tag]

   while { [llength $L] > 0 } {
      set v [lindex $L 0]        ;# Get head of queue
      set L [lrange $L 1 end]    ;# Pop head of queue
      $cmd $graph(__tag__) $v
      foreach n $graph($v) {
         if { !$visited($n) } { ;# If vertex hasn't been visited
            set visited($n) 1
            lappend L $n   ;# add to end of queue
         }
      }
   }
}

##
# Visit outgoing adjacenies of a vertex (one level only)
#
# @param graphName Name of graph
# @param cmd Command to invoke at each vertex. graphName and vertex tag will
#            be append to command each invokation
# @param tag Vertex whose children should be visited.
proc MapOutAdjacencies { graphName cmd tag } {
   upvar 0 Graph::$graphName graph
   foreach { t } $graph($tag) {
      $cmd $graph(__tag__) $t $lvl
   }
}
##
# Visit incoming adjacenies of a vertex (one level only)
#
# @param graphName Name of graph
# @param cmd Command to invoke at each vertex. graphName and vertex tag will
#            be append to command each invokation
# @param tag Vertex whose children should be visited.
proc MapInAdjacencies { graphName cmd tag } {
   upvar 0 Graph::$graphName graph
   set verts [GetInAdjacencies $graphName $tag]
   foreach { t } $verts {
      $cmd $graph(__tag__) $t $lvl
   }
}

##
# Return whether or not the graph is empty
#
proc IsEmpty { graphName } {
   return [expr [Size $graphName] == 0]
}
##
# Returns the number of vertices in the graph
#
proc Size { graphName } {
   upvar 0 Graph::$graphName graph
   return [llength $graph(__vertices__)]
}

##
# Find a vertex with the specified value for the specified field.
#
# @param graphName Name of graph
# @param field Name of field to check
# @param value Value to to compare
# @param verts Set of verts to search, defaults to all verts in graph
# @returns Vertex tag or {} if none found
proc FindVertex { graphName field value { verts {} } } {
   if { $verts == {} } {
      set verts [$graphName GetVertices]
   }
   foreach v $verts {
      if { $value == [$graphName Get $v $field] } {
         return $v
      }
   }
   return [NullVertex]
}

##
# Field-by-field copy function that is the default field copy function for the
# Copy method.
proc BasicVertexCopy { destTree destVertex srcTree srcVertex } {
   $destTree SetAll $destVertex [$srcTree Get $srcVertex]
}
##
# A simple function that can be passed to MapDepthFirst to
# print out a vertex and all of its fields
#
proc Print { graph vertex { lvl -1 } } {
   # Print fields of vertex
   puts "$vertex->[$graph GetOutAdjacencies $vertex]"
   puts "$vertex<-[$graph GetInAdjacencies $vertex]"
   foreach { f  v } [$graph Get $vertex] {
      PrintTabs $lvl
      puts "$f = $v"
   }
}

proc compareVerts { g field v1 v2 } {
   return [string compare [$g Get $v1 $field] [$g Get $v2 $field]]
}
proc SortVertListByField { g L field { comp compareVerts } } {
#   proc compare { v1 v2 } \
#      "return [string compare [$g Get \$v1 $field] [$g Get \$v2 $field]]"
   return [lsort -command "$comp $g $field " $L]
}

proc SortOutAjacencies { graphName v field { comp compareVerts } } {
   upvar 0 Graph::$graphName g
   set g($v) [$graphName SortVertListByField \
                                 [$graphName GetOutAdjacencies $v] \
                                 $field \
                                 $comp]
   return $g($v)
}

} ;# End Graph namespace

IfStandAlone { ;# Some test code

set T [Graph::Create { name }] ;# Create a graph where each vertex has a 'name' field
set n1 [$T InsertVertex] 
$T Set $n1 name HOWDY 

# Insert a vertex and initialize its name in one call
set n2 [$T InsertVertex { name DOODY }]
$T AddEdge $n1 $n2

# Insert an unitialized vertex
set n3 [$T InsertVertex]
# Set a field of a vertex
$T Set $n3 name JUDY
$T AddEdge $n2 $n3

set n4 [$T InsertVertex { name TEX } ]
$T AddEdge $n1 $n4

set n5 [$T InsertVertex { name MEX } ]
$T AddEdge $n1 $n5

set n6 [$T InsertVertex { name aaa } ]
$T AddEdge $n1 $n6

$T SortOutAjacencies $n1 name

# Print the graph
$T MapBreadthFirst $n1 Graph::Print

# Delete a vertex
$T DeleteVertex $n3
puts "---------------------"

$T Map Graph::Print

puts "---------------------"

}
