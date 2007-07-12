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
  $line =~ s/NEXT_/NEXT/;
  $line =~ s/NOT /NOT/;
  
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
%allAtts = ();
for ($i=0; $i<=$#rules; $i++) {
  # for each rule
  
  # store the result attribute in the set of all generated attributes
  $rules[$i][0] =~ /^([^_]*)/;
  $headAtt = $1;
  $allAtts{$headAtt} = 1;
      
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
    $allAtts{$att} = 1;
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
foreach $att (keys %allAtts) {
# for each attribute
  foreach $otherAtt (keys %attParents) {
  # look at the other attributes' parents
    if (defined ${ $attParents{$otherAtt} }{$att}) {
    # if the other attribute has it is a parent, the other attribute is a child
      ${ $attChildren{$att} }{$otherAtt} = 1;
    }
  }
}

#foreach $att (keys %attParents) {
#  if ($att =~ /^TRUE/ or $att =~ /^L/ or $att =~ /^NEXT/ or $att =~ /^A/) {
#    print "for att $att:\n";
#    foreach $parent (keys %{ $attParents{$att} }) {
#      if ($parent =~ /^TRUE/ or $parent =~ /^L/ or $parent =~ /^NEXT/ or $parent =~ /^A/) { 
#        print "\tparent is $parent\n";
#      }
#    }
#  }
#}
#
#foreach $att (keys %attChildren) {
#  if ($att =~ /^TRUE/ or $att =~ /^L/ or $att =~ /^NEXT/ or $att =~ /^A/ or $att =~ /^NOTTRUE/) { 
#    print "for att $att:\n";
#    foreach $child (keys %{ $attChildren{$att} }) {
#      if ($child =~ /^TRUE/ or $child =~ /^L/ or $child =~ /^NEXT/ or $child =~ /^A/) { 
#        print "\tchild is $child\n";
#      }
#    }
#  }
#}
#

# find GS predicates whose children are only enabling legal actions, and the
# next version of itself
# They can also lead to non-zero goals
#
# These are 'action enabling state': all else being equal, if they are on the state, and go away, we
# want to assume that is always a bad thing
#
# This enables us to detect things like shooting at nothing in MM to be bad--
# we see a game-state change (no longer holding gun), we need to know that
# isn't enough to justify pursuing that path.

foreach $att (keys %attChildren) {
  if ($att =~ /^TRUE/) {
    $disqualified = 0;
    $qualified = 0;
    $att =~ /^TRUE(.*)/;
    $root = $1;
    #   print "inspecting $root\n";
    foreach $child (keys %{ $attChildren{$att} }) {
      if ($child =~ /^L/ or $child =~ /^NEXT/) { 
        if ($child =~ /^NEXTgoal(\d+)/) {
          if ($1 == 0) {
#            print "disqualified $root for leading to zero goal.\n";
            $disqualified = 1;
            # this predicate could lead to a goal of zero, 
            # so our assumption about it always being bad to be rid of it is
            # false
          }
        }
        elsif (not $child =~ /^L/ and not $child =~ /NEXT$root/) {
          $disqualified = 1;
#             print "disqualified: $child\n";
        }
        if ($child =~ /^L/) {
          $qualified = 1;
        }
      }
    }
    # check that the negation of the predicate doesn't lead to anything
    # except non-optimal goals
    $negation = "NOT$att";
    foreach $child (keys %{ $attChildren{$negation} }) {
      if ($child =~ /^NEXT/ or $child =~ /^L/) {
        if ($child =~ /^NEXTgoal(\d+)/) {
          if ($1 < 100) {
            next;
          }
          else {
            $disqualified = 1;
            #  print "disqualified $att as $negation leads to $child.\n";
          }
        }
        else {
          $disqualified = 1;
          #  print "disqualified $att as $negation leads to $child.\n";
        }
      }
    }

    if ($qualified == 1 and $disqualified == 0) {
      print "action-enabling-state $root\n";
    }
  }
}


# remove temp file
print `rm $file`;
