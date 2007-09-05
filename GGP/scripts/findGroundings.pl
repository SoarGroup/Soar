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


# read through a parsed kif, gather rules into an array of arrays
# where each outer array is a rule
foreach $line (`cat $file`) {
  chomp $line;
  $line =~ s/TRUE_/TRUE/;
  $line =~ s/NEXT_/NEXT/;
  if ($line =~ /NOT /) {
    # ignore negated conditions, as the variables must be bound elsewhere
    next;
  }
  $line =~ s/_+/_/g;
  if ($line =~ /^distinct_/) {
    next;
  }
  $line =~ s/_[\d\.]+/_!Number!/g;
  $bind0 = 0;
  $bind1 = 0;
  $bind2 = 0;
  $bind3 = 0;
  $bind4 = 0;
  
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
      $line =~ s/init_/TRUE/;
      if ($line =~ /^([^_]+)$/) {
        $predicate = $1;
      }
      elsif ($line =~ /^([^_]+)_([^_]+)$/) {
        $predicate = $1;
        $bind0 = $2;
      }
      elsif ($line =~ /^([^_]+)_([^_]+)_([^_]+)$/) {
        $predicate = $1;
        $bind0 = $2;
        $bind1 = $3;
      }
      elsif ($line =~ /^([^_]+)_([^_]+)_([^_]+)_([^_]+)$/) {
        $predicate = $1;
        $bind0 = $2;
        $bind1 = $3;
        $bind2 = $4;
      }
      elsif ($line =~ /^([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)$/) {
        $predicate = $1;
        $bind0 = $2;
        $bind1 = $3;
        $bind2 = $4;
        $bind3 = $5;
      }
      elsif ($line =~ /^([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)$/) {
        $predicate = $1;
        $bind0 = $2;
        $bind1 = $3;
        $bind2 = $4;
        $bind3 = $5;
        $bind4 = $6;
      }
      else {
        die "bad line: $line\n";
      }
      
      if ($bind0) {
        ${ $groundings{"$predicate/0"} }{$bind0} = 1;
      } 
      if ($bind1) {
        ${ $groundings{"$predicate/1"} }{$bind1} = 1;
      } 
      if ($bind2) {
        ${ $groundings{"$predicate/2"} }{$bind2} = 1;
      } 
      if ($bind3) {
        ${ $groundings{"$predicate/3"} }{$bind3} = 1;
      } 
      if ($bind4) {
        ${ $groundings{"$predicate/4"} }{$bind4} = 1;
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
    $bind0 = 0;
    $bind1 = 0;
    $bind2 = 0;
    $bind3 = 0;
    $bind4 = 0;
    if ($line =~ /^([^_]+)$/) {
      $predicate = $1;
    }
    elsif ($line =~ /^([^_]+)_([^_]+)$/) {
      $predicate = $1;
      $bind0 = $2;
    }
    elsif ($line =~ /^([^_]+)_([^_]+)_([^_]+)$/) {
      $predicate = $1;
      $bind0 = $2;
      $bind1 = $3;
    }
    elsif ($line =~ /^([^_]+)_([^_]+)_([^_]+)_([^_]+)$/) {
      $predicate = $1;
      $bind0 = $2;
      $bind1 = $3;
      $bind2 = $4;
    }
    elsif ($line =~ /^([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)$/) {
      $predicate = $1;
      $bind0 = $2;
      $bind1 = $3;
      $bind2 = $4;
      $bind3 = $5;
    }
    elsif ($line =~ /^([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)$/) {
      $predicate = $1;
      $bind0 = $2;
      $bind1 = $3;
      $bind2 = $4;
      $bind3 = $5;
      $bind4 = $6;
    }
    else {
      die "bad line: $line\n";
    }

    $varPredicate = $predicate;
    if ($j == 0) {
      # mark this as a predicate in the head
      $varPredicate = "head!$predicate";
    }

    if ($bind0 and not $bind0 =~ /V/) {
      ${ $groundings{"$predicate/0"} }{$bind0} = 1;
    }
    elsif ($bind0) {
      ${ $varNames{"$varPredicate/0"} }{$bind0} = 1;
    } 
    if ($bind1 and not $bind1 =~ /V/) {
      ${ $groundings{"$predicate/1"} }{$bind1} = 1;
    }
    elsif ($bind1) {
      ${ $varNames{"$varPredicate/1"} }{$bind1} = 1;
    } 
    if ($bind2 and not $bind2 =~ /V/) {
      ${ $groundings{"$predicate/2"} }{$bind2} = 1;
    }
    elsif ($bind2) {
      ${ $varNames{"$varPredicate/2"} }{$bind2} = 1;
    } 
    if ($bind3 and not $bind3 =~ /V/) {
      ${ $groundings{"$predicate/3"} }{$bind3} = 1;
    }
    elsif ($bind3) {
      ${ $varNames{"$varPredicate/3"} }{$bind3} = 1;
    } 
    if ($bind4 and not $bind4 =~ /V/) {
      ${ $groundings{"$predicate/4"} }{$bind4} = 1;
    }
    elsif ($bind4) {
      ${ $varNames{"$varPredicate/4"} }{$bind4} = 1;
    } 
  }

  # let a predicatePos inherit from another predicatePos if it is in the head
  # what is inherited should be the intersection of the possible groundings of
  # all of the other predicatePos's with the same variable
  foreach $predicatePos (keys %varNames) {
    #print "predicatePos: $predicatePos\n";
    next unless ($predicatePos =~ /head!/); # skip all except rule-head
    foreach $variable (keys %{ $varNames{$predicatePos} }) {
      $predicatePos =~ s/head!//;
      #print "  has var $variable\n";
      foreach $otherPredicate (keys %varNames) {
        next if ($otherPredicate =~ /head!/); # different head predicatePos's can't inherit
        next if ($predicatePos =~ /^$otherPredicate$/);
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

foreach $position (sort keys %groundings) {
#  print "p: $position\n";
  foreach $ground (sort keys %{ $groundings{$position} }) {
    $depth = $groundings{$position}{$ground};
    $printPosition = $position;
    $printPosition =~ s/\// /;
    $printPosition =~ s/^AXN//;
    $printPosition =~ s/TRUE//;
    print "$printPosition $ground $depth\n";
  }
}

# remove temp file
print `rm $file`;
