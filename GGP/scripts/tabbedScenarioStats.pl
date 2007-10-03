#!/usr/bin/perl

$env = $ARGV[0];
$level = $ARGV[1];
$scenario = $ARGV[2];

$stats = "./runStats.pl";

@regrets = `./scenarioRegret.pl $env $level $scenario`;

if ($env =~ "10") {
  # level 10 has no environment
  $scenario = $level;
  $level = 10;
  die unless ($#ARGV == 1);
}
else {
  die unless  ($#ARGV == 2);
}

@source = ();
@source1 = ();
@source2 = ();
@targetNS = ();
@targetS = ();

if ($env =~ /^e/) {
  if ($level == 6) {
    @source1 = `$stats escape-$level-$scenario-source-1.log`;
    @source2 = `$stats escape-$level-$scenario-source-2.log`;
  }
  else {
    @source1 = `$stats escape-$level-$scenario-source.log`;
  }
  @targetNS = `$stats escape-$level-$scenario-target_no_source.log`;
  @targetS = `$stats escape-$level-$scenario-target_after_source.log`;
}
elsif ($env =~ /^w/ or $env =~ /^m/) {
  if ($level == 6) {
    @source1 = `$stats wargame-$level-$scenario-source-1.log`;
    @source2 = `$stats wargame-$level-$scenario-source-2.log`;
  }
  else {
    @source1 = `$stats wargame-$level-$scenario-source.log`;
  }
  @targetNS = `$stats wargame-$level-$scenario-target_no_source.log`;
  @targetS = `$stats wargame-$level-$scenario-target_after_source.log`;
}
elsif ($env =~ /^r/) {
  if ($level == 6) {
    @source1 = `$stats mrogue-$level-$scenario-source-1.log`;
    @source2 = `$stats mrogue-$level-$scenario-source-2.log`;
  }
  else {
    @source1 = `$stats mrogue-$level-$scenario-source.log`;
  }
  @targetNS = `$stats mrogue-$level-$scenario-target_no_source.log`;
  @targetS = `$stats mrogue-$level-$scenario-target_after_source.log`;
}
elsif ($env =~ /^b/) {
  @source1 = `$stats build-$level-$scenario-source.log`;
  @targetNS = `$stats build-$level-$scenario-target_no_source.log`;
  @targetS = `$stats build-$level-$scenario-target_after_source.log`;
}
elsif ($level =~ "10") {
  @source1 = `$stats differing-10-$scenario-source.log`;
  @targetNS = `$stats differing-10-$scenario-target_no_source.log`;
  @targetS = `$stats differing-10-$scenario-target_after_source.log`;
}
else {die "bad environment";}

foreach (@source1) { chomp; }
foreach (@source2) { chomp; }
foreach (@targetNS) { chomp; }
@targetSWithRegret = ();
$i = 0;
foreach $line (@targetS) { 
  chomp $line; 
  if ($i == 8) {
    foreach $regret (@regrets) {
      # inject regret calculations
      chomp $regret;
      push @targetSWithRegret, $regret;
    }
  }
  push @targetSWithRegret, $line;
  $i++;
}

$maxIdx = max($#source1, $#source2, $#targetSWithRegret, $#targetNS);

for ($i=0; $i<=$maxIdx; $i++) {
  if ($i <= $#source1) {
    print "$source1[$i]";
  }
  else { print "."};
  print "\t";
  if ($level == 6) {
    if ($i <= $#source2) {
      print "$source2[$i]";
    }
    else { print "."};
    print "\t";
  }
  if ($i <= $#targetNS) {
    print "$targetNS[$i]";
  }
  else { print "."};
  print "\t";
  if ($i <= $#targetSWithRegret) {
    print "$targetSWithRegret[$i]";
  }
  else { print "."};
  print "\n";
}

sub max() {
  $val = -1;
  foreach $arg (@_) {
    if ($arg > $val) {
      $val = $arg;
    }
  }

  return $val;
}
