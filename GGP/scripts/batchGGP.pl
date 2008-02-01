#!/usr/bin/perl

our $remote = "./runRemoteScenarioNoSource.pl";

die unless ($#ARGV == 1);

our @all_machines = (
  "b1",
  "b2",
  "w1",
  "w2",
  "n1",
  "n2",
  "s1",
  "s2",
  "g",
  "a",
  "f",
  "bb",
  "r1",
  "r2"
);

our @scenarios_base = (
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
#  "r 6 1",
#  "r 6 2",
#  "r 6 3",
#  "r 8 1",
#  "r 8 2",
#  "r 9 1",
#  "r 9 2",
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

our @hard_base = (
  "b 7 1",
  "b 7 3",
#  "r 6 1",
#  "r 6 3"
);

$dirBase = "$ENV{HOME}/GGP-batches/$ARGV[0]";
$numRuns = int($ARGV[1]);

# repeat each scenario $numRuns times, in a separate directory
@scenarios = ();
@hard = ();
foreach $i (1 .. $numRuns) {
  $d = "$dirBase/$i";
  if (not -d $d) {
    print "creating $d\n";
    print `mkdir -p $d`;
  }
  @scenarios = (@scenarios, map { "$_ $i" } @scenarios_base);
  @hard = (@hard, map {"$_ $i"} @hard_base);
}

our @easy = ();

foreach $s (@scenarios) {
  unless (grep(/^$s$/, @hard)) {
    push(@easy, $s);
  }
}

our %doneScenarios = ();
foreach (@scenarios) {
  $doneScenarios{$_} = 0;
}

our %startedScenarios = ();
foreach (@scenarios) {
  $startedScenarios{$_} = 0;
}

our %scenarioMachines = ();
our %openMachines = ();

foreach (@all_machines) {
  $openMachines{$_} = 1;
}

our $parity = 0;
while (1) {
  @fast = get_machines("fast");
  @slow = get_machines("slow");
  @machines = (@fast, @slow);

  $doneCount = updateDoneScenarios();
  #print "$doneCount are done\n";
  if ($doneCount == ($#scenarios + 1)) {
    print "all runs complete.\n";
    exit(1);
  }

  # first, fill up all the fast machines with hard jobs
  FAST: foreach $m (@fast) {
    if ($openMachines{$m} != 1) {
      next;
    }
    foreach $s (@hard) {
      if ($startedScenarios{$s} == 0 and $doneScenarios{$s} == 0) {
        startScenario($s, $m);
        next FAST;
      }
    }
  }

  # now run the easy jobs on any free machine
  MACHINE: foreach $m (@machines) {
    if ($openMachines{$m} != 1) {
      next;
    }
    #print "finding a job for $machine\n";
    foreach $s (@easy) {
      #print "maybe $s? $startedScenarios{$s} $doneScenarios{$s}\n";
      if ($startedScenarios{$s} == 0 and $doneScenarios{$s} == 0) {
        startScenario($s, $m);
        next MACHINE;
      }
    }
  }

  close $REPORT;
  sleep (10);
  print ".";
}

sub startScenario() {
  my($s,$m) = @_;
  $s =~ /(.*) (\d+)$/;
  $scenario = $1;
  $dirn = $2;
  $rundir = "$dirBase/$dirn";
  print "starting " . expandScenario($s) . " on machine $m.\n";
  system("$remote $rundir $m $scenario > /dev/null 2>&1 &");
  $scenarioMachines{$s} = $m;
  $startedScenarios{$s} = 1;
  $openMachines{$m} = -1;
}

sub updateDoneScenarios() {
  $totalDone = 0;
  
  foreach $s (keys %doneScenarios) {
    #print "check $s\n";
    $file = expandScenario($s) . "-target_after_source.log";
    $tns = expandScenario($s) . "-target_no_source.log";
    if ($doneScenarios{$s} == 0) {
      if (-e $file) {
        if (-s $file > 0) {
          print expandScenario($s) ." is done. s/ns: ";
          print decisionCount($file) . "/" . decisionCount($tns) . "\n";

          if (defined $scenarioMachines{$s}) {
            $openMachines{$scenarioMachines{$s}} = 1;
            #print "open $scenarioMachines{$s}\n";
          }
          $doneScenarios{$s} = 1;
          $totalDone++;
        }
        else {
          # this scenario wasn't run correctly. Probably cancelled and should be restarted
          $startedScenarios{$s} = 0;
        }
      }
    }
    else {
      #print "$s was done before.\n";
      $totalDone++;
    }
  }

  if ($parity == 0) {
    open REPORT, ">batch-log-$$";
    print REPORT "batchGGP log\n";
    print REPORT `date`;
    print REPORT "logs are stored in $dirBase\n";
    foreach $scenario (@scenarios) {
      $fullScenario = expandScenario($scenario);
      $file = "$fullScenario-target_after_source.log";
      $tns = "$fullScenario-target_no_source.log";
      if ($doneScenarios{$scenario} == 1) {
        print REPORT "$fullScenario is done. s/ns: "; 
        print REPORT decisionCount($file) . "/" . decisionCount($tns) . "\n";
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
  if ($scen =~ /(\S+) (\d+) (\d+) (\d+)/) {
    $env = $1;
    $lvl = $2;
    $scn = $3;
    $count = $4;

    if ($1 =~ /e/) {
      return "$dirBase/$count/escape-$lvl-$scn";
    }
    elsif ($1 =~ /r/) {
      return "$dirBase/$count/mrogue-$lvl-$scn";
    }
    elsif ($1 =~ /w/) {
      return "$dirBase/$count/wargame-$lvl-$scn";
    }
    elsif ($1 =~ /b/) {
      return "$dirBase/$count/build-$lvl-$scn";
    }
    else {
      die;
    }
  }
  elsif ($scen =~ /10 (\d+) (\d+)/) {
    return "$dirBase/$2/differing-10-$1";
  }
  else {
    die;
  }
}
    
sub decisionCount() {
  $file = shift;
  if (-e $file) {
    $line = `tail -n 100 $file | grep 'O:' | tail -n 1`;
    $line =~ /(\d+):/;
    return $1;
  }
  else {
    return "***";
  }
}

sub get_machines() {
  $f = shift;
  @m = ();
  open(F, "< $f") or die;
  while (<F>) {
    chomp;
    push(@m, $_) if not /#/;
  }
  close(F);
  return @m;
}
