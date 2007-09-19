#!/usr/bin/perl

# find facts and init statements dealing with constants
# use in constant mapper, give a mapping a bump if the constants
# have identical init statements

# output: predicate-name hash-of-context constant-name

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
  $line =~ s/:(-?[\d\.]+)/:!Number$1/g;
  #if ($line =~ /^distinct:/) {
  #  next;
  #}
  
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
      $line =~ s/init://;
     
      @bindings = ();
      $line =~ s/^([^:]+)// or die "can't parse to get predicate: $line";
      $predicate = $1;
      if ($predicate eq "location" or $predicate eq "blocked") {
        # ignore facts about these (spatial assumptions)
        next;
      }
      while ($line =~ s/^:([^:]+)//) {
        push @bindings, $1;
      }
      for ($i=0; $i<=$#bindings; $i++) {
        next if (not defined $bindings[$i]);
        next if ($bindings[$i] =~ /!Number/);
        $contextString = "hash";
        for ($j=0; $j<=$#bindings; $j++) {
          next if ($j == $i);
          if (defined $bindings[$j]) {
            $contextString .= "^p$j:$bindings[$j]";
          }
        }
        print "$predicate $contextString $bindings[$i]\n";
      }
    }
  }
}
# remove temp file
print `rm $file`;
