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
# Main datamap window.
# Just has menus for saving patches and HTML export and for displaying
# problem-space and operators
#
# Requires:
#     bwutil.tcl
#     htmlout.tcl
#     datamap.tcl
#     patchwin.tcl
#     dmpatch.tcl

package require BWidget

namespace eval DmWin {
   # Get the name of the window for the given datamap
   proc Get { dm } { return .$dm }

   # Create a window for the given datamap unless one already exists, in
   # which case the existing window is simply raised.
   proc Create { dm } {
      # If there's already a window for this handle, raise it and return
      set w .$dm
      if [winfo exists $w] {
         raise $w
         return $w
      }
      set w [toplevel $w]

      bind $w <Destroy> [list DmWin::onDestroy {%W} $w $dm]

      wm title $w "Datamap: [Datamap::GetName $dm]"

      # Create a menu bar
      set mb [menu $w.mb]
      $w config -menu $mb
      foreach m { File ProblemSpaces Operators } {
         set mnu$m [menu $mb.mnu$m]
         $mb add cascade -label $m -menu $mb.mnu$m
      }

      $mnuFile add command -label "Apply patch file" \
                           -command "DmWin::onApplyPatch $w $dm"
      $mnuFile add command -label "Save patch file" \
                           -command "DmWin::onSavePatch $w $dm"
      $mnuFile add command -label "Export to HTML" \
                           -command "DmWin::onExportToHtml $w $dm"
      $mnuFile add command -label "Export to XML" \
                           -command "DmWin::onExportToXml $w $dm"

      $mnuProblemSpaces config -postcommand "DmWin::psMenuPost $w $dm" 
      $mnuOperators config -postcommand "DmWin::opMenuPost $w $dm" 

      set f [LabelFrame $w.htmlFrame \
                        -text "HTML Export Options" \
                        -side top -relief ridge]
      foreach { v l } { HtmlHideState "Hide State/Operator" \
                        HtmlShowComments "Show Comments" \
                        HtmlColorPatched "Highlight Patched Attributes" \
                        HtmlShowSpecial "Show special tags" } {

         set cb [checkbutton $f.[string tolower $v] \
                             -text $l \
                             -variable DmGenCfg::params($v) \
                             -anchor w]
         pack $cb -side top
      }
      pack [label $f.suppressedAttrLbl -text "Suppressed Attributes:"] -side top
      pack [entry $f.suppressedAttr \
                  -textvariable DmGenCfg::params(SuppressedAttributes)] -side top

      pack $f -side left

      return $w
   }
   proc onDestroy { widget w dm } {
      if { [string compare $widget $w] != 0 } { return }

      # Clean up all problem-space and operator windows we've 
      # spawned.
      set all [concat [Datamap::GetProblemSpaces $dm] \
                      [Datamap::GetOperators $dm]]
      foreach h $all {
         if [winfo exists [PatchWin::Get $h]] {
            destroy [PatchWin::Get $h]
         }
      }
   }
   proc onApplyPatch { w dm } {
      set tl {
         { "Datamap Patch File" { ".dmp" } }
         { "All Files" {*} }
      }
      set f [tk_getOpenFile -title "Apply Datamap Patch File..." \
                            -parent $w \
                            -filetypes $tl]
      if { $f != "" } { ;# user pressed open
         DmPatch::ApplyPatchFile $dm $f
      }

   }
   proc onSavePatch { w dm } {
      set tl {
         { "Datamap Patch File" { ".dmp" } }
         { "All Files" {*} }
      }
      set f [tk_getSaveFile -title "Apply Datamap Patch File..." \
                            -parent $w \
                            -initialfile "[Datamap::GetName $dm].dmp" \
                            -filetypes $tl]
      if { $f != "" } { ;# user pressed open
         DmPatch::SavePatches $dm $f
      }
   }

   proc onExportToHtml { w dm } {
      set tl {
         { "HTML Files" { ".html" } }
         { "All Files" {*} }
      }
      # tk_chooseDirectory isn't available until Tk8.3 :(
      set msg "You will now be asked to choose a directory for HTML export.
 Tk8.0 doesn't have a choose directory command, so please choose a file and
 the HTML export will output to a directory called html located in the same
 directory as the file you choose."
      tk_messageBox -message $msg
      set f [tk_getSaveFile -title "Export Datamap To HTML..." \
                            -parent $w \
                            -initialfile "[Datamap::GetName $dm].html" \
                            -filetypes $tl]
      if { $f != "" } { ;# user pressed open
         set dirName [file join [file dirname $f] html]
         file mkdir $dirName
         DmGenHtmlOut::WriteHtmlDatamap $dirName $dm
      }
   }

   proc onExportToXml { w dm } {
      set tl {
         { "XML Files" { ".xml" } }
         { "All Files" {*} }
      }
      # tk_chooseDirectory isn't available until Tk8.3 :(
      set msg "You will now be asked to choose a directory for XML export.
 Tk8.0 doesn't have a choose directory command, so please choose a file and
 the XML export will output to a directory called XML located in the same
 directory as the file you choose."
      tk_messageBox -message $msg
      set f [tk_getSaveFile -title "Export Datamap To XML..." \
                            -parent $w \
                            -initialfile "[Datamap::GetName $dm].xml" \
                            -filetypes $tl]
      if { $f != "" } { ;# user pressed open
         set dirName [file join [file dirname $f] xml]
         file mkdir $dirName
         DmGenXmlOut::WriteXmlDatamap $dirName $dm
      }
   }

   proc psMenuPost { w dm } {
      set mnu $w.mb.mnuProblemSpaces
      $mnu delete 0 end
      foreach ps [lsort [Datamap::GetProblemSpaces $dm]] {
         set n [Datamap::GetPsOrOpName $ps]
         $mnu add command -label $n -command "PatchWin::Create $ps"
      }
   }
   proc opMenuPost { w dm } {
      set mnu $w.mb.mnuOperators
      $mnu delete 0 end
      foreach op [lsort [Datamap::GetOperators $dm]] {
         set n [Datamap::GetPsOrOpName $op]
         $mnu add command -label $n -command "PatchWin::Create $op"
      }
   }
}
