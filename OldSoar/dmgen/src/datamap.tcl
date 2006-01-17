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
# Datamap management code. Manages a set of datamaps created by the user.
# Also has the Generate function for generating a datamap from a set of
# productions. Here's the typical usage:
#     ... Load productions ...
#     set dm [Datamap::Create "Name Of Datamap"]
#     Datamap::Generate $dm
#     # Now you can use the various accessor functions to get at the produced
#     # datamap.
# 
# Requires:
#     util.tcl
#     graph.tcl
#     log.tcl
#     production.tcl
#     partition.tcl
#     merge.tcl
#     config.tcl 
#     soarutil.tcl

namespace eval Datamap {

   # Returns a list of valid vertex field names
   #
   # name - Name of the "attribute"
   # value - List of values of the attribute
   # side - Which side of the production was the attribute on 
   #           L = left
   #           R = right 
   #           B = both
   # type - Type of the attribute (int string identifier unknown)
   # range - currently unused
   # link - A link to another problemspace or operator. Is a list of
   #        aliases { type root path }, e.g.
   #           { { S top-ps {} } { S any-ps {} } }
   #
   # prods - List of production names that contributed to this node.
   #
   proc VertexFields {} {
      return [list name \
                   value \
                   side  \
                   type \
                   range \
                   link \
                   special \
                   prods \
                   comment \
                   patched]
   }
   # Returns a 'struct' of default vertex field values
   proc DefVertexFields {} {
      return [list name unknown \
                   value {} \
                   side  B  \
                   type identifier \
                   range {} \
                   link {} \
                   special {} \
                   prods {} \
                   comment "" \
                   patched {}]
   }

   variable datamaps
   set datamaps(0) 0

   ##
   # Creates a datamap (problem-spaces + operators) and returns a handle to it
   #
   # @param name User-specified name given to the datamap. 
   # @returns A handle to the datamap for use in later calls.
   proc Create { { name "" } } {
      variable datamaps
      set dm [GetTag DM]
      set datamaps($dm,name) $name
      return $dm
   }

   ##
   # Clear the contents of a datamap. Good for resetting before another call
   # to Generate.
   #
   # @param dm Datmap handle.
   proc Clear { dm } {
      variable datamaps
      # Clean up graphs
      foreach n [array names datamaps $dm,*,*,graph] {
         Graph::Destroy $datamaps($n)
      }
      # Clean up entries
      foreach n [array names datamaps $dm,*,*,*] {
         unset datamaps($n)
      }
   }
   ##
   # Remove a datamap from memory. The handle will be useless afterward.
   #
   # @param dm handle of datamap to destroy.
   proc Destroy { dm } {
      variable datamaps
      Clear $dm
      unset datamaps($dm,name)
   }

   ##
   # Returns the user specified name of a datamap.
   proc GetName { dm } {
      variable datamaps
      if ![info exist datamaps($dm,name)] {
         return ""
      }
      return $datamaps($dm,name)
   }
   ##
   # Create a datamap structure for a problem-space or operator
   #
   # @param dm Datamap handle
   # @param type S (problem space) or O (operator)
   # @param name Name of ps or op (no whitespace!)
   # @returns Handle to ps or op
   proc createPsOrOp { dm type name } {
      set g [Graph::Create [VertexFields]]
      set s [$g InsertVertex [DefVertexFields]]

      set names(S) "state"
      set names(O) "operator"

      $g Set $s name $names($type)

      variable datamaps
      set datamaps($dm,$type,$name,graph) $g
      set datamaps($dm,$type,$name,start) $s
      return "$dm,$type,$name"
   }
   ##
   # Same as createPsOrOp, except that if the requested ps or op already
   # exists, the existing one is returned rather than creating a new one.
   #
   # @param dm Datamap handle
   # @param type S (problem space) or O (operator)
   # @param name Name of ps or op (no whitespace!)
   # @returns Handle to ps or op
   proc findOrCreatePsOrOp { dm type name } {
      set h [GetPsOrOp $dm $type $name]
      if { $h != {} } {
         return $h
      }
      return [createPsOrOp $dm $type $name]
   }
   ##
   # Create a new problem-space and return a handle to it
   #
   # @param dm Datamap handle
   # @param name Name of problem-space (no whitespace!)
   # @returns a new problem-space handle
   proc CreateProblemSpace { dm name } {
      return [createPsOrOp $dm S $name]
   }
   proc FindOrCreateProblemSpace { dm name } {
      return [findOrCreatePsOrOp $dm S $name]
   }
   ##
   # Create a new operator and return a handle to it
   #
   # @param dm Datamap handle
   # @param name Name of operator (no whitespace!)
   # @returns a new operator handle
   proc CreateOperator { dm } {
      return [createPsOrOp $dm O $name]
   }
   proc FindOrCreateOperator { dm name } {
      return [findOrCreatePsOrOp $dm O $name]
   }

   ##
   # Remove a problem space or operator from datamap
   proc removePsOrOp { h } {
      variable datamaps
      foreach n [array names datamaps $h,*] {
         unset datamaps($n)
      }
   }
   proc RemoveProblemSpace { h } { removePsOrOp $h }
   proc RemoveOperator { h } { removePsOrOp $h }

   ##
   # Returns a list of ps or op handles in the datamap.
   #
   # @param dm Datamap handle
   # @param type S or O
   # @returns List of problem-space or operator handles
   proc getPsOrOps { dm type } {
      variable datamaps
      set r {}
      foreach d [array names datamaps $dm,$type,*,graph] {
         lappend r [join [lrange [split $d ","] 0 2] ","] ;#Handle
      }
      return $r
   }

   proc GetProblemSpaces { dm } { return [getPsOrOps $dm S] }
   proc GetOperators { dm } { return [getPsOrOps $dm O] }

   ##
   # Generate or append to the given datamap using the information in the given
   # productions. If no productions are given, then all loaded Soar productions
   # are used.
   #
   # @param dm Datamap handle returned by Create
   # @param prods Optional list of names of productions to process
   proc Generate { dm { prods {} } } {

      Log ""
      Log [clock format [clock seconds]]
      Log "Datamap generation initiated for datamap [GetName $dm]"
      if { [llength $prods] == 0 } {
         Log "No productions specified, using all loaded productions."
         set prods [SoarUtil::GetSoarProductions]
      }
      set total 0
      set passedParse 0
      set failedParse 0
      set psOrOpFound 0
      set noPsOrOpFound 0
      set aliasSet [DmGenCfg::GetAliasSet]
      foreach p $prods {
         Log "Parsing production: $p"
         incr total
         set g [Production::Parse $p]
         if { $g == {} } {
            LogError "Error parsing production: $p"
            incr failedParse
         } else {
            incr passedParse
            set parts [Partition::PartitionProduction $g $aliasSet]
            if { $parts != {} } {
               incr psOrOpFound
               Log "Processing production partitions"
               ProcessProd $dm $p $g $parts
               # If the user wants topstate stuff also copied to the ps that
               # it's used in, we do it one more time with slightly different
               # options. This is a bit of a hack, but...
               if { [DmGenCfg::Get CopyTopPs] && [DmGenCfg::Get FillTopPs] } {
                  DmGenCfg::Set FillTopPs 0
                  set parts [Partition::PartitionProduction $g $aliasSet]
                  if { $parts != {} } {
                     Log "Copying top-state to calling problem space"
                     ProcessProd $dm $p $g $parts
                  }
                  DmGenCfg::Set FillTopPs 1
               }
            } else {
               incr noPsOrOpFound
               Log "No problem spaces or operators identified"
            }
         }
      }
      Log "Updating attribute types"
      foreach ps [concat [GetProblemSpaces $dm] [GetOperators $dm]] {
         updateAttrTypes $ps
      }

      Log "Pruning extraneous problem-space and operator names"
      foreach ps [concat [GetProblemSpaces $dm] [GetOperators $dm]] {
         prunePsAndOpNames $ps
      }

      # Sort attributes alphabetically by name
      Log "Sorting attributes"
      SortAttributes $dm [DefAttrSortPriority]

      Log [clock format [clock seconds]]
      Log "Datamap Generation complete:"
      Log "   $total productions attempted"
      Log "   $failedParse productions failed to parse"
      Log "   $passedParse productions parsed successfully"
      Log "      $psOrOpFound productions had identifiable problem space or operator"
      Log "      $noPsOrOpFound productions had NO identifiable problem space or operator"
      LogFlush
   }

   ##
   # Get handle to a PS or OP from its type and name
   # @param dm datamap handle
   # @param type S or O
   # @param name Name of ps or op
   proc GetPsOrOp { dm type name } {
      variable datamaps
      set h "$dm,$type,$name"
      if [info exists datamaps($h,graph)] {
         return $h
      }
      return {}
   }

   ##
   # Get handles to particular PS or op of a datamap. Returns {} if it
   # doesn't exist.
   proc GetProblemSpace { dm name } { return [GetPsOrOp $dm S $name] }
   proc GetOperator { dm name } { return [GetPsOrOp $dm O $name] }

   # What datamap does this handle belong to?
   proc GetDatamap { h } {
      return [lindex [split $h ","] 0]
   }
   # What is the name of the PS or OP that this handle points to.
   proc GetPsOrOpName { h } {
      return [lindex [split $h ","] end]
   }
   ##
   # What is the type of this handle
   #  Problem-space -> S
   #  Operator -> O
   proc GetType { h } {
      return [lindex [split $h ","] 1]
   }
   # Is this handle for a problem space?
   proc IsProblemSpace { h } {
      return [expr [string compare [GetType $h] "S"] == 0]
   }
   # Is this handle for an operator?
   proc IsOperator { h } {
      return [expr [string compare [GetType $h] "O"] == 0]
   }
   # Get the graph for this handle
   proc GetGraph { h } {
      variable datamaps
      return $datamaps($h,graph)
   }
   # Get the start vertex for the graph of this handle
   #  i.e. The state or operator vertex.
   proc GetStartVertex { h } {
      variable datamaps
      return $datamaps($h,start)
   }

   ##
   # Find a vertex given a vertex name path
   #
   # @param dmGraph Graph
   # @param start Vertex to start search from
   # @param path List of vertex names e.g. { state operator name }
   # @returns The vertex tag, or Graph::NullVertex if not found
   proc FindVertexFromPath { dmGraph start path } {
      set NV [Graph::NullVertex]

      set v $start
      
      set n [lindex $path 0]
      if { [string compare [$dmGraph Get $v name] $n] != 0 } {
         return $NV
      }
      if { [llength $path] == 1 } {
         return $v
      }
      set path [lrange $path 1 end]
      
      foreach s $path {
         set v [$dmGraph FindVertex name $s [$dmGraph GetOutAdjacencies $v]]
         if { $v == $NV } {
            return $NV
         }
      }
      return $v
   }

   ##
   # Given a datamap graph, a start vertex and a path of node names
   # Finds the node at the end of the path, or creates the path if
   # it doesn't exist yet.
   # 
   # This function is slightly different from FindVertexFromPath above
   # in that it creates the path if it doesn't exist yet and the
   # path argument does NOT include the first 'state' or 'operator'
   # element. Inconsistent, yes, but that's the way it goes.
   proc FindOrCreateVertexFromPath { dmGraph start path } {
      set NV [Graph::NullVertex]
      
      set v $start ;# return start when path is empty
      foreach s $path {
         set v [$dmGraph FindVertex name $s [$dmGraph GetOutAdjacencies $start]]
         if { $v == $NV } {
            set v [$dmGraph InsertVertex [Datamap::DefVertexFields]]
            $dmGraph Set $v name $s
            $dmGraph AddEdge $start $v
            Log "Created vertex [$dmGraph Get $start name] --> $s"
         }
         set start $v
      }
      return $v
   }

   ##
   # Default priority list for datamap attribute sorting.  
   proc DefAttrSortPriority { } {
      return [list { \
               attribute \
               impasse \
               operator \
               problem-space \
               superstate \
               top-state \
               io \ 
               name \
             }]
   }

   ##
   # Sort the sub nodes of each node by name.
   # Names are sorted alphabetically unless the appear in the priority list. If
   # a name appears in the priority list, it will sort higher than any other
   # name, unless the other name is also in the priority list and appears
   # earlier.
   #
   # After this is called, a call to GetOutAdjacencies of any graph vertex will 
   # return the adjacencies in the sorted order.
   #
   # @param dm    Datamap to sort
   # @param pri   Name priority list
   proc SortAttributes { dm pri } {
      set stype(float) -real
      set stype(int) -integer
      set stype(string) -ascii

      foreach ps [concat [GetProblemSpaces $dm] [GetOperators $dm]] {
         set g [GetGraph $ps]
         foreach v [$g GetVertices] {
            $g SortOutAjacencies $v name "Datamap::byNameSortFunc $pri "
            
            # Sort the values of the attribute if the type is known.
            set t [$g Get $v type]
            if { $t != "unknown" && $t != "identifier" } {
               set vals [$g Get $v value]
               $g Set $v value [lsort $stype($t) $vals]
            }
         }
      }
   }

   ##
   # Vertex comparison function for sorting nodes by name using a priority list.
   #
   # @param priority Priority list
   # @param g The graph (passed by Graph::SortOutAjacencies)
   # @param field The field (passed by Graph::SortOutAjacencies)
   # @param v1 First vertex
   # @param v2 Second vertex
   # @returns -1,0, or 1 indicating result of comparing v1 to v2
   proc byNameSortFunc { priority g field v1 v2 } {
      set n1 [$g Get $v1 name]
      set n2 [$g Get $v2 name]

      set i1 [lsearch $priority $n1]
      set i2 [lsearch $priority $n2]

      if { $i1 >= 0 && $i2 >= 0 } { ;# They're both in the priority list
         return [expr $i1 - $i2]
      } elseif { $i1 >= 0 } { ;# v1 is in the priority list 
         return -1
      } elseif { $i2 >= 0 } { ;# v2 is in the priority list
         return 1
      } else { ;# neither is in the priority list, sort alphabetically, case-insensitive
         return [string compare [string tolower $n1] [string tolower $n2]]
      }
   }

   ##
   # Fix up the type of the attributes after generation
   proc updateAttrTypes { psOrOp } {
      set g [GetGraph $psOrOp]
      foreach v [$g GetVertices] {
         if { [$g Get $v link] != {} || [llength [$g GetOutAdjacencies $v]] > 0 } {
            $g Set $v type identifier
         } else {
            $g Set $v type [SoarUtil::GuessSoarTypeFromList [$g Get $v value]]
         }
      }
   }

   proc prunePsAndOpNames { psOrOp } {
      if [IsProblemSpace $psOrOp] {
         set spec "ProblemSpaceSpec"
      } else {
         set spec "OperatorSpec"
      }
      set g [GetGraph $psOrOp]
      set name [GetPsOrOpName $psOrOp]

      foreach v [$g GetVertices] {
         if { [$g Get $v special] == $spec } {
            set vals [$g Get $v value]
            if { [llength $vals] > 0  && \
                 [lsearch -exact $vals $name] != -1 } {
               $g Set $v value $name
            }
         }
      }
   }
} ;# namespace Datamap

