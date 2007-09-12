#!/usr/bin/perl
# report three classes of predicates:
# -those that are on GS, but never change (fake game state)
# -those that don't derive from non-fake GS (static elaborations)
# -those that are on GS and change, but don't effect anything else (except a
# goal of < 100 and terminal) (timeout counters)

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
  $line =~ s/TRUE:/TRUE/;
  
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


%attParents = ();
for ($i=0; $i<=$#rules; $i++) {
  # for each rule
  
  # store the result attribute in the set of all generated attributes
  $rules[$i][0] =~ /^([^:]*)/;
  $headAtt = $1;
      
  $keep = 0;
  for ($j=1; $j<=$#{@rules[$i]} and not $keep; $j++) {
    # for each line in body of rule
    $rules[$i][$j] =~ /^([^:]*)/;
    $att = $1;
    if ($att =~ /^NOT /) {
      $rules[$i][$j] =~ /^NOT ([^:]*)/;
      $att = $1;
    }

    # get the attribute referenced (either positively or negatively)
    ${ $attParents{$headAtt} }{$att} = 1;
  }
}

$quiescent = 0;
$waves = 0;
while ($quiescent == 0) {
  $quiescent = 1;
  foreach $att (keys %attParents) {
  # for each attribute
    foreach $parent (keys %{ $attParents{$att} }) {
    # inspect each known parent, that has parents
      if (defined $attParents{$parent}) {
        foreach $pparent (keys %{ $attParents{$parent} }) {
        # look at that parent's parents
          if (not defined ${ $attParents{$att} }{$pparent}) {
          # if the parent's parent is not a parent, it is now
            ${ $attParents{$att} }{$pparent} = 1;
            $quiescent = 0;
          }
        }
      }
    }
  }
  $waves++;
}

%attChildren = ();
# now, derive children from parents
foreach $att (keys %attParents) {
# for each attribute
  foreach $otherAtt (keys %attParents) {
  # look at the other attributes' parents
    if (defined ${ $attParents{$otherAtt} }{$att}) {
    # if the other attribute has it is a parent, the other attribute is a child
      ${ $attChildren{$att} }{$otherAtt} = 1;
    }
  }
}

foreach $att (keys %attParents) {
  print "for att $att:\n";
  foreach $parent (keys %{ $attParents{$att} }) {
    print "\tparent is $parent\n";
  }
}

foreach $att (keys %attChildren) {
  print "for att $att:\n";
  foreach $child (keys %{ $attChildren{$att} }) {
    print "\tchild is $child\n";
 }
}

# report attributes which are on GS, but don't change
# they don't have any parents except themselves
%fakeGS = ();
OUTER: foreach $att (keys %attParents) {
  unless ($att =~ /^TRUE/) {
    next;
  }

  foreach $parent (keys %{ $attParents{$att} }) {
    if (not $parent =~ /$att/) {
      next OUTER;
    }
  }
  $fakeGS{$att} = 1;
  $att =~ s/TRUE//;
  print "fakegs $att\n";
}

# report attributes which are not derived from GS, or only from fake GS
foreach $att (keys %attParents) {
  if (defined $fakeGS{$att}) { next; }
  $noGS = 1;
  foreach $parent (keys %{ $attParents{$att} }) {
    if ($parent =~ /TRUE/ and not defined $fakeGS{$parent}) {
      $noGS = 0;
    }
  }

  if ($noGS == 1) {
    $att =~ s/TRUE//;
    print "static $att\n";
  }
}

# report GS attributes which have no GS parents except themselves,
# and no GS children except themselves
# these are probably counters

OUTER2: foreach $att (keys %attParents) {
  if (defined $fakeGS{$att}) {
    next;
  }
  unless ($att =~ /^TRUE/) {
    next;
  }

  foreach $parent (keys %{ $attParents{$att} }) {
    if ($parent =~ /^TRUE/) {
      if (not $parent =~ /$att/) {
        next OUTER2;
      }
    }
  }

  foreach $child (keys %{ $attChildren{$att} }) {
    if ($child =~ /^TRUE/ and not $child =~ /$att/ and not $child =~ /TRUEgoal/) {
      # I suppose this could miss the case where the goal of the game is to sit
      # still..
      next OUTER2;
    }
  }

  $att =~ s/TRUE//;
  print "counter $att\n";
}


# remove temp file
print `rm $file`;
