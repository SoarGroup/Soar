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
# Implementation of the patch window which displays a problem-space or
# operator and allow it to be edited by the user. This window is spawned
# when the user chooses a ps or op from the datamap window.
#
# Requires:
#     bwutil.tcl
#     datamap.tcl
#     soarutil.tcl

package require BWidget

namespace eval PatchWin {
   # Returns the widget path for a patch window given a ps or op handle.
   # This will return a value, even if the window doesn't exist!
   proc Get { h } { return .$h }

   # Create a patch window for the given ps or op handle, or raise it if
   # one already exists.
   #
   # @param h Handle to PS or OP, returned from Datamap:: functions.
   # @returns Widget path of window.
   proc Create { h } {
      set w [Get $h]

      # If there's already a window for this handle, raise it and return
      if [winfo exists $w] {
         raise $w
         return $w
      }
      set dm [Datamap::GetDatamap $h]
      set w [toplevel $w] ;# create toplevel window
      #wm geometry $w 500x400

      # set up a window close handler
      bind $w <Destroy> [list PatchWin::onDestroy {%W} $w $h]

      # set the window title
      set dmName [Datamap::GetName $dm]
      if [Datamap::IsProblemSpace $h] {
         wm title $w "Datamap: $dmName, Problem Space: [Datamap::GetPsOrOpName $h]"
      } else {
         wm title $w "Datamap: $dmName, Operator: [Datamap::GetPsOrOpName $h]"
      }

      # Add a menubar
      set mb [menu $w.mb]
      $w config -menu $mb
      $mb add command -label "Datamap" -command "raise [DmWin::Get $dm]"
      foreach m { Edit ProblemSpaces Operators } {
         set mnu$m [menu $mb.mnu$m]
         $mb add cascade -label $m -menu $mb.mnu$m
      }

      # Set up post callbacks for PS and OP menus
      $mnuProblemSpaces config -postcommand "PatchWin::psMenuPost $w $h" 
      $mnuOperators config -postcommand "PatchWin::opMenuPost $w $h" 

      # Create a tree on the left of the window
      ScrolledWindow $w.sw
      set g [Datamap::GetGraph $h]
      set s [Datamap::GetStartVertex $h]
      set tree [BwUtil::NewBWidgetTreeFromGraph $w.tree $g $s name]
      $tree bindText <ButtonPress-1> "PatchWin::onTreeLeftClick $w $h"
      $tree bindText <ButtonPress-3> "PatchWin::onTreeRightClick $w $h %X %Y"
      $w.sw setwidget $tree
      #ExpandBWidgetTree $tree

      # Create a popup menu for right-clicking tree nodes
      set pu [menu $tree.popup -tearoff 0]
      menu $pu.prods


      # Create a frame on the right with widgets for editing the selected
      # tree node.
      set f [frame $w.f]
      
      
      grid [label $f.nameLbl -text "Name:"] -column 0 -row 1 -sticky e
      grid [entry $f.name] -column 1 -row 1 -sticky ew

      grid [label $f.valueLbl -text "Value(s):"] -column 0 -row 2 -sticky e
      grid [entry $f.value] -column 1 -row 2 -stick ew

      grid [label $f.typeLbl -text "Type:"] -column 0 -row 3 -sticky e
      set cb [ComboBox $f.type -values [Production::NodeTypes] \
                             -editable false]
      $cb setvalue @0
      grid $cb -column 1 -row 3 -sticky ew


      grid [label $f.commentLbl -text "Comment:"] -column 0 -row 4 -sticky e
      grid [text $f.comment] -column 1 -row 4 

      set cb [checkbutton $f.showUnknownTypes \
                          -text "Mark (?) nodes with type unknown" \
                          -command "PatchWin::onUpdateAllNodes $w $h" \
                          -variable $f.showUnknownTypes]
      grid $cb -column 1 -row 5
      set cb [checkbutton $f.showEmptyValues \
                          -text "Mark (@) nodes with empty value" \
                          -command "PatchWin::onUpdateAllNodes $w $h" \
                          -variable $f.showEmptyValues]
      grid $cb -column 1 -row 6

      foreach v [$g GetVertices] {
         updateNodeDisplay $w $h $g $v
      }

      pack $w.sw $tree -expand yes -fill both -side left

      pack $f -side right -expand yes -fill both

      return $w
   }
   proc psMenuPost { w h } {
      set mnu $w.mb.mnuProblemSpaces
      $mnu delete 0 end

      # Fill the PS menu with all PSs, except for this one if it's a PS.
      if [Datamap::IsProblemSpace $h] {
         set name [Datamap::GetPsOrOpName $h]
      } else { 
         set name "" ;# Just so it doesn't match below
      }
      set dm [Datamap::GetDatamap $h]
      foreach ps [lsort [Datamap::GetProblemSpaces $dm]] {
         set n [Datamap::GetPsOrOpName $ps]
         if { [string compare $n $name] != 0 } {
            $mnu add command -label $n \
                     -command "PatchWin::Create $ps"
         }
      }
   }
   proc opMenuPost { w h } {
      set mnu $w.mb.mnuOperators
      $mnu delete 0 end

      # Fill the OP menu with all OPs, except for this one if it's a OP.
      if [Datamap::IsOperator $h] {
         set name [Datamap::GetPsOrOpName $h]
      } else { 
         set name "" ;# Just so it won't match below
      }
      set dm [Datamap::GetDatamap $h]
      foreach op [lsort [Datamap::GetOperators $dm]] {
         set n [Datamap::GetPsOrOpName $op]
         if { [string compare $n $name] != 0 } {
            $mnu add command -label $n \
                     -command "PatchWin::Create $op"
         }
      }
   }
   proc onDestroy { widget w h } {
      if { [string compare $widget $w] != 0 } {
         return
      }
      ;# nothing to do here yet...
   }

   # If 'new' value of vertex field is different from the current one,
   # patch the vertex with the new value.
   proc patchOnChange { g v field new } {
      set old [$g Get $v $field]
      if { [string compare $new $old] != 0 } {
         DmPatch::PatchVertexFast $g $v $field $new
         return 1
      }
      return 0
   }
   proc updateNodeDisplay { w h graph node } {
      set tree $w.tree
      set name [$graph Get $node name]
      array set data [$tree itemcget $node -data]
      if { [llength [$graph Get $node link]] > 0 || \
           [llength $data(BackEdges)] > 0 } {
         set name "$name -->"
      }
      # check for unknown type
      set type [$graph Get $node type]
      global $w.f.showUnknownTypes
      if { [set $w.f.showUnknownTypes] && \
           [string compare $type "unknown"] == 0 } {
         set name "? $name"
      }
      # check for empty value
      global $w.f.showEmptyValues
      if { [set $w.f.showEmptyValues] && \
           [string compare $type "identifier"] != 0 && \
           [llength [$graph Get $node value]] == 0 } {
         set name "@ $name"
      }
      $tree itemconfigure $node -text $name
      if { [llength [$graph Get $node patched]] > 0 } {
         $tree itemconfigure $node -fill blue
      }
   }
   proc onTreeLeftClick { w h node } {
      set f $w.f
      set g [Datamap::GetGraph $h]

      set sel [$w.tree selection get]
      if { $sel != "" } {
         set c [expr [patchOnChange $g $sel name [$f.name get]] || \
                     [patchOnChange $g $sel value [$f.value get]] || \
                     [patchOnChange $g $sel type [BwUtil::GetComboBoxText $f.type]] || \
                     [patchOnChange $g $sel comment [split [BwUtil::GetTextBoxText $f.comment] "\n"]]]
         if $c {
            updateNodeDisplay $w $h $g $sel
         }
      }
      
      $w.tree selection set $node
      
      BwUtil::SetEntryText $f.name [$g Get $node name]
      BwUtil::SetEntryText $f.value [$g Get $node value]
      BwUtil::SetComboBoxValue $f.type [$g Get $node type]
      BwUtil::SetTextBoxText $f.comment [join [$g Get $node comment] "\n"]

      foreach p { name value type comment } {
         $f.[join [list $p Lbl] "" ] configure -foreground black
      }
      foreach p [$g Get $node patched] {
         $f.[join [list $p Lbl] "" ] configure -foreground blue
      }
   }
   proc onTreeRightClick { w h x y node } {
      onTreeLeftClick $w $h $node

      set pu $w.tree.popup
      $pu delete 0 end

      set g [Datamap::GetGraph $h]

      # If source productions are available, show a menu with a list of them
      set prods [$g Get $node prods]
      set hasProds 0
      if { [llength $prods] > 0 } {
         set hasProds 1
         $pu add cascade -label "Productions" -menu $pu.prods
         $pu.prods config \
               -postcommand "PatchWin::onSourceProdsPost $pu.prods $h $node"
      }

      set link [$g Get $node link]

      # if there are any links, add menu items to follow them
      set hasLinks 0 
      if { [llength $link] > 0 } {
         if { $hasProds } {
            $pu add separator ;# separate this from source prods
         }
         set hasLinks 1
         set dm [Datamap::GetDatamap $h]
         foreach { type root path } [join $link " "] {
            if { $type == "S" } {
               set typeName "Problem Space"
            } else {
               set typeName "Operator"
            }
            set nh [Datamap::GetPsOrOp $dm $type $root]
            $pu add command -label "Go to $typeName: $root" \
                           -command "PatchWin::Create $nh"
         }
      }
      # If there are any backedges, we add menu items to go to those
      # vertices from here.
      array set data [$w.tree itemcget $node -data]
      if { [llength $data(BackEdges)] > 0 } {
         if { $hasLinks } {
            $pu add seperator ;# separate this from the links
         }
         set hasLinks 1
         set s [Datamap::GetStartVertex $h]
         array set pi [Dijkstra::ShortestPaths $g Dijkstra::UnitWeight $s]
         foreach v $data(BackEdges) {
            set path [join [DmPatch::GetVertexPath $g $s $v pi] "." ]
            $pu add command -label "Backedge: $path" \
                            -command "PatchWin::onTreeLeftClick $w $h $v"
         }
      }

      if { $hasProds || $hasLinks } {
         tk_popup $w.tree.popup $x $y
      }
   }
   # Command called when the source productions submenu of a right-click is
   # posted. Just fills in the list of productions and adds a command that
   # prints the production to the console.
   proc onSourceProdsPost { mnu h node } {
      $mnu delete 0 end
      set g [Datamap::GetGraph $h]
      foreach p [$g Get $node prods] {
         set cmd "SoarUtil::GetSoarPrint $p"
         $mnu add command -label $p \
            -command "puts \"\[$cmd\]\""
      }
   }

   proc onUpdateAllNodes { w h } {
      set g [Datamap::GetGraph $h]
      foreach v [$g GetVertices] {
         updateNodeDisplay $w $h $g $v
      }
   }
}
