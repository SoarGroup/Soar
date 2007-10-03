#!/usr/bin/perl

$update = "./updateGGP.pl";
$runSTP = "./runSourceTargetPair.pl";
$runSSTP = "./runSourceSourceTargetPair.pl";

$env = $ARGV[0];
$level = $ARGV[1];
$scenario = $ARGV[2];

if ($env =~ "10") {
  # level 10 has no environment
  $scenario = $level;
  $level = 10;
  die unless ($#ARGV == 1);
}
else {
  die unless  ($#ARGV == 2);
}

if ($env =~ /^e/) {
  $source = "../kif/eval/escape/escape-$level-$scenario-source.kif";
  $sourceOne = "../kif/eval/escape/escape-$level-$scenario-source-1.kif";
  $sourceTwo = "../kif/eval/escape/escape-$level-$scenario-source-2.kif";
  $target = "../kif/eval/escape/escape-$level-$scenario-target.kif";
}
elsif ($env =~ /^w/ or $env =~ /^m/) {
  $source = "../kif/eval/mm/wargame-$level-$scenario-source.kif";
  $sourceOne = "../kif/eval/mm/wargame-$level-$scenario-source-1.kif";
  $sourceTwo = "../kif/eval/mm/wargame-$level-$scenario-source-2.kif";
  $target = "../kif/eval/mm/wargame-$level-$scenario-target.kif";
}
elsif ($env =~ /^r/) {
  $source = "../kif/eval/rogue/mrogue-$level-$scenario-source.kif";
  $sourceOne = "../kif/eval/rogue/mrogue-$level-$scenario-source-1.kif";
  $sourceTwo = "../kif/eval/rogue/mrogue-$level-$scenario-source-2.kif";
  $target = "../kif/eval/rogue/mrogue-$level-$scenario-target.kif";
}
elsif ($level =~ "10") {
  $source = "../kif/eval/level10/differing-10-$scenario-source.kif";
  $target = "../kif/eval/level10/differing-10-$scenario-target.kif";
}
elsif ($env =~ /^b/) {
  $source = "../kif/eval/build/build-$level-$scenario-source.kif";
  $sourceOne = "../kif/eval/build/build-$level-$scenario-source-1.kif";
  $sourceTwo = "../kif/eval/build/build-$level-$scenario-source-2.kif";
  $target = "../kif/eval/build/build-$level-$scenario-target.kif";
}
else {die "bad environment";}

#print `$update`;

if ($level == 6) {
  exec ("$runSSTP $sourceOne $sourceTwo $target");
}
else {
  exec ("$runSTP $source $target");
}
