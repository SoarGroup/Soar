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
# Code to write a datamap nicely formatted in HTML
#
# Requires:
#   pushd, popd
#   datamap.tcl
#   util.tcl
#   graph.tcl

namespace eval DmGenHtmlOut {

# a simple hack to write out a string in HTML with proper escapes
proc EscapeHtmlString { s } {
   set s [join [split $s "<"] "&lt;"]
   set s [join [split $s ">"] "&gt;"]

   return $s
}
proc startHtmlDoc { fd { title "" } } {
   puts $fd "<html>"
   puts $fd "   <head>"
   puts $fd "      <title>$title</title>"
   puts $fd "   </head>"
   puts $fd "   <body>"
}
proc endHtmlDoc { fd } {
   puts $fd "   </body>"
   puts $fd "</html>"
}
proc startFrameHtmlDoc { fd { title "" } } {
   puts $fd "<html>"
   puts $fd "   <head>"
   puts $fd "      <title>$title</title>"
   puts $fd "   </head>"
}
proc endFrameHtmlDoc { fd } {
   puts $fd "</html>"
}

##
# Write an HTML datamap to directory fn
proc WriteHtmlDatamap { fn dm } {
   pushd $fn
   
   writeHtmlIndex "Datamap: [Datamap::GetName $dm]"
   writeHtmlToc $dm
   
   set pss [lsort [Datamap::GetProblemSpaces $dm]]
   set ops [lsort [Datamap::GetOperators $dm]]
   foreach h [concat $pss $ops] {
      writeHtmlPsOrOp $h
   }

   popd ;# pop back to original directory
}

proc writeHtmlIndex { title } {
   set fd [open "index.html" w]

   startFrameHtmlDoc $fd $title
   puts $fd "<FRAMESET COLS=\"25%,75%\">      \n   \
               <FRAME SRC=\"toc.html\" NAME=\"toc\">       \n   \
               <FRAMESET ROWS=\"75%,25%\">      \n \
                  <FRAME SRC=\"about:blank\" NAME=\"main\">   \n \
                  <FRAMESET COLS=\"50%,50%\">      \n \
                     <FRAME SRC=\"about:blank\" NAME=\"vals\">   \n \
                     <FRAME SRC=\"about:blank\" NAME=\"prods\">   \n \
                  </FRAMESET>                      \n \
               </FRAMESET>                      \n \
            </FRAMESET>" 

   endFrameHtmlDoc $fd

   close $fd
}
proc writeHtmlToc { dm } {
   set fd [open "toc.html" w]

   startHtmlDoc $fd "Table of Contents"

   puts $fd "<a name=\"toc\">"
   puts $fd "<p><a href=\"#ps\">Problem Spaces</a>"
   puts $fd "<p><a href=\"#ops\">Operators</a>"

   set pss [lsort [Datamap::GetProblemSpaces $dm]]
   puts $fd "<a name=\"ps\">"
   puts $fd "<p><h3><b>Problem Spaces</b></h3>"
   foreach h $pss {
      writeHtmlTocEntry $fd $h
   }

   puts $fd "<a name=\"ops\">"
   set ops [lsort [Datamap::GetOperators $dm]]
   puts $fd "<p><h3><b>Operators</b></h3>"
   foreach h $ops {
      writeHtmlTocEntry $fd $h
   }

   endHtmlDoc $fd
   close $fd
}
proc writeHtmlTocEntry { fd h } {
   set name [Datamap::GetPsOrOpName $h]
   set type [Datamap::GetType $h]
   puts $fd "<a href=\"$type-$name.html\" TARGET=\"main\">$name</a><br>"
}

proc writeHtmlPsOrOp { h } {
   global HtmlVisited
   if [info exists HtmlVisited] {
      unset HtmlVisited
   }
   set graph [Datamap::GetGraph $h]
   set name [Datamap::GetPsOrOpName $h]
   if [Datamap::IsProblemSpace $h] {
      set typeName "Problem Space"
      set type S
   } else {
      set typeName "Operator"
      set type O
   }

   set fd [open "$type-$name.html" w]
   startValsFile $h
   startProdsFile $h
   startHtmlDoc $fd "$typeName: $name"

   foreach v [$graph GetVertices] {
      set HtmlVisited($v) 0
   }
   puts $fd "<p><h2>$typeName: $name</h2>"
   puts $fd "<ul>"
   writeHtmlDatamap_r $fd $graph [Datamap::GetStartVertex $h] 1
   puts $fd "</ul>"

   endHtmlDoc $fd
   endValsFile
   endProdsFile

   close $fd
}
proc startValsFile { h } {
   global ValsPsOrOp
   set ValsPsOrOp $h
}
proc reallyStartValsFile { } {
   global ValsPsOrOp ValsFd ValsFileName

   set h $ValsPsOrOp

   set name [Datamap::GetPsOrOpName $h]
   if [Datamap::IsProblemSpace $h] {
      set typeName "Problem Space"
      set type S
   } else {
      set typeName "Operator"
      set type O
   }
   set ValsFileName "$type-$name-vals.html"
   set ValsFd [open $ValsFileName w]
   startHtmlDoc $ValsFd "$typeName $name - Values"

   puts $ValsFd "<p><h2>Over-run values for $typeName $name</h2></p>"
}

proc appendToValsFile { attr vals } {
   global ValsFd ValsFileName
   if ![info exists ValsFd] {
      reallyStartValsFile
   }
   set tag [GetTag valTag]
   set fd $ValsFd


   puts $fd "<a name=\"$tag\">"
   puts $fd "<p><b>Values for attribute '$attr'</b></p>"
   puts $fd "<p>"
   foreach v $vals {
      puts $fd "   [EscapeHtmlString $v]<br>"
   }
   puts $fd "</p>"
   puts $fd "<br><br><br><br><br><br>"
   
   return "$ValsFileName#$tag"
}

proc endValsFile { } {
   global ValsFd ValsPsOrOp
   if [info exists ValsFd] {
      close $ValsFd
      unset ValsFd
      unset ValsPsOrOp
   }
}

proc startProdsFile { h } {
   global ProdsPsOrOp ProdsFd ProdsFileName
   set ProdsPsOrOp $h

   set name [Datamap::GetPsOrOpName $h]
   if [Datamap::IsProblemSpace $h] {
      set typeName "Problem Space"
      set type S
   } else {
      set typeName "Operator"
      set type O
   }
   set ProdsFileName "$type-$name-prods.html"
   set ProdsFd [open $ProdsFileName w]
   startHtmlDoc $ProdsFd "$typeName $name - Productions"

   puts $ProdsFd "<p><h2>Source productions for $typeName $name</h2></p>"
}

proc appendToProdsFile { attr prods } {
   global ProdsFd ProdsFileName

   set tag [GetTag prodTag]
   set fd $ProdsFd

   puts $fd "<a name=\"$tag\">"
   puts $fd "<p><b>Source productions for attribute '$attr'</b></p>"
   puts $fd "<p>"
   foreach p [lsort $prods] {
      puts $fd "   [EscapeHtmlString $p]<br>"
   }
   puts $fd "</p>"
   puts $fd "<br><br><br><br><br><br>"
   
   return "$ProdsFileName#$tag"
}

proc endProdsFile { } {
   global ProdsFd ProdsPsOrOp
   if [info exists ProdsFd] {
      close $ProdsFd
      unset ProdsFd
      unset ProdsPsOrOp
   }
}

##
# Is this vertex the start of a soarList, and if so, how deep does it go?
# Returns depth of (next,item) pairs, or 0 if none.
proc getSoarListDepth { g v } {
   set NV [Graph::NullVertex]
   set out [$g GetOutAdjacencies $v]
   if { [llength $out] < 2 } { return 0 }
   set vitem [$g FindVertex name item $out]
   if { $vitem == $NV } { return 0 }
   set vnext [$g FindVertex name next $out]
   if { $vnext == $NV } { return 0 }

   return [expr [getSoarListDepth $g $vnext] + 1]
}

proc writeHtmlDatamap_r { fd graph vert { top 0 } } {
   global HtmlVisited 
   set HtmlVisited($vert) 1

   set maxValues [DmGenCfg::Get HtmlMaxValuesPerLine]

   set show [expr !$top || ($top && ![DmGenCfg::Get HtmlHideState])]

   set out [$graph GetOutAdjacencies $vert]

   if { $show } {
      puts $fd "<li>"
      array set props [$graph Get $vert]
      if { [DmGenCfg::Get HtmlColorPatched] && \
           [llength $props(patched)] > 0 } {
         puts $fd "<font COLOR=\"#0000FF\">"
         puts $fd "<b>[EscapeHtmlString $props(name)]</b>,"
         puts $fd "</font>"
      } else {
         puts $fd "<b>[EscapeHtmlString $props(name)]</b>,"
      }
      puts $fd "Type: <i>$props(type)</i>"
      set nVals [llength $props(value)]
      if { $nVals > 0 && $nVals <= $maxValues } {
         set valString [EscapeHtmlString [join $props(value) " ,  "]]
         puts $fd ", Value(s): $valString"
      } elseif { $nVals > $maxValues } {
         set tag [appendToValsFile $props(name) $props(value)]
         puts $fd ", <a href=\"$tag\" TARGET=\"vals\">\[Values\]</a>"
      }

      # write out which productions it came from...
      set tag [appendToProdsFile $props(name) $props(prods)]
      puts $fd ", <a href=\"$tag\" TARGET=\"prods\">\[Productions\]</a>"

      if { [DmGenCfg::Get HtmlShowSpecial] } {
         puts $fd "<br>Special: $props(special)"
      }
      if { $props(link) != {} } {
         puts $fd "<br>Links: "
         set txt {}
         foreach { type root path } [join $props(link) " "] {
            set tgt $root
            if { [llength $path] > 0 } {
               set tgt "$tgt.[join $path .]"
            }
            lappend txt "<a href=\"$type-$root.html\">[EscapeHtmlString $tgt]</a>"
         }
         puts $fd [join $txt ", "]
      }
      if { [DmGenCfg::Get HtmlShowComments] && [llength $props(comment)] > 0 } {
         puts $fd "<br>Comment: <i>"
         foreach l $props(comment) {
            puts $fd "$l"
            puts $fd "<br>"
         }
         puts $fd "</i>"
      }

      puts $fd "<ul>"
   }

   foreach v $out {
      set name [$graph Get $v name]
      if { !$HtmlVisited($v) } { ;# we haven't hit this vertex yet
         if { [lsearch -exact [DmGenCfg::Get HtmlSuppressedAttributes] $name] == -1 } {
            set depth 0
            if { $name == "next" } {
               set depth [getSoarListDepth $graph $v]
            }
            if { $depth > [DmGenCfg::Get HtmlMaxSoarListDepth] } {
               puts $fd "<i><b> ... Suppressed soarList with depth = $depth ... </b></i>"
            } else {
               writeHtmlDatamap_r $fd $graph $v
            }
         }
      } else {
         puts $fd "<br><i>...Backedge to attribute $name...</i>"
      }
   }
   if { $show } {
      puts $fd "</ul>"
   }
}
} ;# namespace DmgenHtmlOut
