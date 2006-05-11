#!/usr/bin/perl
use strict;

# mineSimulator.pl
# Sam Wintermute, 5/10/2006

our $mineTime = 30;
our $dropTime = 1;
our $simulationTime = 10000;
my $numMiners = 2;

my $maxPath = 125;
my $minPath = 75;

my $addonPath = 100;

my @mineDropDists;

our $numMinerals = 1;
our $numDrops = 15;

my @tempArr;
for (my $i=0; $i<$numMinerals; $i++) {
  @tempArr = ();
  for (my $j=0; $j<$numDrops; $j++) {
    # set the mine-drop distance randomly
    my $tmp = int(rand ($maxPath-$minPath+1)) + $minPath;
    push @tempArr, $tmp;
    if (not ($i==0 and $j==0)) { print "mdd[$i][$j] = $tmp\n"; }
  }
  push @mineDropDists, [@tempArr];
}

$mineDropDists[0][0] = $addonPath;
print "mdd[0][0] = $addonPath\n";
#print "mdd[1][2]= " .$mineDropDists[1][2]. "\n";

our @miners = ();
for (my $i=0; $i<$numMiners; $i++) {
  @tempArr = ();
  # miner[x][0] = mineral#
  # miner[x][1] = dropoff#
  # miner[x][2] = distance
  push @tempArr, int(rand($numMinerals));
  push @tempArr, int(rand($numDrops));
  push @tempArr, $mineDropDists[$tempArr[0]][$tempArr[1]];
  push @miners, [@tempArr];
}
print "base:\n";
my @results1 = runSim();
print "adding worker to route 0,0, pathlength $mineDropDists[0][0]:\n";
@tempArr= ();
push @tempArr, 0;
push @tempArr, 0;
push @tempArr, $mineDropDists[0][0];
push @miners, [@tempArr];
my @results2 = runSim();
my $pathlength = ($mineDropDists[0][0]);
# results arrays: rate, dUsage[0], mUsage[0]
print "RESULTS: Ud $results1[1]\n";
print "RESULTS: Um $results1[2]\n";
print "RESULTS: deltaM ";
print ($results2[0] - $results1[0]);
print "\n";
print "RESULTS: deltaUd ";
print ($results2[1] - $results1[1]);
print "\n";
print "RESULTS: deltaUm ";
print ($results2[2] - $results1[2]);
print "\n";
print "RESULTS: deltaMn ";
my $x = ($results2[0] - $results1[0])*$pathlength;
print $x;
print "\n";
print "RESULTS: deltaUdn ";
$x=($results2[1] - $results1[1])*$pathlength;
print $x;
print "\n";
print "RESULTS: deltaUmn ";
$x=($results2[2] - $results1[2])*$pathlength;
print $x;
print "\n";

sub runSim {
  my @mineralQueues = ();
  my @dropQueues = ();
  my @minerStates = ();

  my $s_wait_for_mineral = 1;
  my $s_moving_to_mineral = 2;
  my $s_mining = 3;
  my $s_moving_to_drop = 4;
  my $s_wait_for_drop = 5;
  my $s_drop = 6;

  my $miner;
  my $numMiners = $#miners + 1;
  for (my $i=0; $i<$numMiners; $i++) {
    # start each miner randomly on the path to the mineral
    my $progress = int(rand($miners[$i][2]+1));
    push @minerStates, [$s_moving_to_mineral, $progress];
  }

  my $minerals = 0;
  my @nextMineralQueues;
  my @nextDropQueues;
  my @nextMinerStates;
  my $mineralNum;
  my $dropNum;
  my $pathlength;
  my @mineralUsage = ();
  my @dropUsage = ();
  for (my $i=0; $i<$numMinerals; $i++) {
    push @mineralUsage, 0;
  }
  for (my $i=0; $i<$numDrops; $i++) {
    push @dropUsage, 0;
  }
  for (my $time=0; $time<$simulationTime; $time++) {
    @nextMineralQueues = @mineralQueues;
    @nextDropQueues = @dropQueues;
    @nextMinerStates = @minerStates;
    for (my $i=0; $i<$numMiners; $i++) {
      $mineralNum = $miners[$i][0];
      $dropNum = $miners[$i][1];
      $pathlength = $miners[$i][2];
#      print "m $mineralNum d $dropNum pl $pathlength\n";
      
      if ($minerStates[$i][0] == $s_wait_for_mineral) {
        # at mineral, waiting- start mining if it opened
        if ($mineralQueues[$mineralNum][0][1] == $i) {
          print "time: $time miner $i done waiting for mineral $mineralNum\n";
          $nextMinerStates[$i][0] = $s_mining;
          $nextMineralQueues[$mineralNum][0][0] = $mineTime;
          $mineralUsage[$mineralNum]++;
        }
      }
      elsif ($minerStates[$i][0] == $s_moving_to_mineral) {
        if ($minerStates[$i][1] == 0) {
          print "time: $time miner $i done moving to mineral $mineralNum\n";
          $nextMinerStates[$i][0] = $s_wait_for_mineral;
          push @{$nextMineralQueues[$mineralNum]}, [0, $i];
        }
        else {
          # on the path still
          $nextMinerStates[$i][1] = $minerStates[$i][1] - 1;
        }
      }
      elsif ($minerStates[$i][0] == $s_mining) {
        die unless ($mineralQueues[$mineralNum][0][1] == $i);
        if ($mineralQueues[$mineralNum][0][0] == 0) {
          # done mining
          print "time: $time miner $i done mining\n";
          $nextMinerStates[$i][0] = $s_moving_to_drop;
          $nextMinerStates[$i][1] = $pathlength + int (rand(11)) - 5;
          # die if negative plength
          die if ($nextMinerStates[$i][1] <= 0);
          shift @{$nextMineralQueues[$mineralNum]};
        }
        else {
          $mineralUsage[$mineralNum]++;
          $nextMineralQueues[$mineralNum][0][0] 
            = $mineralQueues[$mineralNum][0][0] - 1;
        }
      }
      elsif ($minerStates[$i][0] == $s_wait_for_drop) {
        # at drop, waiting- start if it opened
        if ($dropQueues[$dropNum][0][1] == $i) {
          print "time: $time miner $i done waiting for drop $dropNum\n";
          $nextMinerStates[$i][0] = $s_drop;
          $nextDropQueues[$dropNum][0][0] = $dropTime;
          $dropUsage[$dropNum]++;
        }
      }
      elsif ($minerStates[$i][0] == $s_moving_to_drop) {
        if ($minerStates[$i][1] == 0) {
          print "time: $time miner $i done moving to drop $dropNum\n";
          $nextMinerStates[$i][0] = $s_wait_for_drop;
          push @{$nextDropQueues[$dropNum]}, [0, $i];
        }
        else {
          # on the path still
          $nextMinerStates[$i][1] = $minerStates[$i][1] - 1;
        }
      }
      elsif ($minerStates[$i][0] == $s_drop) {
        die unless ($dropQueues[$dropNum][0][1] == $i);
        if ($dropQueues[$dropNum][0][0] == 0) {
          # done dropping
          print "time: $time miner $i done dropping\n";
          $nextMinerStates[$i][0] = $s_moving_to_mineral;
          $nextMinerStates[$i][1] = $pathlength + int (rand(11)) - 5;
          # die if negative plength
          die if ($nextMinerStates[$i][1] <= 0);
          shift @{$nextDropQueues[$dropNum]};
          # minerals delivered!
          $minerals++;
        }
        else {
          $dropUsage[$dropNum]++;
          $nextDropQueues[$dropNum][0][0] 
            = $dropQueues[$dropNum][0][0] - 1;
        }
      }
    }
    @mineralQueues = @nextMineralQueues;
    @dropQueues = @nextDropQueues;
  #  print "q0: $queue[0][0] $queue[0][1]\n";
    @minerStates = @nextMinerStates;
    
    #for ($i=1; $i<=$#queue; $i++) {
    #  $queue[$i][0]++;
    #  # increment wait times
    #}
  }

  my $rate = 100 * ($minerals / $simulationTime);
  my $dUsage =  100*(($dropUsage[0])/$simulationTime);
  my $mUsage =  100*(($mineralUsage[0])/$simulationTime);
  print "minerals: $minerals ticks: ";
  print $simulationTime;
  print " rate: $rate";
  print " for $numMiners miners.\n";
  for (my $i=0; $i<$numMinerals; $i++) {
    print "mineral $i usage: ";
    print 100*(($mineralUsage[$i])/$simulationTime);
    print "%\n";
  }
  for (my $i=0; $i<$numDrops; $i++) {
    print "dropoff $i usage: ";
    print 100*(($dropUsage[$i])/$simulationTime);
    print "%\n";
  }
  return ($rate, $dUsage, $mUsage);
}
