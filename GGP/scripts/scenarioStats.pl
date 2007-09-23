#!/usr/bin/perl

$update = "./updateGGP.pl";
$runSTP = "./runsourcetargetPair.pl";
$runSSTP = "./runsourcesourcetargetPair.pl";

$env = $ARGV[0];
$level = $ARGV[1];
$scenario = $ARGV[2];

$stats = "./runStats.pl";

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
  if ($level == 6) {
    print `$stats escape-$level-$scenario-source-1.log`;
    print "\n";
    print `$stats escape-$level-$scenario-source-2.log`;
  }
  else {
    print `$stats escape-$level-$scenario-source.log`;
  }
  print "\n";
  print `$stats escape-$level-$scenario-target_no_source.log`;
  print "\n";
  print `$stats escape-$level-$scenario-target_after_source.log`;
}
elsif ($env =~ /^w/ or $env =~ /^m/) {
  if ($level == 6) {
    print `$stats wargame-$level-$scenario-source-1.log`;
    print "\n";
    print `$stats wargame-$level-$scenario-source-2.log`;
  }
  else {
    print `$stats wargame-$level-$scenario-source.log`;
  }
  print "\n";
  print `$stats wargame-$level-$scenario-target_no_source.log`;
  print "\n";
  print `$stats wargame-$level-$scenario-target_after_source.log`;
}
elsif ($env =~ /^r/) {
  $env = "rogue";
  if ($level == 6) {
    print `$stats mrogue-$level-$scenario-source-1.log`;
    print "\n";
    print `$stats mrogue-$level-$scenario-source-2.log`;
  }
  else {
    print `$stats mrogue-$level-$scenario-source.log`;
  }
  print "\n";
  print `$stats mrogue-$level-$scenario-target_no_source.log`;
  print "\n";
  print `$stats mrogue-$level-$scenario-target_after_source.log`;
}
elsif ($level =~ "10") {
  print `$runStats differing-10-$scenario-source.log`;
  print "\n";
  print `$runStats differing-10-$scenario-target_no_source.log`;
  print "\n";
  print `$runStats differing-10-$scenario-target_after_source.log`;
}
else {die "bad environment";}

