#!/usr/bin/perl

die unless ($#ARGV == 0);

our @machines = (
  "b1",
#  "b2",
  "w1",
  "w2",
  "s1",
  "s2",
  "n1",
  "n2",
  "g",
  "a",
  "f",
  "bb"
);

our @scenarios = (
  "e 6 1",
  "e 6 2",
  "e 7 1",
  "e 7 2",
  "e 8 1",
  "e 8 2",
  "e 9 1",
  "w 6 1",
  "w 6 2",
  "w 7 1",
  "w 7 2",
  "w 8 1",
  "w 8 2",
  "w 9 1",
  "w 9 2",
  "r 6 1",
  "r 6 2",
  "r 6 3",
  "r 8 1",
  "r 8 2",
  "r 9 1",
  "r 9 2",
  "b 7 1",
  "b 7 2",
  "b 7 3",
  "b 8 1",
  "10 1",
  "10 2",
  "10 3",
  "10 4",
  "10 5",
  "10 6",
  "10 7",
  "10 8",
  "10 11",
  "10 12",
  "10 13",
  "10 14",
  "10 15",
  "10 16",
  "10 17",
  "10 18"
);

$runName = $ARGV[0];

our $runDir = "$ENV{HOME}/GGP-batches/$runName";
if (not -d "$runDir") {
  print "creating $runDir\n";
  print `mkdir $runDir`;
}

our %doneScenarios = ();
foreach $scenario (@scenarios) {
  $doneScenarios{$scenario} = 0;
}

our %startedScenarios = ();
foreach $scenario (@scenarios) {
  $startedScenarios{$scenario} = 0;
}


our %scenarioMachines = ();
our %openMachines = ();

foreach $machine (@machines) {
  $openMachines{$machine} = 1;
}

our $parity = 0;
while (1) {
  $doneCount = updateDoneScenarios();
  #print "$doneCount are done\n";
  if ($doneCount == ($#scenarios + 1)) {
    print "all runs complete.\n";
    exit(1);
  }

  MACHINE: foreach $machine (keys %openMachines) {
    if ($openMachines{$machine} != 1) {
      next;
    }
    #print "finding a job for $machine\n";
    for ($i=0; $i<=$#scenarios; $i++) {
      $s = $scenarios[$i];
      #print "maybe $s? $startedScenarios{$s} $doneScenarios{$s}\n";
      if ($startedScenarios{$s} == 0 and $doneScenarios{$s} == 0) {
        print "starting " . expandScenario($s) . " on machine $machine.\n";
        system("./runRemoteScenario_headless.pl $runDir $machine $s > /dev/null 2>&1 &");
        $scenarioMachines{$s} = $machine;
        $startedScenarios{$s} = 1;
        $openMachines{$machine} = -1;
        next MACHINE;
      }
    }
  }

  close $REPORT;
  sleep (10);
  print ".";
}

sub updateDoneScenarios() {
  $totalDone = 0;
  
  foreach $s (keys %doneScenarios) {
    #print "check $s\n";
    $file = expandScenario($s) . "-target_after_source.log";
    $tns = expandScenario($s) . "-target_no_source.log";
    if ($doneScenarios{$s} == 0) {
      if (-e "$runDir/$file") {
        print expandScenario($s) ." is done. s/ns: ";
        print decisionCount("$runDir/$file") . "/" . decisionCount("$runDir/$tns") . "\n";
        
        if (defined $scenarioMachines{$s}) {
          $openMachines{$scenarioMachines{$s}} = 1;
          #print "open $scenarioMachines{$s}\n";
        }
        $doneScenarios{$s} = 1;
        $totalDone++;
      }
    }
    else {
      #print "$s was done before.\n";
      $totalDone++;
    }
  }

  if ($parity == 0) {
    open REPORT, ">batch-log";
    print REPORT "batchGGP log\n";
    print REPORT `date`;
    print REPORT "logs are stored in $runDir\n";
    foreach $scenario (@scenarios) {
      $fullScenario = expandScenario($scenario);
      $file = "$fullScenario-target_after_source.log";
      $tns = "$fullScenario-target_no_source.log";
      if ($doneScenarios{$scenario} == 1) {
        print REPORT "$fullScenario is done. s/ns: "; 
        print REPORT decisionCount("$runDir/$file") . "/" . decisionCount("$runDir/$tns") . "\n";
      }
      elsif ($startedScenarios{$scenario} == 1) {
        print REPORT "$fullScenario is running on $scenarioMachines{$scenario}.\n";
      }
      else {
        print REPORT "$fullScenario has not started.\n";
      }
    }
    close REPORT;
  }
  $parity++;
  if ($parity == 3) {
    $parity = 0;
  }

  return $totalDone;
}

sub expandScenario() {
  $scen = shift;
  if ($scen =~ /(\S+) (\d+) (\d+)/) {
    $env = $1;
    $lvl = $2;
    $scn = $3;

    if ($1 =~ /e/) {
      return "escape-$lvl-$scn";
    }
    elsif ($1 =~ /r/) {
      return "mrogue-$lvl-$scn";
    }
    elsif ($1 =~ /w/) {
      return "wargame-$lvl-$scn";
    }
    elsif ($1 =~ /b/) {
      return "build-$lvl-$scn";
    }
    else {
      die;
    }
  }
  elsif ($scen =~ /10 (\d+)/) {
    return "differing-10-$1";
  }
  else {die;}
}
    
sub decisionCount() {
  $file = shift;
  $line = `tail -n 100 $file | grep 'O:' | tail -n 1`;
  $line =~ /(\d+):/;
  return $1;
}
