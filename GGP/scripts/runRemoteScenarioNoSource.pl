#!/usr/bin/perl

#@expected_logs = ("target_no_source.log", "target_after_source.log");
@expected_logs = ("target_after_source.log");

$dir = $ARGV[0];
$machine = $ARGV[1];
$env = $ARGV[2];
$level = $ARGV[3];
$num = $ARGV[4];

if ($env =~ "10") {
  # level 10 has no environment
  $env = "";
  $num = $level;
  $level = 10;
  die unless ($#ARGV == 3);
}
else {
  die unless  ($#ARGV == 4);
}

$runScenario = "./runScenarioNoSource.pl";

%machineAliases = ();
%machineDirs = ();

$machineAliases{"b1"} = "bahamut";
$machineDirs{"b1"} = "GGP/scripts";
$machineAliases{"b"} = "bahamut";
$machineDirs{"b"} = "GGP/scripts";
$machineAliases{"b2"} = "bahamut";
$machineDirs{"b2"} = "2GGP/GGP/scripts";
$machineAliases{"b3"} = "bahamut";
$machineDirs{"b3"} = "2GGP/GGP/scripts";
$machineAliases{"w1"} = "wyrm";
$machineDirs{"w1"} = "GGP/scripts";
$machineAliases{"w"} = "wyrm";
$machineDirs{"w"} = "GGP/scripts";
$machineAliases{"w2"} = "wyrm";
$machineDirs{"w2"} = "2GGP/GGP/scripts";
$machineAliases{"s1"} = "smaug";
$machineDirs{"s1"} = "GGP/scripts";
$machineAliases{"s"} = "smaug";
$machineDirs{"s"} = "GGP/scripts";
$machineAliases{"s2"} = "smaug";
$machineDirs{"s2"} = "2GGP/GGP/scripts";
$machineAliases{"r1"} = "141.212.109.250";
$machineDirs{"r1"} = "GGP/scripts";
$machineAliases{"r2"} = "141.212.109.250";
$machineDirs{"r2"} = "GGP2/scripts";
$machineAliases{"a"} = "auk";
$machineDirs{"a"} = "GGP/scripts";
$machineAliases{"g"} = "grapes";
$machineDirs{"g"} = "GGP/scripts";
$machineAliases{"bb"} = "badboy";
$machineDirs{"bb"} = "GGP/scripts";
$machineAliases{"f"} = "flamingo";
$machineDirs{"f"} = "GGP/scripts";
$machineAliases{"m1"} = "d-109.232";
$machineDirs{"m1"} = "GGP/scripts";
$machineAliases{"m2"} = "d-109.232";
$machineDirs{"m2"} = "2GGP/GGP/scripts";
$machineAliases{"n1"} = "winter";
$machineDirs{"n1"} = "GGP/scripts";
$machineAliases{"n2"} = "winter";
$machineDirs{"n2"} = "2GGP/GGP/scripts";
$machineAliases{"y1"} = "d-109.245";
$machineDirs{"y1"} = "GGP/scripts";
$machineAliases{"y2"} = "d-109.245";
$machineDirs{"y2"} = "2GGP/GGP/scripts";

die unless (defined $machineAliases{$machine});

if ($env eq "e") {
  $longEnv = "escape";
}
elsif ($env eq "w") {
  $longEnv = "wargame";
}
elsif ($env eq "r") {
  $longEnv = "mrogue";
}
elsif ($env eq "b") {
  $longEnv = "build";
}
else {
  $longEnv = "differing";
}

# log the current run so that we can kill it easily
open STAT, '>>/tmp/GGP-batch-runs';
print STAT "$$ $env $level $num\n";
close STAT;

$cmd = "ssh $machineAliases{$machine} \"cd $machineDirs{$machine}; $runScenario $env $level $num\"";
system($cmd);

$scenario_name = "$longEnv-$level-$num";
$remote_file_base = "$machineAliases{$machine}:$machineDirs{$machine}/$scenario_name";
$local_file_base = "$dir/$scenario_name";

if ($?) {
  # error occurred somewhere
  print "#$#$#$#$# WARNING: $cmd exited abnormally\n";
  open ERR, '>>/tmp/GGP-errors';
  print ERR "$cmd exited abnormally\n";
  close ERR;
  # copy over logs with .err appended to names
  @log_copy_cmds = map {"scp $remote_file_base-$_ $local_file_base-$_.err"} @expected_logs;
}
else {
  # copy over logs normally
  @log_copy_cmds = map {"scp $remote_file_base-$_ $local_file_base-$_"} @expected_logs;
}

foreach (@log_copy_cmds) {
  system($_);
}
