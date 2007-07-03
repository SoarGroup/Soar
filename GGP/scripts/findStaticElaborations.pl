#!/usr/bin/perl
# report all predicates in a kif file that don't derive from game state
# (and hence never change throughout the game)

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

# read through a parsed kif, gather rules into an array of arrays
# where each outer array is a rule
foreach $line (`cat $file`) {
  chomp $line;
  
  if (not $line =~ /\w/) { next; }
  if (not $inRule) {
    if ($line =~ /BEGIN /) {
      $line =~ s/BEGIN //;
      $inRule = 1;
      $ruleCount++;
      push @lines, $line;
    }
    else {
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


@include = ();
%derivedAtts = ();
%allAtts = ();
for ($i=0; $i<=$#rules; $i++) {
  # for each rule
  
  # store the result attribute in the set of all generated attributes
  $rules[$i][0] =~ /^([^_]*)/;
  $allAtts{$1} = 1;
      
  $keep = 0;
  for ($j=1; $j<=$#{@rules[$i]} and not $keep; $j++) {
    # for each line in body of rule
    $rules[$i][$j] =~ /^([^_]*)/;
    $att = $1;
    if ($att =~ /^NOT /) {
      $rules[$i][$j] =~ /^NOT ([^_]*)/;
      $att = $1;
    }
    # get the attribute referenced (either positively or negatively)
    if ($att =~ /TRUE([^_]*)/) {
      # if the attribute referenced is on the game state (something TRUE),
      # the attribute in the head of the rule is derived from the game state
      $rules[$i][0] =~ /^([^_]*)/;
      $derivedAtts{$1} = 1;
 #     print "$1 is derived.\n";
       # we know the result is derived, so move on to the next rule
      $keep = 1;
    }
  }
  if ($keep) {
    # parallel array to @rules, marking if each rule references GS
    push @include, 1;
  }
  else {
    push @include, 0;
  }
}

die unless ($#include == $#rules);

$quiescent = 0;
$pass = 2;
while (not $quiescent) {
  # while we are still adding more derived attributes
  $quiescent = 1;
 # print "pass $pass\n";
  $pass++;
  for ($i=0; $i<=$#rules; $i++) {
    # for each rule
    $keep = 0;
    # skip those we already know are derived
    if ($include[$i]) { next; }
    for ($j=1; $j<=$#{@rules[$i]} and not $keep; $j++) {
      # for each line in rule body
      $rules[$i][$j] =~ /^([^_]*)/;
      $att = $1;
      if ($att =~ /^NOT /) {
        $rules[$i][$j] =~ /^NOT ([^_]*)/;
        $att = $1;
      }
      # get the attribute referenced
      foreach $dAtt (keys %derivedAtts) {
        # search set of known derived attributes 
        # (we know it isn't checking GS, since we catch that above)
        if ($dAtt =~ /^$att$/) {
          $keep = 1;
          $include[$i] = 1;
          $rules[$i][0] =~ /^([^_]*)/;
          if (not defined $derivedAtts{$1}) {
            $derivedAtts{$1} = 1;
           # print "$1 is derived.\n";
            # need to cycle again, to see if this attribute is referenced
            # anywhere
            $quiescent = 0;
          }
        }
      }
    }
  }
}

# report each attribute not derived from game state
foreach $att (keys %allAtts) {
  if (not defined $derivedAtts{$att}) {
    print "$att\n";
  }
}

# remove temp file
print `rm $file`;
