#!/usr/bin/perl

# Report predicate positions that are always bound to constants (and what those
# constants are)
#
# Those predicates could then be equivalently re-written 
# eg (health hero ?val) -> (heroHealth ?val), if the first parameter is never a
# variable

die unless ($#ARGV == 0);
$kifFile = $ARGV[0];
$processor = "./processKif.pl";
die unless (-e $kifFile);
die unless (-e $processor);

$file = "$kifFile\:fip\.proc";
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

%variables = ();

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
      if ($bindings[$k] and not $bindings[$k] =~ /^V/) {
        $groundings{"$predicate/$k"}{$bindings[$k]} = 1;
      }
      elsif ($bindings[$k]) {
        # bound to a variable
        $variables{"$predicate/$k"} = 1;
      }
    }
  }
}

foreach $position (sort keys %groundings) {
#  print "p: $position\n";
  foreach $ground (sort keys %{ $groundings{$position} }) {
    if (defined $variables{$position}) { next; }
    if ($ground =~ /!Number!/) { next; }
    $printPosition = $position;
    $printPosition =~ s/\// /;
    #   $printPosition =~ s/^AXN//;
    #  $printPosition =~ s/TRUE//;
    print "$printPosition $ground\n";
  }
}

# remove temp file
print `rm $file`;
