#!/usr/bin/perl

our $file = $ARGV[0];
if (not -e $file) {
  die "first arg is file\n";
}

$makeStatic = "./makeStatic.pl $file";
$fixFact = "./fixFact.pl $file";

print `echo "source mm-common.soar" >> $file`;
print `$makeStatic diag`;
print `$makeStatic nCell`;
print `$fixFact lessThan`;
