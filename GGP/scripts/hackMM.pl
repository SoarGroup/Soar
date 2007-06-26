#!/usr/bin/perl

our $file = $ARGV[0];
if (not -e $file) {
  die "first arg is file\n";
}

$makeStatic = "./makeStatic.pl $file";
$fixFact = "./fixFact.pl $file";
$fixVar = "./fixVar11.pl $file";

print `echo "source mm-common.soar" >> $file`;
print `$makeStatic diag`;
print `$makeStatic plus`;
print `$makeStatic minus`;
print `$makeStatic greaterthan`;
print `$makeStatic lessthan`;
print `$makeStatic gtequal`;
print `$makeStatic nsteps`;
print `$makeStatic stepblocked`;
print `$makeStatic cell`;
print `$makeStatic distinctcell`;
#print `$fixFact lessthan`;
print `$fixVar`;
