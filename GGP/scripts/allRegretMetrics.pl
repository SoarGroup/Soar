#!/usr/bin/perl

foreach $line (`cat $ARGV[0]`) {
  unless ($line =~ /target_after_source.log/) {
    next;
  }
  $line =~ /^(.*\/)(\w+)-(\d+)-(\d+)-target_after_source.log/ or die "!$line";
  $path = $1;
  $env = $2;
  $level = $3;
  $scenario = $4;
  $source2 = ();

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
  elsif ($env =~ /^w/) {
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
  elsif ($env =~ /^m/) {
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
  elsif ($env =~ /^d/) {
    $source1 = "differing-10-$scenario-source.log";
    $targetNS = "differing-10-$scenario-target_no_source.log";
    $targetS = "differing-10-$scenario-target_after_source.log";
  }

  $source1 = $path . $source1;
  $source2 = $path . $source2;
  $targetNS = $path . $targetNS;
  $targetS = $path . $targetS;
  die unless (-e $source1);
  die unless (-e $targetNS);
  die unless (-e $targetS);

  if (defined $source2) {
    $goodthings = $source2;
    die "$source2 not found" unless (-e $source2);
  }
  else {
    $goodthings = $source1;
  }

  $goodthings =~ s/\.log/\.goodthings.soar/;
  die unless (-e $goodthings);
  
  $targetSsub = $targetS;
  $targetSsub =~ s/log$/submit/;
  die unless (-e $targetSsub);
  @targetSsubStats = `cat $targetSsub`;
  
  $targetNSsub = $targetNS;
  $targetNSsub =~ s/log$/submit/;
  die unless (-e $targetNSsub);
  @targetNSsubStats = `cat $targetNSsub`;

  $genUserTime = 0;
  $genSysTime = 0;
  $genRealTime = 0;
  foreach $line (`grep 'GEN TIME' $goodthings`) {
    chomp $line;
    $line =~ /(\S+?) real,(\S+?) user,(\S+?) sys/ or die "!$result";
    $real = $1;
    $genUserTime += $2;
    $genSysTime += $3;
    $genRealTime = realSeconds($real);
  }

  $nsTimeLine = `grep 'UNIX TIME' $targetNS`;
  $sTimeLine = `grep 'UNIX TIME' $targetS`;

  $nsTimeLine =~ /(\S+?) real,(\S+?) user,(\S+?) sys/ or die "!$result";
  $nsReal = $1;
  $nsUser = $2;
  $nsSys = $3;
  $nsReal = realSeconds($nsReal);

  $nsSubUser = $targetNSsubStats[1];
  $nsSubUserSys = $targetNSsubStats[2];
  $nsSubReal = $targetNSsubStats[3];

  $sTimeLine =~ /(\S+?) real,(\S+?) user,(\S+?) sys/ or die "!$result";
  $sReal = $1;
  $sUser = $2;
  $sSys = $3;
  $sReal = realSeconds($sReal);
  
  $sSubUser = $targetSsubStats[1];
  $sSubUserSys = $targetSsubStats[2];
  $sSubReal = $targetSsubStats[3];
  
  $nsSoarLine = `grep 'decisions (' $targetNS`;
  $sSoarLine = `grep 'decisions (' $targetS`;

  $nsSoarLine =~ /^(\d+) decisions \((\S+) msec/ or die;
  $nsSoarTime = $1*$2*0.001;

  $sSoarLine =~ /^(\d+) decisions \((\S+) msec/ or die;
  $sSoarTime = $1*$2*0.001;

  push @{$line[0]}, "$env-$level-$scenario";
  # four versions of no-source time
  push @{$line[1]}, $nsSoarTime;
  push @{$line[2]}, $nsUser;
  push @{$line[3]}, $nsUser + $nsSys;
  push @{$line[4]}, $nsReal;
  # four version of source time
  push @{$line[5]}, $sSoarTime + $genUserTime;
  push @{$line[6]}, $sUser + $genUserTime;
  push @{$line[7]}, $sUser + $sSys + $genUserTime + $genSysTime;
  push @{$line[8]}, $sReal + $genRealTime;
  # four versions of no-source time, including submission
  push @{$line[9]}, $nsSoarTime + $nsSubUser;
  push @{$line[10]}, $nsUser + $nsSubUser;
  push @{$line[11]}, $nsUser + $nsSys + $nsSubUserSys;
  push @{$line[12]}, $nsReal + $nsSubReal;
  # four versions of source time, including submission
  push @{$line[13]}, $sSoarTime + $genUserTime + $sSubUser;
  push @{$line[14]}, $sUser + $genUserTime + $sSubUser;
  push @{$line[15]}, $sUser + $sSys + $genUserTime + $genSysTime + $sSubUserSys;
  push @{$line[16]}, $sReal + $genRealTime + $sSubReal;
  # four versions of regret
  push @{$line[17]}, regret($nsSoarTime, $sSoarTime + $genUserTime);
  push @{$line[18]}, regret($nsUser, $sUser + $genUserTime);
  push @{$line[19]}, regret($nsUser + $nsSys, $sUser + $genUserTime + $sSys + $genSysTime);
  push @{$line[20]}, regret($nsReal, $sReal + $genRealTime);
  # four versions of regret, including submission
  push @{$line[21]}, regret($nsSoarTime + $nsSubUser, $sSoarTime + $genUserTime + $sSubUser);
  push @{$line[22]}, regret($nsUser + $nsSubUser, $sUser + $genUserTime + $sSubUser);
  push @{$line[23]}, regret($nsUser + $nsSys + $nsSubUserSys, $sUser + $genUserTime + $sSys + $genSysTime + $sSubUserSys);
  push @{$line[24]}, regret($nsReal + $nsSubReal, $sReal + $genRealTime + $sSubReal);
}

for ($i=0; $i<=24; $i++) {
  for ($j=0; $j<=$#{$line[1]}; $j++) {
    print  "$line[$i][$j]\t";
  }
  print "\n";
}


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

sub realSeconds() {
  $string = shift;
  if ($string =~ /(\d+):(\d+):(\d+)/) {
    return 3600*$1 + 60*$2 + $3;
  }
  elsif ($string =~ /(\d+):(\S+)/) {
    return 60*$1 + $2;
  }
  die;
}
