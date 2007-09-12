#!/usr/bin/perl
# detect constants that are only present in the heads of init rules
# these constants are arbitrary

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

%initConstants = ();
%nonInitConstants = ();

%implicitPredicates = ();

# read through a parsed kif, gather rules into an array of arrays
# where each outer array is a rule
foreach $line (`cat $file`) {
  chomp $line;
  $initHead = 0;
  $line =~ s/TRUE:/TRUE/;
  $line =~ s/NEXT:/NEXT/;
  $line =~ s/NOT //;
  $line =~ s/:+/:/g;
  $line =~ s/:[\d\.]+/:!Number!/g;
  
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
      if ($line =~ /init:/) {
        $initHead = 1;
      }
     
      @bindings = ();
      $line =~ s/^([^:]+)// or die "can't parse to get predicate: $line";
      $predicate = $1;
      if ($predicate eq "init") {
        $line =~ s/^:([^:]+)// or die "can't parse to get predicate: $line";
        $predicate = $1;
      }
      while ($line =~ s/^:([^:]+)//) {
        push @bindings, $1;
      }
      
      for ($i=0; $i<=$#bindings; $i++) {
        if ($initHead and $bindings[$i]) {
          $initConstants{$bindings[$i]} = 1;
        }
        elsif ($bindings[$i]) {
          $nonInitConstants{$bindings[$i]} = 1;
        }
      }
    }
  }
  elsif ($line =~ /END/) {
    $inRule = 0;
  }
  else {
    @bindings = ();
    $line =~ s/^([^:]+)// or die "can't parse to get predicate: $line";
    $predicate = $1;
    while ($line =~ s/^:([^:]+)//) {
      push @bindings, $1;
    }
    
    for ($i=0; $i<=$#bindings; $i++) {
      if ($bindings[$i] and not $bindings[$i] =~ /^V/) {
        $nonInitConstants{$bindings[$i]} = 1;
      }
    }
  }
}

foreach $constant (keys %initConstants) {
  if (not defined $nonInitConstants{$constant}) {
    print "arbitrary $constant\n";
  }
}

print `rm $file`;
