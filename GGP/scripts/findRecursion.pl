#!/usr/bin/perl
# report three classes of predicates:
# -those that are on GS, but never change (fake game state)
# -those that don't derive from non-fake GS (static elaborations)
# -those that are on GS and change, but don't effect anything else (except a
# goal of 0) (timeout counters)

die unless ($#ARGV == 0);
$kifFile = $ARGV[0];
$processor = "./processKif_unaliasNT.pl";
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
  $line =~ s/TRUE_/TRUE/;
  
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
  $rules[$i][0] =~ /^([^_]*)/;
  $headAtt = $1;
      
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
#  print "for att $att:\n";
  foreach $parent (keys %{ $attParents{$att} }) {
    if ($parent =~ /^$att$/) {
      print "Recursion on $att.\n";
    }
 #     print "\tparent is $parent\n";
  }
}


# remove temp file
print `rm $file`;
