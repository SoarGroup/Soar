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
# Code for merging a production into a datamap...
#
# Requires:
#     config.tcl
#     datamap.tcl
#     graph.tcl

## 
# Process a partitioned production, merging it into the appropriate
# problem spaces and operators of the passed datamap.
#
# @param dm Datamap to fill
# @param name Name of source production
# @param graph Production graph
# @param part Partition info returned by PartitionProduction
proc ProcessProd { dm name graph part } {
   # $part is a list of 3-tuples, described in partition.tcl
   foreach { start targets verts } $part {
      foreach { type root path } $targets {
         if { $type == "S" } { ;# it's a problem space
            set h [Datamap::FindOrCreateProblemSpace $dm $root]
         } else { ;# it's an operator
            set h [Datamap::FindOrCreateOperator $dm $root]
         }
         set hGraph [Datamap::GetGraph $h]
         set hStart [Datamap::GetStartVertex $h]
         set hStart [Datamap::FindOrCreateVertexFromPath $hGraph $hStart $path]
         MergeProdIntoDatamap $name $graph $start $verts \
                              $hGraph \
                              $hStart
      }
   }
}

##
# Merge a partitioned production into a particular problem space or
# operator graph.
#  This is just a setup function. The real work is done in the recursive
#  mergeProdIntoDatamap function.
#
# @param pName Name of production
# @param pGraph Production graph
# @param pState 'state' or 'operator' vertex of production
# @param part Production partition info
# @param dmGraph Target ps or op graph
# @param dmState 'state' or 'operator' vertex of ps or op
proc MergeProdIntoDatamap { pName pGraph pState part dmGraph dmState } {
   global gMergeMap gMergeParts
   if [info exists gMergeMap] { unset gMergeMap }
   if [info exists gMergeParts] { unset gMergeParts }
   foreach { v ps } $part {
      set gMergeMap($v) [Graph::NullVertex]
      set gMergeParts($v) $ps ;# external PS links
   }
   mergeProdIntoDatamap $pName $pGraph $pState $dmGraph $dmState
}

##
# Merge a partitioned production into a particular problem space or
# operator graph.  Performs a simultaneous depth-first traversal of
# the production graph and the datamap graph, filling in the datamap
# graph as necessary. 
#
# Uses two global variables (gMergeMap and gMergeParts) initialized
# by MergeProdIntoDatamap to keep track of where it's been (to avoid
# cycles) and to make links when the boundary of a partition is
# reached. gMergeMap is a map from production vertex to datamap vertex.
#
# @param pName Name of production
# @param pGraph Production graph
# @param pTag current production vertex
# @param dmGraph Target ps or op graph
# @param dmTag current ps or op vertex
proc mergeProdIntoDatamap { pName pGraph pTag dmGraph dmTag } {
   global gMergeMap gMergeParts 
   
   set NV [Graph::NullVertex] ;# cache NullVertex value
   set dmOut [$dmGraph GetOutAdjacencies $dmTag]
   foreach v [$pGraph GetOutAdjacencies $pTag] {
      # Have we been here before? Let's avoid cycles...
      # Is this the coorect thing to do?
      if { [info exists gMergeMap($v)] } {
         set dv $gMergeMap($v)
         if { $dv != $NV } {
            $dmGraph AddEdge $dmTag $dv ;# just add an edge
            continue
         }
      }
      set name [$pGraph Get $v name]
#      puts "M $name"
      set dv [$dmGraph FindVertex name $name $dmOut]
      if { $dv != $NV } { ;# DM already has this node...

         set gMergeMap($v) $dv ;# update merge map

         set pt [$pGraph Get $v type]
         set dmt [$dmGraph Get $dv type]
         if { $pt != $dmt } {
            if { $dmt == "unknown" } {
               # here we know that $pt != "unknown" so it's a better choice
               # for the type, set it.
               $dmGraph Set $dv type $pt
            } elseif { $pt == "unknown" } {
               # here we know that $dt != "unknown" so it's a better choice
               # for the type, keep it.
            } else {
               # neither type is unknown, but they're in conflict,
               # we'll just log it for now since it's hard to know
               # which is the better guess for the type.
               LogWarning "Non-matching data types! $pt != $dmt, $name"
            }
         }

         $dmGraph Set $dv special [lunion [$dmGraph Get $dv special] \
                                        [$pGraph Get $v special]]
         
         $dmGraph Set $dv value [lunion [$dmGraph Get $dv value] \
                                        [$pGraph Get $v value]]

         $dmGraph Set $dv side [Production::MergeSideValues \
                                 [$dmGraph Get $dv side] \
                                 [$pGraph Get $v side]]

         if { [DmGenCfg::Get SaveSourceProds] } {
            $dmGraph Set $dv prods [lunion [$dmGraph Get $dv prods] $pName]
         }

         set l [$dmGraph Get $dv link]
         # If there is a known link, add it.
         if { [info exists gMergeParts($v)] } {
            # WARN: This is a little dangerous...it relies on the fact that
            # lunion preserves some order, i.e. that the link type (O or S)
            # will still be the first element after the union is done.
            # bad bad bad.
            $dmGraph Set $dv link [lunion $l $gMergeParts($v)]
         }
         # Recursively merge this branch of the graph.
         # This isn't an else because in some productions we may have been able
         # to determine a link, but in others we may not have. Think of a
         # production that puts attributes on the operator without testing the
         # name of the operator.
         mergeProdIntoDatamap $pName $pGraph $v $dmGraph $dv

      # Edge is missing from DM, or node doesn't exist
      # Make sure that this vertex is in the current partition (don't
      # follow links)
      } elseif { [info exists gMergeMap($v)] } { 
         set dv $gMergeMap($v)
         if { $dv != $NV } { ;# Backedge, we've already seen $v and $dv before.
            $dmGraph AddEdge $dmTag $dv ;# just add a vertex
         } else {
            # Create a new vertex in the datamap
            set special [$pGraph Get $v special]
            set type [$pGraph Get $v type]
            set value [$pGraph Get $v value]
            set side [$pGraph Get $v side]
            set dv [$dmGraph InsertVertex [Datamap::DefVertexFields]]
            $dmGraph SetL $dv [list name $name \
                                   type $type \
                                   side $side \
                                   value $value \
                                   special $special \
                                   link $gMergeParts($v)]

            if { [DmGenCfg::Get SaveSourceProds] } {
               $dmGraph Set $dv prods $pName
            }

            $dmGraph AddEdge $dmTag $dv
            set gMergeMap($v) $dv ;# update merge map

            # Are we at the boundary of a partition? If not, keep going
            # by recursively following this graph branch.
            if { $gMergeParts($v) == {} } {
               mergeProdIntoDatamap $pName $pGraph $v $dmGraph $dv
            }
         }
      }
   }
}
