#!/usr/bin/perl

our $file = $ARGV[0];
if (not -e $file) {
  die "first arg is file\n";
}

$makeStatic = "./makeStatic.pl $file";
$fixFact = "./fixFact.pl $file";
$fixVar = "./fixVar11.pl $file";

print `echo "source escape-common.soar" >> $file`;
print `$fixVar`;
