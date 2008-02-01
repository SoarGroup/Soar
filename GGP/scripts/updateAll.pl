#!/usr/bin/perl


%machineAliases = ();
%machineDirs = ();

$machineAliases{"b1"} = "bahamut";
$machineDirs{"b1"} = "GGP";
$machineAliases{"b2"} = "bahamut";
$machineDirs{"b2"} = "2GGP/GGP";
#$machineAliases{"b3"} = "bahamut";
#$machineDirs{"b3"} = "3GGP/GGP";
$machineAliases{"w1"} = "wyrm";
$machineDirs{"w1"} = "GGP";
$machineAliases{"w2"} = "wyrm";
$machineDirs{"w2"} = "2GGP/GGP";
$machineAliases{"s1"} = "smaug";
$machineDirs{"s1"} = "GGP";
$machineAliases{"s2"} = "smaug";
$machineDirs{"s2"} = "2GGP/GGP";
$machineAliases{"r1"} = "141.212.109.250";
$machineDirs{"r1"} = "GGP";
$machineAliases{"r2"} = "141.212.109.250";
$machineDirs{"r2"} = "GGP2";
$machineAliases{"a"} = "auk";
$machineDirs{"a"} = "GGP";
$machineAliases{"g"} = "grapes";
$machineDirs{"g"} = "GGP";
$machineAliases{"bb"} = "badboy";
$machineDirs{"bb"} = "GGP";
$machineAliases{"f"} = "flamingo";
$machineDirs{"f"} = "GGP";
#$machineAliases{"m1"} = "d-109.232";
#$machineDirs{"m1"} = "GGP";
#$machineAliases{"m2"} = "d-109.232";
#$machineDirs{"m2"} = "2GGP/GGP";
$machineAliases{"n1"} = "winter";
$machineDirs{"n1"} = "GGP";
$machineAliases{"n2"} = "winter";
$machineDirs{"n2"} = "2GGP/GGP";
#$machineAliases{"y1"} = "d-109.245";
#$machineDirs{"y1"} = "GGP";
#$machineAliases{"y2"} = "d-109.245";
#$machineDirs{"y2"} = "2GGP/GGP";

foreach $m (keys %machineAliases) {
  print "updating $machineAliases{$m}:$machineDirs{$m}..\n";
  #print `ssh $machineAliases{$m} \"cd $machineDirs{$m}; ./updateGGP.pl\"`;
  #print `ssh $machineAliases{$m} \"cd $machineDirs{$m}; rsync --recursive --exclude '*/.svn' wyrm:GGP-master/* ."`;
  print `rsync --recursive --exclude '*/.svn' ~/GGP-master/* $machineAliases{$m}:$machineDirs{$m}`;
}
