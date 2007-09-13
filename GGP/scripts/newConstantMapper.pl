#!/usr/bin/perl


die unless ($#ARGV == 1);

$sourceKif = $ARGV[0];
$targetKif = $ARGV[1];

$ENV{"GGP_PATH"} = "../";
$ENV{"PYTHONPATH"} = ".:../analogy/rule_mapper:./pyparser";

$findGroundings = "./findGroundings.pl";
$findConstantDevs = "./findConstantDerivations.pl";
$ruleMapper = "python ../analogy/rule_mapper/rule_mapper2.py";

checkFor($sourceKif);
checkFor($targetKif);

%sourcePredicateToTarget = ();
%targetPredicateToSource = ();

%sourcePPosToGroundings = ();
%targetPredicateToGroundings = ();

%sourceConstantsToPPos = ();
%targetPredicateToGroundings = ();

$unchangeable{"exit"} = 1;
$unchangeable{"north"} = 1;
$unchangeable{"south"} = 1;
$unchangeable{"east"} = 1;
$unchangeable{"west"} = 1;

#foreach $line (`cat esc-maps`) {
foreach $line (`$ruleMapper $sourceKif $targetKif | grep -v UNROLL`) {
  print "map predicate $line";
  if ($line =~ /(\S+) (\S+)/) {
    push @{ $sourcePredicateToTarget{$1} }, $2;
    push @{ $targetPredicateToSource{$2} }, $1;
  }
}

# mappings that are always present
push @{ $sourcePredicateToTarget{"goal"} }, "goal";
push @{ $targetPredicateToSource{"goal"} }, "goal";
push @{ $sourcePredicateToTarget{"terminal"} }, "terminal";
push @{ $targetPredicateToSource{"terminal"} }, "terminal";

foreach $line (`$findGroundings $sourceKif`) {
  $line =~ /(\S+) (\d+) (\S+) (\d+)/ or die;
  $predicate = $1;
  $position = $2;
  $constant = $3;
  $score = $4;
  unless ($constant =~ /!Number!/ or defined $unchangeable{$constant}) {
    push @{ $sourcePPosToGroundings{"$predicate"} }, "$constant $score";
    push @{ $sourceConstantsToPPos{$constant} }, "$predicate $position $score";
  }
}
  
foreach $line (`$findGroundings $targetKif`) {
  $line =~ /(\S+) (\d+) (\S+) (\d+)/ or die;
  $predicate = $1;
  $position = $2;
  $constant = $3;
  $score = $4;
  unless ($constant =~ /!Number!/ or defined $unchangeable{$constant}) {
    push @{ $targetPredicateToGroundings{"$predicate"} }, "$position $constant $score";
    push @{ $targetConstantsToPPos{$constant} }, "$predicate $score";
  }
}

foreach $line (`$findConstantDevs $sourceKif`) {
  $line =~ /(\S+) (\S+)/ or die;
  $predicate = $1;
  $constant = $2;
  $position = 999; # marker for constant deriver
  $score = 1; # not used

  unless ($constant =~ /!Number!/ or defined $unchangeable{$constant}) {
    push @{ $sourcePPosToGroundings{"$predicate"} }, "$constant $score";
    push @{ $sourceConstantsToPPos{$constant} }, "$predicate $position $score";
  }
}

foreach $line (`$findConstantDevs $targetKif`) {
  $line =~ /(\S+) (\S+)/ or die;
  $predicate = $1;
  $constant = $2;
  $position = 999; # marker for constant deriver
  $score = 1; # not used

  unless ($constant =~ /!Number!/ or defined $unchangeable{$constant}) {
    push @{ $targetPredicateToGroundings{"$predicate"} }, "$position $constant $score";
    push @{ $targetConstantsToPPos{$constant} }, "$predicate $score";
  }
}

%mappingScores = ();
%constantPairUsedPredicate = ();

foreach $sourceConstant (keys %sourceConstantsToPPos) {
#  print "looking @ source constant $sourceConstant\n";
  foreach $sourcePredicateSet (@{ $sourceConstantsToPPos{$sourceConstant} }) {
    $sourcePredicateSet =~ /(\S+) (\d+) (\d+)/;
    $sourcePredicate = $1;
    $sourcePosition = $2;
    $sourceScore = $3;
    #   print " looking @ source predicate $sourcePredicate\n";
    foreach $targetPredicate (@{ $sourcePredicateToTarget{$sourcePredicate} }) {
      # print "  looking @ target predicate $targetPredicate\n";
      foreach $targetConstantSet (@{ $targetPredicateToGroundings{$targetPredicate} }) {
        $targetConstantSet =~ /(\d+) (\S+) (\d+)/;
        $targetPosition = $1;
        $targetConstant = $2;
        $targetScore = $3;
        # we don't care about the positions being identical, except we don't
        # want to count a constant that is a deriver in one and a binding in
        # the other as a match
        next if ($sourcePosition eq "999" and not $targetPosition eq "999");
        next if ($targetPosition eq "999" and not $sourcePosition eq "999");
        # print "   looking @ target constant $targetConstant\n";
        if ($sourcePosition eq "999") {
          $score = .95; # let bindings dominate over derivers in case of a tie
        }
        else {
          $score = 1;
        }
        if (defined $constantPairUsedPredicate{$sourceConstant}{$targetConstant}{$targetPredicate}) { next; }
        if (not defined $mappingScores{$sourceConstant}{$targetConstant}) {
          $mappingScores{$sourceConstant}{$targetConstant} = $score;
          $mappingTargetPredicates{$sourceConstant}{$targetConstant} = "$targetPredicate/$targetPosition";
        }
        else {
          $mappingScores{$sourceConstant}{$targetConstant} += $score;
          $mappingTargetPredicates{$sourceConstant}{$targetConstant} .= " $targetPredicate/$targetPosition";
        }
        $constantPairUsedPredicate{$sourceConstant}{$targetConstant}{$targetPredicate} = 1;
      }
    }
  }
}

%mappingsByScore = ();

foreach $ 
sourceConstant (keys %mappingScores) {
  # print "for source $sourceConstant:\n";
  foreach $targetConstant (keys %{ $mappingScores{$sourceConstant} }) {
    # print " $targetConstant: $mappingScores{$sourceConstant}{$targetConstant}\n";
    push @{ $mappingsByScore{$mappingScores{$sourceConstant}{$targetConstant}} }, "$sourceConstant $targetConstant";
  }
}

%usedSourceConstants = ();
%usedTargetConstants = ();

foreach $score (sort {$b <=> $a} keys %mappingsByScore) {
  @usingSource = ();
  @usingTarget = ();
  foreach $mapping (@{ $mappingsByScore{$score} }) {
    $mapping =~ /(\S+) (\S+)/;
    $source = $1;
    $target = $2;

    if (not defined $usedSourceConstants{$source} and not defined $usedTargetConstants{$target}) {
      #  print "score $score: $source -> $target\n";
      #  print "used in $mappingTargetPredicates{$source}{$target}\n";
      print "map constant $source $target\n";
      push @usingSource, $source;
      push @usingTarget, $target;
    }
  }
  foreach (@usingSource) {
      $usedSourceConstants{$_} = 1;
  }
  foreach (@usingTarget) {
       $usedTargetConstants{$_} = 1;
  }
}

foreach $sourceConstant (keys %sourceConstantsToPPos) {
  if (not defined $usedSourceConstants{$sourceConstant}) {
    print "NO MAPPING for source constant $sourceConstant\n";
  }
}

foreach $targetConstant (keys %targetConstantsToPPos) {
  if (not defined $usedTargetConstants{$targetConstant}) {
    print "NO MAPPING for target constant $targetConstant\n";
  }
}

sub checkFor() {
  $file = shift;
  die "$file does not exist" unless (-e $file);
}

sub max() {
  $max =-100000;
  foreach (@_) {
    if ($_ > $max) {
      $max = $_;
    }
  }

  return $max;
}
