#!/usr/bin/perl

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
  $target = "../kif/escape/escape-$level-$scenario-target.kif";
}
elsif ($env =~ /^w/ or $env =~ /^m/) {
  $target = "../kif/mm/wargame-$level-$scenario-target.kif";
}
elsif ($env =~ /^r/) {
  $target = "../kif/rogue/mrogue-$level-$scenario-target.kif";
}
elsif ($level =~ "10") {
  $target = "../kif/level10/differing-10-$scenario-target.kif";
}
elsif ($env =~ /^b/) {
  $target = "../kif/build/build-$level-$scenario-target.kif";
}
else {die "bad environment";}

exec ("./runTarget.pl $target");
