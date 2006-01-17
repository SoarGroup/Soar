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
# dmgen Config file module.

namespace eval DmGenCfg {

   variable defaults { 
      __section__ 0 
"dmgen configuration file.\n\n\
Various configuration parameters can be set with this file.\n\
Parameters are set with this syntax:\n\
\tSet ParameterName ParameterValue\n\
Note that is Set with a CAPITAL S\n\
If ParameterValue has white space, it must be enclosed in quotes.\n\
Comments are started with the # symbol."

      __section__ 0 "Setup parameters."
      
      SoarLibraryPath ""
"Path to Soar library (i.e. /path/to/Soar-x.x/library).\n\
If this is just an empty string, then the SOAR_LIBRARY environment\n\
variable is used.\n\
Example: Set SoarLibraryPath \"c:/soar-8.3/library\""

      __section__ 0 "Datamap generation parameters."

      ProblemSpaceSpec "name"
"Problem-space path specifier. Space delimited list of attributes\n\
that gives the path to the problem-space name.\n\
For TacAirSoar-style systems (^problem-space.name), this would be:\n\
\tSet ProblemSpaceSpec \"problem-space name\"\n\
For Visual Soar-style systems (^name), this would be:\n\
\tSet ProblemSpaceSpec \"name\""

      FillTopPs 1 
"Put anything under a top-state into the top-ps problem space,\n\
even if it doesn't explicitly reference top-ps problem space name."

      FillAnyPs 1  
"If a production does not test a problem space name, put the\n\
attributes it tests into the any-ps problem space."

      CopyTopPs 1  
"If FillTopPs is on, copy top-state attributes to top-ps,\n\
but leave them in the original problem-space as well."

      SaveSourceProds 1  
"Store source production info in the 'prods' attribute of\n\
each node in the datamap. Since these lists can get really\n\
long, turning this off can speed things up a bit."

      ExcludedProds {}
"List of productions that should be excluded from processing.\n\
Removing large productions that simply load data structures, such as\n\
maps, can speed up datamap generation process a lot.\n\
Example: Set ExcludedProds { elaborate*foo propose*bar }"

      __section__ 0 "Output related parameters"

      InputFile source.soar
"Name of file, in current directory, that is sourced to load all\n\
productions that should be processed."

      DatamapName Datamap
"Name given to generated datamap."

      OutputFormat xml
"Output format to write to. Valid values are:\n\
\txml  - Writes the datamap in XML, suitable for SoarDoc\n\
\thtml - Writes a simple set of HTML pages\n\
\tnone - No output\n\
Example: Set OutputFormat xml"

      OutputDirectory xml
"Directory to which output is written, relative to current directory\n\
at time of execution.\n\
Example: Set OutputDirectory xml"

      LogFileName "dmgenlog.txt"
"Name of log file, written to the current directory.\n\
If set to the empty string, then no log is written."

      LogAutoFlush 0
"If 1, then the logis flushed everytime it is written to.\n\
This can slow things down a lot, but may be useful for debugging."
      
      __section__ 0 "HTML output related parameters"

      HtmlHideState 1
" Don't show the root of the 'tree' since it's redundant (state or operator)"

      HtmlShowComments 1 "Write comments"

      HtmlColorPatched 0
" If a vertex has been patched by the user, highlight its name."

      HtmlShowSpecial 0
"Show values of 'special' attributes. Only really useful for debugging."

      HtmlSuppressedAttributes {}
"List of attribute names that will not be written."

      HtmlMaxValuesPerLine 4
"Maximum number of attribute values that will be written inline before
they are moved to the separate values window."

      HtmlMaxSoarListDepth 4
"Maximum depth to which a detectable soarList is displayed."

   }

   # Table of parameter values, indexed by param name
   variable params
   if [array exists params] { unset params }

   variable aliasSet [AliasSet::Create]
   
   ##
   # Read a config file.
   proc Read { fileName } {
      source $fileName
   }

   ##
   # Write a config file to the given file.
   #
   # @param fileName Name of file to write to, or stdout if it is ""
   proc Write { { fileName "" } } {
      if { $fileName != "" } {
         set fd [open $fileName w]
      } else {
         set fd stdout
      }
      variable defaults
      variable params
      foreach { name value comments } $defaults {
         if { $name != "__section__" } {
            puts $fd "##\n# - $name -"
            set c [join [split $comments "\n"] "\n# "]
            puts $fd "# $c"
            set v $params($name)
            puts $fd "Set $name [escapeValue $v]\n"
         } else {
            puts $fd "###############################################################################"
            set c [join [split $comments "\n"] "\n# "]
            puts $fd "# $c\n"
         }
      }

      puts $fd "# Aliases #####################################################################\n\
                # Aliases give dmgen hints about shared structure in memory. For example, if\n\
                # you know that whenever top-state is tested that it is referring to a\n\
                # particular named state, you can create an alias to map the structure under\n\
                # top-state to the correct state. For example:\n\
                #\n\
                #     Alias S:*.top-state --> S:predict\n\
                #\n\
                # will map any structure detected under the top-state attribute of any state\n\
                # to the 'predict' problem-space.\n\
                #\n\
                # The general syntax is:\n\
                #    Alias \[S|O|*\]:\[name|*\].attribute.path --> \[S|O\]:name.attribute.path\n\
                # Note that wildcards are only allowed on the left hand side of the alias.\n"

      variable aliasSet
      foreach a [$aliasSet GetAliases] {
         puts $fd "Alias [$a ToString]\n"
      }
      if { $fd != "stdout" } {
         close $fd
      }
   }

   ##
   # Set the value of a parameter. 
   #
   # @param param Name of parameter
   # @param value Value of parameter
   proc Set { param value } {
      variable params
      set params($param) $value

      # Custom param setting code
      # Todo, make this more data driven!
      if { $param == "ProblemSpaceSpec" } {
         Production::ProblemSpaceSpec $value
      } elseif { $param == "LogFileName" } {
         LogSetFileName $value
      } elseif { $param == "LogAutoFlush" } {
         LogSetAutoFlush $value
      }

   }

   ##
   # Get the value of a parameter
   #
   # @param Name of parameter
   proc Get { param } {
      variable params
      return $params($param)
   }

   ##
   # Create an alias
   proc Alias { args } {
      variable aliasSet
      set a [Alias::Create [join $args " "]]
      $aliasSet AddAlias $a
      return $a
   }

   proc GetAliasSet { } {
      variable aliasSet
      return $aliasSet
   }

   proc escapeValue { v } {
      set v [join [split $v "\\"] "\\\\"]
      set v [join [split $v "\""] "\\\""]
      if { $v == "" || [regexp "\[ \r\n\t\]" $v] } {
         set v "\"$v\""
      }
      return $v
   }

   #
   # Load the default values into the parameter value table.
   # do this last so we know that Set proc is loaded and callable.
   foreach { name val comments } $defaults {
      if { $name != "__section__" } {
         Set $name $val
      }
   }

} ;# namespace eval DmGenCfg
