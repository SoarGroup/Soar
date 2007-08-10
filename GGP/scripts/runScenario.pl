#!/usr/bin/perl

die unless ($#ARGV == 2);

$update = "./updateGGP.pl";
$runSTP = "./runSourceTargetPair.pl";
$runSSTP = "./runSourceSourceTargetPair.pl";

$env = $ARGV[0];
$level = $ARGV[1];
$scenario = $ARGV[2];

if ($env =~ /^e/) {
  $env = "escape";
  $source = "../kif/escape/escape-tl$level-$scenario-source.kif";
  $sourceOne = "../kif/escape/escape-tl$level-$scenario-source1.kif";
  $sourceTwo = "../kif/escape/escape-tl$level-$scenario-source2.kif";
  $target = "../kif/escape/escape-tl$level-$scenario-target.kif";
}
elsif ($env =~ /^w/ or $env =~ /^m/) {
  $env = "mm";
  $source = "../kif/mm/wargame-tl$level-$scenario-source.kif";
  $sourceOne = "../kif/mm/wargame-tl$level-$scenario-source1.kif";
  $sourceTwo = "../kif/mm/wargame-tl$level-$scenario-source2.kif";
  $target = "../kif/mm/wargame-tl$level-$scenario-target.kif";
}
elsif ($env =~ /^r/) {
  $env = "rogue";
  $source = "../kif/rogue/mRogue-TL-Level-$level-$scenario-Source.kif";
  $sourceOne = "../kif/rogue/mRogue-TL-Level-$level-$scenario-Source-1.kif";
  $sourceTwo = "../kif/rogue/mRogue-TL-Level-$level-$scenario-Source-2.kif";
  $target = "../kif/rogue/mRogue-TL-Level-$level-$scenario-Target.kif";
}
else {die "bad environment";}

#print `$update`;

if ($level == 6) {
  exec ("$runSSTP $env $sourceOne $sourceTwo $target");
}
else {
  exec ("$runSTP $env $source $target");
}
