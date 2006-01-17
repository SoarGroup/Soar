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
# Code for partitioning a production graph into problem-spaces and operators.
#
# Requires:
#     util.tcl
#     production.tcl
#     config.tcl 
#     log.tcl

namespace eval Partition {

##
# Given a production graph and a state vertex, get a list of problem
# spaces that that state refers to.
#
# The reason that this can return a list is because a production may
# test more than one problem space name.
#
# @param graph Production graph
# @param state State vertex
# @returns A list of problem space names
proc GetPsNames { graph state } {

   if ![$graph Get $state isState] {
      return {}
   }

   set special [$graph Get $state special]

   # We route everything under a top-state attribute to the top-ps problem
   # space
   # CFG - topstate to top-ps?
   if { [DmGenCfg::Get FillTopPs] && $special == "TopState" } {
      return "top-ps"
   }

   # Search for children that test ^problem-space.name
   set spec [Production::ProblemSpaceSpec]
   set node $state
   foreach s $spec {
      set node [$graph FindVertex name $s [$graph GetOutAdjacencies $node]]
      if { $node == [Graph::NullVertex] } {
         break
      }
   }
   if { $node != [Graph::NullVertex] } {
      set psNames [$graph Get $node value] ;# list of problem-space names
      # If they are testing for NOT a problem-space name, we assign it to
      # any-ps. This is a bit of a hack, but a general solution for
      # interpretting all relational operators in all tests is out of reach at
      # the moment.
      # an example of this would be:
      # (state <s> ^problem-space.name { <> waiting } )
      if { [$graph Get $node valRelOps] == "not_equal" } {
         set psNames "any-ps"
         LogInfo "not_equal relation found on problem-space.name test, assigning to any-ps."
      }
   } elseif { [DmGenCfg::Get FillAnyPs] && $special != "SuperState" && $special != "TopState" } {
      # A non-superstate that does not inspect problem-space.name goes to any-ps
      # Since we have no idea what the superstate is, it would be stupid to stick
      # that stuff on any-ps.
      set psNames "any-ps"
   } else {
      set psNames {}
   }
      
   return $psNames
}

##
# Like GetPsNames, except for operators.  Given an operator attribute,
# it finds all the names referenced by that operator.
proc GetOpNames { graph op } {
   set special [$graph Get $op special]
   if { $special != "Operator" } {
      return {}
   }

   # TODO This should use Production::OperatorSpec just like GetPsNames.
   set node [$graph FindVertex name name [$graph GetOutAdjacencies $op]]
   if { $node == [Graph::NullVertex] } {
      return {}
   }
   set opNames [$graph Get $node value] ;# return list of operator names

   # If they are testing for NOT an operator name, we treat it as if there
   # was no operator named which means that any structure on the operator
   # attribute will be copied into the problem-space where it's encountered.
   # This is a bit of a hack, but a general solution for
   # interpretting all relational operators in all tests is out of reach at
   # the moment.
   # an example of this would be:
   # (state <s> ^operator.name { <> waiting } )
   if { [$graph Get $node valRelOps] == "not_equal" } {
      LogInfo "not_equal relation found on operator.name test"
      set opNames {}
   }

   return $opNames ;# return list of operator names
}

##
# Partition a production graph into constituent problem spaces and operators.
#
# This gives us a way to merge the various parts of the production into the
# correct problem spaces or operators in the datamap. If top-state is 
# tested, then everthing under that goes to top-ps. If there are multiple
# states testing multiple problem space names, they will all go to the
# correct place.
# 
# Returns a list of tuples, each representing a partition:
# 0 = production start vertex
#        This is the production vertex where the partition starts
# 1 = list of datamap target paths (type root {path}) (path may be empty)
#        This is a list of target paths in the datamap where the start vertex
#        and its children (in the partition) should be merged to.
# 2 = list of { vertex { aliases } } pairs in the partition. Alias will be {} when
#     there is no alias.
#        This is a list of all the vertices in the partition. A vertex with an
#        associated alias should not be "followed". That is, there will be
#        another partition with this vertex as its start vertex. This alias is
#        used simply to indicated shared structure across problem-spaces and 
#        operators.
#
# Partition start vertices include:
#  - identifiable state and operator vertices (using GetPsNames, and GetOpNames)
#  - vertices that match a user-defined alias in the config file
#
# The first type is easy and the set of start vertices can be determined before
# the real partitioning procedure starts. The second type has to be done more 
# dynamically since whether a vertex matches an alias pattern depends on its
# context (the state or operator along with its path).  We can do the alias
# matching during the partitioning procedure of the vertices of the first
# type. Vertices that match an alias can be added to a queue to be partitioned 
# later.
#
# @param p Production graph
# @returns The list of 3-tuples described above.
proc PartitionProduction { p aliasSet } {

   set Parts {}
   # Q is a list of partition start vertices to process. Each entry in the
   # list is a list of the form:
   #     { vertex { list of targets } }
   # where each target is a tuple of the form:
   #     type root path
   # i.e. each target has the same format as an alias
   # We start with just the identifiable states and operators in the
   # production. More may be added to the queue as we progress...
   set Q {}
   foreach s [concat [Production::GetStates $p] [Production::GetOperators $p]] {
      if { [$p Get $s isState] } { 
         set type S
         set psNames [GetPsNames $p $s]
      } else { ;# s is an operator
         set type O
         set psNames [GetOpNames $p $s]
      }

      # If there were no problem spaces or operators found, we just ignore it.
      # This should only happen if FillAnyPs is turned off!
      if { $psNames == {} } { continue }

      set targets {}
      foreach n $psNames {
         lappend targets $type $n {}   ;# (type root path), path is empty for states
      }
      lappend Q [list $s $targets]
   }

   # For all identified states and operators in the production...
   while { [llength $Q] > 0 } {
      foreach { s targets } [lindex $Q 0] {}  ;# get next state and targets
      set Q [lrange $Q 1 end] ;# pop off next state

      # Prepare for depth first traversal...
      foreach v [$p GetVertices] {
         set visited($v) 0
      }
      set visited($s) 1

      set P {} ;# Current partition we're building
      foreach v [$p GetOutAdjacencies $s] {
         partitionProduction $p $v Q visited P $targets {} $aliasSet
      }

      lappend Parts $s $targets $P ;# Add current partition to list of partitions
   }
   return $Parts
}

proc partitionProduction { p v qname visitedName partName targets path aliasSet } {
   upvar $visitedName visited
   upvar $partName P
   upvar $qname Q

   set path [concat $path [$p Get $v name]]
   #puts "$v Path=$path"
   #puts "$v targets=$targets, path=$path"
   if { $visited($v) } { return }
   set visited($v) 1

   # check whether this vertex matches the LHS any alias mappings.
   # If it does this function will add the vertex to the queue for us
   # but we still have to add it (and it's new links) to the current
   # partition.
   set newTargets [checkForAliases $aliasSet $v Q $targets $path]
   if { $newTargets != {} } {
      set link {}
      foreach { tgtType tgtRoot tgtPath } $newTargets {
         lappend link [list $tgtType $tgtRoot $tgtPath]
      }
      lappend P $v $link ;# add v to current partition

      # Since it's in the queue, this vertex will be the start of
      # another partition so we can quit here.
      return
   }

   # If this is not a state with a problem space, or a named operator,
   # we keep going, otherwise, we stop the partition here, it will
   # become a link.
   set vIsState [$p Get $v isState]
   set vPsNames [GetPsNames $p $v]
   set link {}
   if { ![expr $vIsState && { $vPsNames != {} }] } {
      # Is this a named operator?
      set vIsOp [expr [string compare [$p Get $v special] "Operator"] == 0]
      set vOpNames [GetOpNames $p $v]
      if { ![expr $vIsOp && { $vOpNames != {} }] } {
         foreach a [$p GetOutAdjacencies $v] {
            partitionProduction $p $a Q visited P $targets $path $aliasSet
         }
      } else { ;# it's a named operator
         #set link [concat O $vOpNames]
         foreach p $vOpNames {
            lappend link [list O $p {}]
         }
      }
   } else { ;#it's a named problem-space
      #set link [concat S $vPsNames]
      foreach p $vPsNames {
         lappend link [list S $p {}]
      }
   } 
   lappend P $v $link ;# Add v to partition
}

##
# Check whether a vertex matches any aliases.
proc checkForAliases { aliasSet v qname targets path } {
   upvar $qname Q
   set newTargets {}
   foreach { type root tgtPath } $targets {
      set alias [$aliasSet FindMatchingAlias $type $root $path]
      if { $alias != {} } {
         Log "Matched alias '[$alias ToString]' with '$type:$root.$path'"
         lappend newTargets [$alias GetToType] [$alias GetToRoot] [$alias GetToPath]
      }
   }
   # aliases were found so add this vertex to the queue so it can be
   # the start of another partition
   if { $newTargets != {} } {
      lappend Q [list $v $newTargets]
   }
   return $newTargets
}

proc test { } {
   sp { test 
      (state <s> ^name MyPs)
         (<s> ^x 1)
         (<s> ^y 5)
         (<s> ^z <z>)
            (<z> ^moo |hello|)
         (<s> ^superstate <ss>)
            (<ss> ^name << SsPs OtherSSps >>)
            (<ss> ^p 2 ^q 3 ^r 4)
         (<s> ^operator <o>)
            (<o> ^name bob ^param howdy)
   -->
      (<ss> ^p 5)
   }

   set g [Production::Parse test]
   puts "Production $g"

   set as [AliasSet::Create]
   $as AddAlias [Alias::Create "S:MyPs.z --> S:SsPs"]
   set p [PartitionProduction $g $as]
   foreach { start targets verts } $p {
      puts "Partition: start=$start"
      foreach { type root path } $targets {
         puts "  Target: type=$type, root=$root, path=$path"
      }
      puts "   verts = $verts"
   }
}

} ;# namespace

IfStandAlone { ;# Some test code
   DmGenPartition::test
}

