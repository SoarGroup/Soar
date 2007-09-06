#!/usr/bin/perl

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
elsif ($level =~ "10") {
  @source1 = `$stats TL-Level-10-$scenario-Source.log`;
  @targetNS = `$stats TL-Level-10-$scenario-Target_no_source.log`;
  @targetS = `$stats TL-Level-10-$scenario-Target_after_source.log`;
}
else {die "bad environment";}

foreach (@source1) { chomp; }
foreach (@source2) { chomp; }
foreach (@targetNS) { chomp; }
foreach (@targetS) { chomp; }

$maxIdx = max($#source1, $#source2, $#targetS, $#targetNS);

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
  if ($i <= $#targetS) {
    print "$targetS[$i]";
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
