#!/usr/bin/perl


%machineAliases = ();
%machineDirs = ();

$machineAliases{"b1"} = "bahamut";
$machineDirs{"b1"} = "GGP/scripts";
$machineAliases{"b2"} = "bahamut";
$machineDirs{"b2"} = "2GGP/GGP/scripts";
$machineAliases{"b3"} = "bahamut";
$machineDirs{"b3"} = "3GGP/GGP/scripts";
$machineAliases{"w1"} = "wyrm";
$machineDirs{"w1"} = "GGP/scripts";
$machineAliases{"w2"} = "wyrm";
$machineDirs{"w2"} = "2GGP/GGP/scripts";
$machineAliases{"s1"} = "smaug";
$machineDirs{"s1"} = "GGP/scripts";
$machineAliases{"s2"} = "smaug";
$machineDirs{"s2"} = "2GGP/GGP/scripts";
$machineAliases{"r1"} = "141.212.109.197";
$machineDirs{"r1"} = "GGP/scripts";
$machineAliases{"r2"} = "141.212.109.197";
$machineDirs{"r2"} = "2GGP/GGP/scripts";
$machineAliases{"a"} = "auk";
$machineDirs{"a"} = "GGP/scripts";
$machineAliases{"g"} = "grapes";
$machineDirs{"g"} = "GGP/scripts";
$machineAliases{"bb"} = "badboy";
$machineDirs{"bb"} = "GGP/scripts";
$machineAliases{"f"} = "flamingo";
$machineDirs{"f"} = "GGP/scripts";

foreach $machine (keys %machineAliases) {
  print "updating $machineAliases{$machine}:$machineDirs{$machine}..\n";
  print `ssh $machineAliases{$machine} \"cd $machineDirs{$machine}; ./updateGGP.pl\"`;
}
