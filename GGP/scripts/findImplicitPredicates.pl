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

$file = "$kifFile\_fip\.proc";
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
  $bind5 = 0;
  $bind6 = 0;
  
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
      elsif ($line =~ /^([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)$/) {
        $predicate = $1;
        $bind0 = $2;
        $bind1 = $3;
        $bind2 = $4;
        $bind3 = $5;
        $bind4 = $6;
        $bind5 = $7;
      }
      elsif ($line =~ /^([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)$/) {
        $predicate = $1;
        $bind0 = $2;
        $bind1 = $3;
        $bind2 = $4;
        $bind3 = $5;
        $bind4 = $6;
        $bind5 = $7;
        $bind6 = $8;
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
      if ($bind5) {
        ${ $groundings{"$predicate/5"} }{$bind5} = 1;
      } 
      if ($bind6) {
        ${ $groundings{"$predicate/6"} }{$bind6} = 1;
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
    $bind5 = 0;
    $bind6 = 0;
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
    elsif ($line =~ /^([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)$/) {
      $predicate = $1;
      $bind0 = $2;
      $bind1 = $3;
      $bind2 = $4;
      $bind3 = $5;
      $bind4 = $6;
      $bind5 = $7;
    }
    elsif ($line =~ /^([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)_([^_]+)$/) {
      $predicate = $1;
      $bind0 = $2;
      $bind1 = $3;
      $bind2 = $4;
      $bind3 = $5;
      $bind4 = $6;
      $bind5 = $7;
      $bind6 = $8;
    }
    else {
      die "bad line: $line\n";
    }

    if ($bind0 and not $bind0 =~ /V/) {
      #${ $groundings{"$predicate/0"} }{$bind0} = 1;
    }
    elsif ($bind0) {
      $groundings{"$predicate/0"} = ();
    } 
    if ($bind1 and not $bind1 =~ /V/) {
      #${ $groundings{"$predicate/1"} }{$bind1} = 1;
    }
    elsif ($bind1) {
      $groundings{"$predicate/1"} = ();
    } 
    if ($bind2 and not $bind2 =~ /V/) {
      #${ $groundings{"$predicate/2"} }{$bind2} = 1;
    }
    elsif ($bind2) {
      $groundings{"$predicate/2"} = ();
    } 
    if ($bind3 and not $bind3 =~ /V/) {
      #${ $groundings{"$predicate/3"} }{$bind3} = 1;
    }
    elsif ($bind3) {
      $groundings{"$predicate/3"} = ();
    } 
    if ($bind4 and not $bind4 =~ /V/) {
      #${ $groundings{"$predicate/4"} }{$bind4} = 1;
    }
    elsif ($bind4) {
      $groundings{"$predicate/4"} = ();
    } 
    if ($bind5 and not $bind5 =~ /V/) {
      #${ $groundings{"$predicate/5"} }{$bind5} = 1;
    }
    elsif ($bind5) {
      $groundings{"$predicate/5"} = ();
    } 
    if ($bind6 and not $bind6 =~ /V/) {
      #${ $groundings{"$predicate/6"} }{$bind6} = 1;
    }
    elsif ($bind6) {
      $groundings{"$predicate/6"} = ();
    } 
  }

}

foreach $position (sort keys %groundings) {
#  print "p: $position\n";
  foreach $ground (sort keys %{ $groundings{$position} }) {
    if ($ground =~ /!Number!/) { next; }
    $printPosition = $position;
    $printPosition =~ s/\// /;
    $printPosition =~ s/^AXN//;
    $printPosition =~ s/TRUE//;
    print "$printPosition $ground\n";
  }
}

# remove temp file
print `rm $file`;
