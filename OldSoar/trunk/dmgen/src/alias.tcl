
namespace eval Alias {

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

   proc Destroy { alias } {
      unset Alias::$alias
      rename $alias {}
   }

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

   proc ToString { aliasName } {
      upvar 0 Alias::$aliasName this
      set fromPath [join $this(__fromPath__) "."]
      set toPath [join $this(__toPath__) "."]
      return "$this(__fromType__):$this(__fromRoot__).$fromPath --> \
              $this(__toType__):$this(__toRoot__).$toPath"
              
   }

   proc Print { aliasName } {
      upvar 0 Alias::$aliasName this
      puts "Alias: from $this(__fromType__) $this(__fromRoot__) $this(__fromPath__) to \
                        $this(__toType__) $this(__toRoot__) $this(__toPath__)"
   }

   proc splitMapPath { path } {
      set parts [split $path "."]
      foreach { type root } [split [lindex $parts 0] ":"] {}
      return [list $type $root [lrange $parts 1 end]]
   }

} ;# namespace Alias

namespace eval AliasSet {

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

   proc Destroy { aliasSet } {
      unset AliasSet::$aliasSet
      rename $aliasSet {}
   }

   proc GetAliases { aliasSetName } {
      upvar 0 AliasSet::$aliasSetName this
      return $this(__aliases__)
   }

   proc AddAlias { aliasSetName alias } {
      upvar 0 AliasSet::$aliasSetName this
      lappend this(__aliases__) $alias
      return $aliasSetName
   }

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
