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
  $line =~ s/NOT //;
  $line =~ s/_+/_/g;
  $line =~ s/_[\d\.]+/_!Number!/g;
  $bind0 = 0;
  $bind1 = 0;
  $bind2 = 0;
  
  if (not $line =~ /\w/) { next; }
  if (not $inRule) {
    if ($line =~ /BEGIN /) {
      $line =~ s/BEGIN //;
      $inRule = 1;
      $ruleCount++;
      push @lines, $line;
    }
    else {
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
      
      if ($bind0) {
        ${ $groundings{"$predicate/0"} }{$bind0} = 1;
      } 
      if ($bind1) {
        ${ $groundings{"$predicate/1"} }{$bind1} = 1;
      } 
      if ($bind2) {
        ${ $groundings{"$predicate/2"} }{$bind2} = 1;
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

#foreach $position (keys %groundings) {
#  foreach $ground (keys %{ $groundings{$position} }) {
#    print "initial grounding: $position $ground\n";
#  }
#}

%aliases = ();
for ($i=0; $i<=$#rules; $i++) {
  # for each rule
  %varNames = ();
#  print "rule starting $rules[$i][0]\n";
  for ($j=0; $j<=$#{ $rules[$i] }; $j++) {
    $line = $rules[$i][$j];
    $bind0 = 0;
    $bind1 = 0;
    $bind2 = 0;
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
    if ($bind0 and not $bind0 =~ /V/) {
      ${ $groundings{"$predicate/0"} }{$bind0} = 1;
    }
    elsif ($bind0) {
      ${ $varNames{"$predicate/0"} }{$bind0} = 1;
    } 
    if ($bind1 and not $bind1 =~ /V/) {
      ${ $groundings{"$predicate/1"} }{$bind1} = 1;
    }
    elsif ($bind1) {
      ${ $varNames{"$predicate/1"} }{$bind1} = 1;
    } 
    if ($bind2 and not $bind2 =~ /V/) {
      ${ $groundings{"$predicate/2"} }{$bind2} = 1;
    }
    elsif ($bind2) {
      ${ $varNames{"$predicate/2"} }{$bind2} = 1;
    } 
  }

  foreach $predicate (keys %varNames) {
    #  print "predicate: $predicate\n";
    foreach $variable (keys %{ $varNames{$predicate} }) {
      #  print "  has var $variable\n";
      foreach $otherPredicate (keys %varNames) {
        next if ($predicate =~ /^$otherPredicate$/);
        if (defined ${ $varNames{$otherPredicate} }{$variable}) {
          #    print "    $otherPredicate also has $variable\n";
          ${ $aliases{$predicate} }{$otherPredicate} = 1;
        }
        else {
          #      print "    $otherPredicate has no $variable\n";
        }
      }
    }
  }
}

$quiescent = 0;
while ($quiescent == 0) {
  $quiescent = 1;

  foreach $alias1 (keys %aliases) {
#    print "inh from $alias1\n";
    if ($alias1 =~ /^distinct\//) {
      next;
    }
    foreach $alias2 (keys %{$aliases{$alias1} }) {
      if ($alias2 =~ /^distinct\//) {
        next;
      }
      #     print "$alias1 is also $alias2\n";
      foreach $ground (keys %{$groundings{$alias1} }) {
        if (not defined ${ $groundings{$alias2} }{$ground}) {
          ${ $groundings{$alias2} }{$ground} = 1;
          #print "$alias2 inherit $ground from $alias1\n";
          $quiescent = 0;
        }
      }
    }
  }
}

foreach $position (sort keys %groundings) {
#  print "p: $position\n";
  foreach $ground (sort keys %{ $groundings{$position} }) {
    $position =~ s/\// /;
    $position =~ s/A//;
    $position =~ s/TRUE//;
    print "$position $ground\n";
  }
}

# remove temp file
print `rm $file`;
