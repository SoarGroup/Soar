#!/usr/bin/perl

die unless ($#ARGV == 0);
$kifFile = $ARGV[0];
$processor = "./processKif.pl";
die unless (-e $kifFile);
die unless (-e $processor);

$file = "$kifFile\.proc";
print `$processor $kifFile > $file`;

$inRule = 0;
our @lines = ();
our @rules = ();
$ruleCount = 0;
%groundings = ();

$findImplicitPredicates = "./findImplicitPredicates.pl $kifFile";

%implicitPredicates = ();

# if there are predicate-positions that are only ever bound to constants,
# pretend that the predicate name includes the constant in question
# (this prevents some overgenerality, eg with types in wargame)

foreach $line (`$findImplicitPredicates`) {
  $line =~ /(\S+) (\d+) (\S+)/ or die;
  $implicitPredicates{"$1/$2 $3"} = 1;
#  print "ip $1/$2 is $3\n";
}

# read through a parsed kif, gather rules into an array of arrays
# where each outer array is a rule
foreach $line (`cat $file`) {
  chomp $line;
  $line =~ s/TRUE:/TRUE/;
  $line =~ s/NEXT:/NEXT/;
  if ($line =~ /NOT /) {
    # ignore negated conditions, as the variables must be bound elsewhere
    next;
  }
  $line =~ s/:+/:/g;
  if ($line =~ /^distinct:/) {
    next;
  }
  $line =~ s/:-?[\d\.]+/:!Number!/g;
  
  if (not $line =~ /\w/) { next; }
  if (not $inRule) {
    if ($line =~ /BEGIN /) {
      $line =~ s/BEGIN //;
      $inRule = 1;
      $ruleCount++;
      push @lines, $line;
    }
    else {
      # not in a rule, a bare declaration
      $line =~ s/init:/TRUE/;
     
      @bindings = ();
      $line =~ s/^([^:]+)// or die "can't parse to get predicate: $line";
      $predicate = $1;
      while ($line =~ s/^:([^:]+)//) {
        push @bindings, $1;
      }
      
      for ($i=0; $i<=$#bindings; $i++) {
        if ($bindings[$i] and defined $implicitPredicates{"$predicate/$i $bindings[$i]"}) { 
          $predicate = "$predicate\%$i\%$bindings[$i]";
          $bindings[$i] = 0;
        }
      }
      
      for ($i=0; $i<=$#bindings; $i++) {
        if ($bindings[$i]) {
          $groundings{"$predicate/$i"}{$bindings[$i]} = 1;
        }
      }
    }
  }
  elsif ($line =~ /END/) {
    $inRule = 0;
    if ($#lines > 0) {
      push @rules, [@lines];
    }
    @lines = ();
  }
  else {
    push @lines, $line;
  }
}

# some implied groundings
$groundings{"+/0"}{"!Number!"} = 1;
$groundings{"+/1"}{"!Number!"} = 1;
$groundings{"+/0"}{"!Number!"} = 1;
$groundings{"-/0"}{"!Number!"} = 1;
$groundings{"-/1"}{"!Number!"} = 1;
$groundings{"-/2"}{"!Number!"} = 1;
$groundings{">/0"}{"!Number!"} = 1;
$groundings{">/1"}{"!Number!"} = 1;
$groundings{">/2"}{"!Number!"} = 1;
$groundings{">=/0"}{"!Number!"} = 1;
$groundings{">=/1"}{"!Number!"} = 1;
$groundings{">=/2"}{"!Number!"} = 1;

#foreach $position (keys %groundings) {
#  foreach $ground (keys %{ $groundings{$position} }) {
#    print "initial grounding: $position $ground\n";
#  }
#}

%inheritances = ();
for ($i=0; $i<=$#rules; $i++) {
  # for each rule
  %varNames = ();
#  print "rule starting $rules[$i][0]\n";
  for ($j=0; $j<=$#{ $rules[$i] }; $j++) {
    $line = $rules[$i][$j];

    @bindings = ();
    $line =~ s/^([^:]+)// or die "can't parse to get predicate: $line";
    $predicate = $1;
    while ($line =~ s/^:([^:]+)//) {
      push @bindings, $1;
    }

    for ($k=0; $k<=$#bindings; $k++) {
      if ($bindings[$k] and defined $implicitPredicates{"$predicate/$k $bindings[$k]"}) { 
        $predicate = "$predicate\%$k\%$bindings[$k]";
        $bindings[$k] = 0;
      }
    }
    
    $varPredicate = $predicate;
    if ($j == 0) {
      # mark this as a predicate in the head
      $varPredicate = "head!$predicate";
    }
    
    
    for ($k=0; $k<=$#bindings; $k++) {
      if ($bindings[$k] and not $bindings[$k] =~ /^V/) {
        $groundings{"$predicate/$k"}{$bindings[$k]} = 1;
      }
      elsif ($bindings[$k]) {
        # bound to a variable
        $varNames{"$varPredicate/$k"}{$bindings[$k]} = 1;
      }
    }
  }

  # let a predicatePos inherit from another predicatePos if it is in the head
  # what is inherited should be the intersection of the possible groundings of
  # all of the other predicatePos's with the same variable
  foreach $predicatePos (keys %varNames) {
    #print "predicatePos: $predicatePos\n";
    next unless ($predicatePos =~ /head!/); # skip all except rule-head
    foreach $variable (keys %{ $varNames{$predicatePos} }) {
      # there can only be one such $variable, since this is only for things in
      # the head
      $predicatePos =~ s/head!//;
      #print "  has var $variable\n";
      foreach $otherPredicate (keys %varNames) {
        next if ($otherPredicate =~ /head!/); # different head predicatePos's can't inherit
        #    next if ($predicatePos =~ /^$otherPredicate$/);
        if (defined $varNames{$otherPredicate}{$variable}) {
          #print "    $otherPredicate also has $variable\n";
          #$inheritances{$otherPredicate}{$predicatePos} = 1;

          # inheritances is a hash of a hash of lists, format:
          # $inheritance{predicateName/position}{rule-number}[list-entry] =
          # other-predicatename/pos
          #print "$predicatePos in rule $i is also $otherPredicate\n";
          push @{ $inheritances{$predicatePos}{$i} }, $otherPredicate;
          # print "there are $#{ $inheritances{$predicatePos}{$i} } sources.\n";
        }
        else {
          #      print "    $otherPredicate has no $variable\n";
        }
      }
    }
  }
}

$quiescent = 0;
$depth = 1;
while ($quiescent == 0) {
  $depth++;
  $quiescent = 1;

  foreach $inheritingPPos (keys %inheritances) {
    foreach $rule (keys %{ $inheritances{$inheritingPPos} }) {
      # build up the intersection of the groundings of all ppos's in the inheritance
      # list
      # the groundings in that intersection are added to the groundings of the
      # inheritingPPos
      # if any are added, we are no longer quiescent
      
      # print "inspecting $inheritingPPos in rule $rule\n";
      # print "there are $#{ $inheritances{$inheritingPPos}{$rule} } sources.\n";
      %inheritedSet = ();
      $sourceString = "";
      for ($i=0; $i<=$#{ $inheritances{$inheritingPPos}{$rule} }; $i++) {
        $sourcePPos = $inheritances{$inheritingPPos}{$rule}[$i];
        # print "  $inheritingPPos is also $sourcePPos\n";
        $sourceString = "$sourceString $sourcePPos";
        if ($i == 0) {
          foreach $grounding (keys %{$groundings{$sourcePPos}}) {
            # the first time through, each grounding is added to the set
            $inheritedSet{$grounding} = 1;
            # print "  inheriting $grounding from $sourcePPos (initially)\n";
          }
        }
        else {
          foreach $grounding (keys %{$groundings{$sourcePPos}}) {
            # now, only those groundings that we present last time through are
            # updated
            if ($inheritedSet{$grounding} >= 1) {
              # print "  $sourcePPos confirms $grounding\n";
              $inheritedSet{$grounding} = $i+1;
            }
          }
          # clean out the inheritedSet entries that weren't present this time
          # through
          foreach $entry (keys %inheritedSet) {
            if ($inheritedSet{$entry} != $i+1) {
              #   print "  $sourcePPos rules out $entry\n";
              $inheritedSet{$entry} = -1;
            }
          }
        }
      }
      foreach $ground (keys %inheritedSet) {
        if ($inheritedSet{$ground} <= 0) { next; }
        if (not defined $groundings{$inheritingPPos}{$ground}) {
          $groundings{$inheritingPPos}{$ground} = $depth;
          # print "$inheritingPPos inherit $ground from$sourceString at $depth\n";
          $quiescent = 0;
        }
      }
    }
  }
}

%expandedGroundings = ();
# re-expand implicit predicates to actual predicates
foreach $position (sort keys %groundings) {
  $originalPosition = $position;
  $position =~ s/%[^\/]+//;

  die if ($position =~ /%/);
  die unless ($position =~ /^[^:]+\/\d+$/);

  # if the ppos was originally type%1%item/0, it is now type/0
  foreach $grounding (keys %{ $groundings{$originalPosition} }) {
    $expandedGroundings{$position}{$grounding} = $groundings{$originalPosition}{$grounding};
  }
}

foreach $entry (keys %implicitPredicates) {
  $entry =~ /(\S+) (\S+)/ or die;
  $expandedGroundings{$1}{$2} = 1;
}

foreach $position (sort keys %expandedGroundings) {
  #print "p: $position\n";
  foreach $ground (sort keys %{ $expandedGroundings{$position} }) {
    $depth = $expandedGroundings{$position}{$ground};
    $printPosition = $position;
    $printPosition =~ s/\// /;
    $printPosition =~ s/^AXN//;
    $printPosition =~ s/TRUE//;
    print "$printPosition $ground $depth\n";
  }
}

# remove temp file
print `rm $file`;
