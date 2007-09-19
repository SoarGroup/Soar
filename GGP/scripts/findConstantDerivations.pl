#!/usr/bin/perl

# find constants that can result in the creation of a predicate
# but are not bound to any of the parameters in that predicate
#
# for example, in this rule
# (<= (carryingAmulet ?hero) 
#     (role ?hero)
#     (carrying ?hero amulet1))
#
# We want some way of knowing that the predicate carryingAmulet has something
# to do with the constant amulet1, but amulet1 isn't a grounding of it, so
# findGroundings won't tell us anything.

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

#foreach $line (`$findImplicitPredicates`) {
#  $line =~ /(\S+) (\d+) (\S+)/ or die;
#  $implicitPredicates{"$1/$2 $3"} = 1;
#  print "ip $1/$2 is $3\n";
#}

# read through a parsed kif, gather rules into an array of arrays
# where each outer array is a rule
foreach $line (`cat $file`) {
  chomp $line;
  $line =~ s/TRUE:/TRUE/;
  $line =~ s/NEXT:/NEXT/;
  $line =~ s/NOT //;
  $line =~ s/:+/:/g;
  $line =~ s/goal\d+/goal/; # map all goal predicates to one
  #if ($line =~ /^distinct:/) {
  #  next;
  #}
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
      # bare declarations aren't derived from anything, so ignore them.
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

%parentConstants = ();
%parentPredicates = ();

for ($i=0; $i<=$#rules; $i++) {
  # for each rule
  %varNames = ();
#  print "rule starting $rules[$i][0]\n";
  $headPredicate = "";
  %headConstants = ();
  for ($j=0; $j<=$#{ $rules[$i] }; $j++) {
    $line = $rules[$i][$j];

    @bindings = ();
    $line =~ s/^([^:]+)// or die "can't parse to get predicate: $line";
    $predicate = $1;
    while ($line =~ s/^:([^:]+)//) {
      push @bindings, $1;
    }

    #for ($k=0; $k<=$#bindings; $k++) {
    #  if ($bindings[$k] and defined $implicitPredicates{"$predicate/$k $bindings[$k]"}) { 
    #    $predicate = "$predicate\%$k\%$bindings[$k]";
    #    $bindings[$k] = 0;
    #  }
    #}
    
    if ($j == 0) {
      $headPredicate = $predicate;
      for ($k=0; $k<=$#bindings; $k++) {
        if ($bindings[$k] and not $bindings[$k] =~ /^V/ and not $bindings[$k] eq "!Number!") {
          $headConstants{$bindings[$k]} = 1;
        }
      }
    }
    else {
      # body predicate, all constant are inherited by the head predicate
      for ($k=0; $k<=$#bindings; $k++) {
        if ($bindings[$k] and not $bindings[$k] =~ /^V/ and not $bindings[$k] eq "!Number!" 
            and not defined $headConstants{$bindings[$k]} ) {
          $parentConstants{$headPredicate}{$bindings[$k]} = 1;
        }
        elsif ($bindings[$k]) {
          # bound to a variable, ignore
        }
      }
      # mark the head predicate to inherit the parentConstants of the body
      # predicate
      unless ($predicate eq "distinct") {
        $parentPredicates{$headPredicate}{$predicate} = 1;
      }
    }
  }
}

$quiescent = 0;
$depth = 1;
while ($quiescent == 0) {
  $depth++;
  $quiescent = 1;

  foreach $childPredicate (keys %parentPredicates) {
    foreach $parentPredicate (keys %{ $parentPredicates{$childPredicate} }) {
      foreach $constant (keys %{$parentConstants{$parentPredicate} }) {
        if (not defined $parentConstants{$childPredicate}{$constant}) {
          $parentConstants{$childPredicate}{$constant} = 1;
          # print "$childPredicate inherits $constant from $parentPredicate at depth $depth\n";
          $quiescent = 0;
        }
      }
    }
  }
}

#%expandedGroundings = ();
# re-expand implicit predicates to actual predicates
#foreach $position (sort keys %groundings) {
#  $originalPosition = $position;
#  $position =~ s/%[^\/]+//;

#  die if ($position =~ /%/);

  # if the ppos was originally type%1%item/0, it is now type/0
#  foreach $grounding (keys %{ $groundings{$originalPosition} }) {
#    $expandedGroundings{$position}{$grounding} = $groundings{$originalPosition}{$grounding};
#  }
#}

#foreach $entry (keys %implicitPredicates) {
#  $entry =~ /(\S+) (\S+)/ or die;
#  $expandedGroundings{$1}{$2} = 1;
#}

foreach $predicate (sort keys %parentConstants) {
  foreach $constant (sort keys %{ $parentConstants{$predicate} }) {
    $predicate =~ s/AXN//;
    $predicate =~ s/TRUE//;
    print "$predicate $constant\n";
  }
}

# remove temp file
print `rm $file`;
