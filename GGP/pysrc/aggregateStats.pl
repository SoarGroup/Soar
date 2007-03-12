#!/usr/bin/perl

@logs = sort @ARGV;

$scriptDir = "./pysrc";
# transfer-case-depth  base-target-prod transfer-target-prod time time sig

@baseDecisions = ();
@baseTimes = ();
@transferDecisions = ();
@transferTimes = ();
@transferDepths = ();
@signatures = ();
foreach $log (@logs) {

#  print "handling $log\n";

  if ($log =~ /source/) {
    next;
  }
  elsif ($log =~ /plain_target/) {
    $line = `grep decisions $log`;
    chomp $line;

    $line =~ /^(\d+) decisions/ or die "can't find number of decisions in $log";
    push @baseDecisions, $1;

    $line = `grep "CPU Time" $log | grep Total`;
    $line =~ /CPU Time:\s+(\S+) sec/ or die;
    push @baseTimes, $1;
  }
  elsif ($log =~ /transfer_target/) {
    $line = `grep decisions $log`;
    chomp $line;

    $line =~ /^(\d+) decisions/ or die;
    push @transferDecisions, $1;

    $line = `grep "CPU Time" $log | grep Total`;
    $line =~ /CPU Time:\s+(\S+) sec/ or die;
    push @transferTimes, $1;

    $line = `$scriptDir/getSolutionDepth.pl $log`;
    chomp $line;
    push @transferDepths, $line;

    $log =~ /^(\S+)_transfer_target/ or die;
    push @signatures, $1;
  }
}

if ($#transferDecisions != $#baseDecisions) {
  die "unequal numbers of base and transfer cases!\n";
}

@overall = ();
for($i=0; $i<$#transferDecisions; $i++) {
  $string = sprintf("%2d, %6d, %6d, %7.3f, %7.3f, %s",
                    $transferDepths[$i], $baseDecisions[$i], $transferDecisions[$i], 
                    $baseTimes[$i], $transferTimes[$i], $signatures[$i]);
  push @overall, $string;
}

@sorted = sort @overall;

foreach $line (@sorted) {
  print "$line\n";
}
