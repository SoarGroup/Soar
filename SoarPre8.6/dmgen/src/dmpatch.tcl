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
# Code to support "patching" of generated datamaps.
# A patch file is a tcl file with a series of patch commands that modify
# datamap attributes and mark them as patched. The code in this file
# implements support for reading and writing patch files.
#
# The following types of patches are currently supported:
#     Modifying values of datamap vertex attributes.
#        'attributes' include name, type, value, link, prods, etc.
#
# The following types of patches would be nice, but are tougher:
#     Add new vertex to datamap (pretty easy)
#     Remove vertex from datamap (harder)
#     Add new edge to datamap (not that hard) 
#
# Requires:
#  dijkstra.tcl
#  datamap.tcl
#  graph.tcl

namespace eval DmPatch {

   ##
   # Set the value of a vertex field and update the vertex patch
   # set appropriately.
   #
   # @param h PS or OP handle
   # @param path .-seperated path from start vertex to vertex to modify
   #              e.g. state.operator.name
   # @param field Name of field to patch
   # @param value New field value
   # @return 1 on success, 0 on failure
   proc PatchVertex { h path field value } {
      set g [Datamap::GetGraph $h]
      set s [Datamap::GetStartVertex $h]
      set path [split $path "."]
      set v [Datamap::FindVertexFromPath $g $s $path]
      if { $v == [Graph::NullVertex] } {
         LogError "Vertex $path could not be found in datamap graph!"
         return 0
      }
      
      PatchVertexFast $g $v $field $value
      return 1
   }

   proc PatchVertexFast { g v field value } {
      $g Set $v $field $value

      set patched [$g Get $v patched]
      set patched [lunion $patched $field]
      $g Set $v patched $patched
   }

   proc RemoveVertex { dmStruct v } {

   }

   ##
   # Write a patch file for a datamap to a file.
   #
   # @param dm Datamap to write patch file for
   # @param fn Name of file.
   # @returns 1 on success, 0 on failure.
   proc SavePatches { dm fn } {
      set fd [open $fn w]
      set pss [lsort [Datamap::GetProblemSpaces $dm]]
      set ops [lsort [Datamap::GetOperators $dm]]
      foreach h [concat $pss $ops] {
         writePatch $fd $h
      }
      close $fd
   }
   ##
   # Write patches for a particular operator or problem space
   # 
   # @param fd File descriptor to write to.
   # @param h Handle to PS or OP.
   proc writePatch { fd h } {
      set g [Datamap::GetGraph $h]
      set start [Datamap::GetStartVertex $h]
      set type [Datamap::GetType $h]
      set name [Datamap::GetPsOrOpName $h]
      array set pi [Dijkstra::ShortestPaths $g Dijkstra::UnitWeight $start]
      foreach v [$g GetVertices] {
         set patches [$g Get $v patched]
         set path [join [GetVertexPath $g $start $v pi] "." ]
         foreach field $patches {
            set value [$g Get $v $field]
            puts $fd "DmPatch::PatchVertex \"\$PDM,$type,$name\" $path $field \{$value\}"
         }
      }
   }

   ##
   # Load a patch file and apply it to a datamap.
   #
   # @param dm Datamap that patches will be applied to.
   # @param fn Name of file.
   proc ApplyPatchFile { dm fn } {
      set PDM $dm ;# The patchfile is expecting this variable..
      source $fn
   }

   ##
   # Get the name path to a vertex.
   #
   # @param g Graph
   # @param s Start vertex of path
   # @param v Vertex we want the path to
   # @param rpi Name of an array initialized from running Dijkstra
   #            ShortestPaths on $g $s.
   # @returns A list of vertex names, e.g. { state operator name }
   proc GetVertexPath { g s v rpi } {
      upvar $rpi pi
      set vertices [Dijkstra::GetVertexPath pi $v] 
      return [$g Get $vertices name]
   }
} ;# namespace DmPatch
