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
# Represents a single alias
namespace eval Alias {

   ##
   # Create an alias from a config file string mapping.
   #
   # @param mapping The mapping string
   # @returns A new Alias object
   proc Create { mapping } {
      set aliasTag [GetTag Alias]
      variable $aliasTag
      upvar 0 $aliasTag alias

      # Initialize the basic object structure
      set alias(__tag__)      $aliasTag
      set alias(__fromType__) ""    ;# S or O, or *
      set alias(__fromRoot__) ""    ;# name of state or operator, may be *
      set alias(__fromPath__) ""    ;# list of attributes that make up path
      set alias(__toType__)   ""    ;# S or O
      set alias(__toRoot__)   ""    ;# name of state or operator
      set alias(__toPath__)   ""    ;# list of attributes that make up path

      # Add a global procedure for accessing this object
      # Using eval forces the variable argument list to be expanded so we can pass
      # it to the proc.
      proc ::$aliasTag { cmd args } "return \[eval Alias::\$cmd $aliasTag \$args\]"

      # we'll abuse the fact that Tcl lists are just strings here...
      set from [lindex $mapping 0]
      set arrow [lindex $mapping 1]
      set to [lindex $mapping 2]

      if { $arrow != "-->" } {
         error "In alias mapping, expected '-->', got '$arrow'"
      }

      foreach { alias(__fromType__) \
                alias(__fromRoot__) \
                alias(__fromPath__) } [splitMapPath $from] {}

      foreach { alias(__toType__) \
                alias(__toRoot__) \
                alias(__toPath__) } [splitMapPath $to] {}

      #parray alias

      if { $alias(__toType__) == "*" } {
         error "Wild-cards not allowed in 'to' side of alias: $mapping"
      }
      if { $alias(__toRoot__) == "*" } {
         error "Wild-cards not allowed in 'to' side of alias: $mapping"
      }
      if { [lsearch -exact $alias(__toPath__) "*"] >= 0 } {
         error "Wild-cards not allowed in 'to' side of alias: $mapping"
      }

      return $aliasTag
   }

   ##
   # Destroy an alias object
   proc Destroy { alias } {
      unset Alias::$alias
      rename $alias {}
   }

   # Accessor functions
   proc GetFromType { aliasName } {
      upvar 0 Alias::$aliasName this
      return $this(__fromType__)
   }
   proc GetFromRoot { aliasName } {
      upvar 0 Alias::$aliasName this
      return $this(__fromRoot__)
   }
   proc GetFromPath { aliasName } {
      upvar 0 Alias::$aliasName this
      return $this(__fromPath__)
   }

   proc GetToType { aliasName } {
      upvar 0 Alias::$aliasName this
      return $this(__toType__)
   }
   proc GetToRoot { aliasName } {
      upvar 0 Alias::$aliasName this
      return $this(__toRoot__)
   }
   proc GetToPath { aliasName } {
      upvar 0 Alias::$aliasName this
      return $this(__toPath__)
   }

   ##
   # Test whether this Alias matches a particular pattern
   #
   # @param aliasName The Alias object
   # @param otherType S or O
   # @param otherRoot problem-space or operator name (no globs!)
   # @param otherPath List of attribute path components to match
   proc Matches { aliasName otherType otherRoot otherPath } {
      upvar 0 Alias::$aliasName this
      if { ![string match $this(__fromType__) $otherType] } {
         return 0
      }
      if { ![string match $this(__fromRoot__) $otherRoot] } {
         return 0
      }
      return [lcompare $this(__fromPath__) $otherPath]
   }

   ##
   # Write this alias as a string compatible with the 'mapping'
   # parameter of the Create function.
   #
   # @param aliasName The alias object
   # @returns A string
   proc ToString { aliasName } {
      upvar 0 Alias::$aliasName this
      set fromPath [join $this(__fromPath__) "."]
      set toPath [join $this(__toPath__) "."]
      return "$this(__fromType__):$this(__fromRoot__).$fromPath --> \
              $this(__toType__):$this(__toRoot__).$toPath"
              
   }

   ##
   # Split a mapping path into its constituent components.
   #
   # Returns a 3-tuple of the form:
   #    (type, root, path)
   # where:
   #     type - S or O or *
   #     root - A problem-space or operator name, or a glob pattern
   #     path - List of attribute path components
   #
   # @param path The path string (something like S:foo.cat.dog)
   # @returns Triple described above...
   proc splitMapPath { path } {
      set parts [split $path "."]
      foreach { type root } [split [lindex $parts 0] ":"] {}
      return [list $type $root [lrange $parts 1 end]]
   }

} ;# namespace Alias

##
# An object representing a collection of Alias objects.
namespace eval AliasSet {

   ##
   # Create a new AliasSet object
   proc Create {} {
      set aliasSetTag [GetTag AliasSet]
      variable $aliasSetTag
      upvar 0 $aliasSetTag aliasSet

      # Initialize the basic graph structure
      set aliasSet(__tag__) $aliasSetTag
      set aliasSet(__aliases__) {}

      # Add a global procedure for accessing this object
      # Using eval forces the variable argument list to be expanded so we can pass
      # it to the proc.
      proc ::$aliasSetTag { cmd args } "return \[eval AliasSet::\$cmd $aliasSetTag \$args\]"

      return $aliasSetTag
   }

   ##
   # Destroy an AliasSet object
   #
   # @param aliasSet The object
   proc Destroy { aliasSet } {
      unset AliasSet::$aliasSet
      rename $aliasSet {}
   }

   ##
   # Returns a list of alias objects
   proc GetAliases { aliasSetName } {
      upvar 0 AliasSet::$aliasSetName this
      return $this(__aliases__)
   }

   ##
   # Add an alias
   proc AddAlias { aliasSetName alias } {
      upvar 0 AliasSet::$aliasSetName this
      lappend this(__aliases__) $alias
      return $aliasSetName
   }

   ##
   # Check all aliases in the set for a match. See Alias::Matches
   #
   # Returns the first matching alias
   proc FindMatchingAlias { aliasSetName otherType otherRoot otherPath } {
      upvar 0 AliasSet::$aliasSetName this

      foreach a $this(__aliases__) {
         if { [$a Matches $otherType $otherRoot $otherPath] } {
            return $a
         }
      }
      return {}
   }
} ;# namespace AliasSet
