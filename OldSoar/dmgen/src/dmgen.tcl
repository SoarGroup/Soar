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
# Main dmgen program
global dmGenVersion
set dmGenVersion "1.0"

proc DmGenGetVersion {} {
   global dmGenVersion
   return $dmGenVersion
}

##
# Generate a datamap and bring up the datamap window
# This function is for use when dmgen has been manually loaded
# into a Tcl interpretter.
#
# @param name Name of the datamap
# @param prods List of productions to process or {} for all loaded.
# @param compiled Strictly for debugging. Ignore.
# @returns The generated datamap
proc GenerateDatamap { { name "" } { prods {} } { compiled 1 } } {
   global tk_version DmGenCompiled

   set DmGenCompiled $compiled

   set dm [Datamap::Create $name]
   Datamap::Generate $dm $prods
   if [info exists tk_version] {
      DmWin::Create $dm
   }
   return $dm
}


###
## Everything from here down is for running from the command line.
proc dmGenUsage {} {
   puts stderr "Usage: tclsh path/to/dmgen.tcl \[options\]"
   puts stderr "   -g : Prints a default config file to stdout."
   puts stderr "   -f configFile : Specify a config file. Defaults to \"dmgenfile\"."
   puts stderr "   -h : Print this message and exit."
   puts stderr "   -C param value : Set configuration parameter, overriding config file."
   puts stderr "   -v : Print dmgen version."
}

proc dmGenPrintVersion { } {
   global dmGenVersion
   puts "Version $dmGenVersion"
}

if [info exists dmGenArgs] {
   unset dmGenArgs
}

set dmGenArgs(GenerateConfig) 0
set dmGenArgs(ConfigParams) {}
set dmGenArgs(Args) {}

proc dmGenParseArgs { } {
   global dmGenArgs argc argv

   for { set i 0 } { $i < $argc } { incr i } {
      set arg [lindex $argv $i]
      if { [string index $arg 0] == "-" } {
         set opt [string range $arg 1 end]
         if { $opt == "C" } {
            if { [expr $argc - $i] < 3 } { 
               puts stderr "Too few arguments to option -C"
               return 0
            }
            set param [lindex $argv [expr $i + 1]]
            set val [lindex $argv [expr $i + 2]]
            lappend dmGenArgs(ConfigParams) $param
            lappend dmGenArgs(ConfigParams) $val

            # Skip the args
            incr i 2
         } elseif { $opt == "f" } {
            if { [expr $argc - $i] < 2 } { 
               puts stderr "Too few arguments to option -f"
               return 0 
            }
            set cfgFile [lindex $argv [expr $i + 1]]
            set dmGenArgs(ConfigFile) $cfgFile 

            # Skip the arg
            incr i
         } elseif { $opt == "g" } {
            set dmGenArgs(GenerateConfig) 1
         } elseif { $opt == "h" } {
            return 0
         } elseif { $opt == "v" } {
            dmGenPrintVersion
            exit 0
         } else {
            puts stderr "Unknown option $opt"
            return 0
         }
      } else {
         lappend dmGenArgs(Args) $arg
      }
   }
   return 1
}

proc dmGenGetProdList { } {
   set excluded [DmGenCfg::Get ExcludedProds]

   if { $excluded == {} } {
      return {}
   }
   set loaded [SoarUtil::GetSoarProductions]
   set prods {}
   foreach p $loaded {
      if { [lsearch $excluded $p] == -1 } {
         lappend prods $p
      }
   }
   return $prods
}

proc dmGenMain { } {
   global dmGenArgs DmGenCompiled env tk_version auto_path errorInfo
 
   puts stderr "dmgen - A Soar static datamap generation tool by Soar Technology, Inc."

   # set up auto_path for dmgen
   # the foreach is just a cheap way of expanding the tuple
   # without having to use lindex and all that.
   foreach { dirName scriptName } [dmGenGetPathToScript] {
      if { $scriptName != "dmgen.tcl" } {
         puts stderr "dmGenMain must be called from top-level"
         exit 1
      }
      if ![file exists $dirName] {
         puts stderr "Could not determine location of dmgen.tcl"
         exit 1
      }
      # Don't really need this any more, see related comment in tclIndex.
      #set DmGenCompiled [file exists [file join $dirName "compiled.txt"]]

      lappend auto_path $dirName
      break
   }

   # parse args
   if ![dmGenParseArgs] {
      dmGenUsage
      exit 1
   }

   if { $dmGenArgs(GenerateConfig) } {
      # Load command-line config overrides, if any
      foreach { param value } $dmGenArgs(ConfigParams) {
         DmGenCfg::Set $param $value
      }
      DmGenCfg::Write
      exit 0
   }

   # Load config file, if any
   if [info exists dmGenArgs(ConfigFile)] {
      set configFile $dmGenArgs(ConfigFile)
   } elseif [file exists dmgenfile] {
      set configFile dmgenfile
   }
   if [info exists configFile] {
      if { [catch { DmGenCfg::Read $configFile } ] } {
         puts stderr "Error while loading config file \"$configFile\":\n$errorInfo" 
         exit 1
      }
   }

   # Load command-line config overrides, if any
   foreach { param value } $dmGenArgs(ConfigParams) {
      DmGenCfg::Set $param $value
   }

   # set up auto_path for Soar
   set soarLibPath [DmGenCfg::Get SoarLibraryPath]
   if { $soarLibPath != "" } {
      # user is overriding SOAR_LIBRARY env. var.
      set env(SOAR_LIBRARY) $soarLibPath
   } elseif [info exists env(SOAR_LIBRARY)] {
      set soarLibPath $env(SOAR_LIBRARY)
   } else {
      LogError "No Soar library path was specified."
      exit 1
   }
   if ![file exists $soarLibPath] {
      LogError "Specified Soar library path \"$soarLibPath\" does not exist."
      exit 1
   }
   lappend auto_path $soarLibPath
   package require Soar

   # load user-specified source.soar
   set inputFile [DmGenCfg::Get InputFile]
   if ![file exists $inputFile] {
      LogError "Could not find input file: $inputFile"
      exit 1
   }
   
   puts "Sourcing input file"
   if { [catch { uplevel #0 source $inputFile }] } {
      LogError "Error while sourcing input file \"$inputFile\":\n$errorInfo" 
      exit 1
   }
   puts "Done Sourcing input file"

   # run dmgen
   set dm [Datamap::Create [DmGenCfg::Get DatamapName]]
   Datamap::Generate $dm [dmGenGetProdList]
   if [info exists tk_version] {
      DmWin::Create $dm
   }
   # write out any desired output...
   set outFmt [DmGenCfg::Get OutputFormat]
   set outDir [DmGenCfg::Get OutputDirectory]
   if { $outFmt == "xml" } {
      file mkdir $outDir
      DmGenXmlOut::WriteXmlDatamap $outDir $dm
   } elseif { $outFmt == "html" } {
      file mkdir $outDir
      DmGenHtmlOut::WriteHtmlDatamap $outDir $dm
   } elseif { $outFmt == "none" } {
      # That was easy.
   } else {
      LogError "Unknown output format: $outFmt"
      exit 1
   }
}

##
# Same as the proc in util.tcl, but we need it here before
# util.tcl may be loaded :(
proc dmGenJoinPathList { pathList } {
   set r ""
   foreach p $pathList {
      set r [file join $r $p]
   }
   return $r
}

##
# Same as the proc in util.tcl, but we need it here before
# util.tcl may be loaded :(
proc dmGenGetPathToScript { } {
   set relPath [info script]
   set scriptName [file tail $relPath]
   set dirName [file dirname $relPath]

   set pathType [file pathtype $dirName]
   if { $pathType == "absolute" } {
      return [list $dirName $scriptName]
   } elseif { $pathType == "relative" } {
      set cur [file split [pwd]]
      set parts [file split $dirName]
      set end [llength $cur]
      for { set i 0 } { $i < [llength $parts] } { incr i } {
         set p [lindex $parts $i]
         if { $p == ".." } {
            incr end -1
         } elseif { $p != "." } {
            break
         }
      }
      set full [concat [lrange $cur 0 [expr $end - 1]] \
                       [lrange $parts $i end]]
      return [list [dmGenJoinPathList $full] $scriptName]
   } else { ;# volumerelative
      return [list $dirName $scriptName]
   }
}
if $tcl_interactive {

} else {
   dmGenMain
}

##
# Don't really need this stuff anymore. If we do need it at some point,
# it goes at the top of the file.
#
#global DmGenAutoLoaded
## Only source dependencies if we're not auto loading...
#if { ($tcl_interactive && ![info exists DmGenAutoLoaded]) } {
#   puts "Not auto-loaded, sourcing files"
#   source "util.tcl"
#   source "log.tcl"
#   source "graph.tcl"
#   source "dijkstra.tcl"
#
#   source "soarutil.tcl"
#   source "lexer.tcl"
#   source "parser.tcl"
#   source "production.tcl"
#   source "partition.tcl"
#   source "merge.tcl"
#   source "datamap.tcl"
#
#   source "htmlout.tcl"
#   source "xmlout.tcl"
#
#   source "dmpatch.tcl"
#   source "config.tcl"
#   
#   if [info exists tk_version] {
#      source "bwutil.tcl"
#      source "dmwin.tcl"
#      source "patchwin.tcl"
#   }
#}
