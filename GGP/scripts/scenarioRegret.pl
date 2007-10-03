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
  if ($level == 6) {
    $source1 = "escape-$level-$scenario-source-1.log";
    $source2 = "escape-$level-$scenario-source-2.log";
  }
  else {
    $source1 = "escape-$level-$scenario-source.log";
  }
  $targetNS = "escape-$level-$scenario-target_no_source.log";
  $targetS = "escape-$level-$scenario-target_after_source.log";
}
elsif ($env =~ /^w/ or $env =~ /^m/) {
  if ($level == 6) {
    $source1 = "wargame-$level-$scenario-source-1.log";
    $source2 = "wargame-$level-$scenario-source-2.log";
  }
  else {
    $source1 = "wargame-$level-$scenario-source.log";
  }
  $targetNS = "wargame-$level-$scenario-target_no_source.log";
  $targetS = "wargame-$level-$scenario-target_after_source.log";
}
elsif ($env =~ /^r/) {
  if ($level == 6) {
    $source1 = "mrogue-$level-$scenario-source-1.log";
    $source2 = "mrogue-$level-$scenario-source-2.log";
  }
  else {
    $source1 = "mrogue-$level-$scenario-source.log";
  }
  $targetNS = "mrogue-$level-$scenario-target_no_source.log";
  $targetS = "mrogue-$level-$scenario-target_after_source.log";
}
elsif ($env =~ /^b/) {
  $source1 = "build-$level-$scenario-source.log";
  $targetNS = "build-$level-$scenario-target_no_source.log";
  $targetS = "build-$level-$scenario-target_after_source.log";
}
elsif ($level =~ "10") {
  $source1 = "differing-10-$scenario-source.log";
  $targetNS = "differing-10-$scenario-target_no_source.log";
  $targetS = "differing-10-$scenario-target_after_source.log";
}
else {die "bad environment";}

if (defined $source2) {
  $goodthings = $source2;
}
else {
  $goodthings = $source1;
}

$goodthings =~ s/\.log/\.goodthings.soar/;
die unless (-e $goodthings);

$genUserTime = 0;
$genSysTime = 0;
foreach $line (`grep 'GEN TIME' $goodthings`) {
  chomp $line;
  $line =~ /,(\S+?) user,(\S+?) sys/;
  $genUserTime += $1;
  $genSysTime += $2;
}

$nsTimeLine = `grep 'UNIX TIME' $targetNS`;
$sTimeLine = `grep 'UNIX TIME' $targetS`;

$nsTimeLine =~ /,(\S+?) user,(\S+?) sys/ or die "!$nsTimeLine";
$nsUser = $1;
$nsSys = $2;

$sTimeLine =~ /,(\S+?) user,(\S+?) sys/ or die "!$sTimeLine";
$sUser = $1;
$sSys = $2;

$nsSoarLine = `grep 'decisions (' $targetNS`;
$sSoarLine = `grep 'decisions (' $targetS`;

$nsSoarLine =~ /^(\d+) decisions \((\S+) msec/ or die;
$nsSoarTime = $1*$2*0.001;

$sSoarLine =~ /^(\d+) decisions \((\S+) msec/ or die;
$sSoarTime = $1*$2*0.001;

print "$nsUser\n";
print $nsUser + $nsSys . "\n";
print $sUser + $genUserTime. "\n";
print $sUser + $sSys + $genUserTime + $genSysTime. "\n";

print regret($nsSoarTime, $sSoarTime + $genUserTime) . "\n";
print regret($nsUser, $sUser + $genUserTime) . "\n";
print regret($nsUser + $nsSys, $sUser + $genUserTime + $sSys + $genSysTime) . "\n";

sub regret() {
  $ns = shift;
  $s = shift;

  $denominator = max($ns, $s);
  $numerator = ($ns - $s)*100.0;
  
  return $numerator / $denominator;
}

sub max() {
  $max = -1;
  foreach (@_) {
    if ($_ > $max) {
      $max = $_;
    }
  }
  return $max;
}
