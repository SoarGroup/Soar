#!/usr/bin/perl

die unless ($#ARGV == 2);

our $kifFile = $ARGV[0];
if (not -e $kifFile) {
  die;
}

our $soarFile = $ARGV[1];
if (not -e $soarFile) {
  die;
}

$env = $ARGV[2];

$makeStatic = "./makeStatic.pl $soarFile";
$fixVar = "./fixVar11.pl $soarFile";
$findStatic = "./findStaticElaborations.pl $kifFile";

print `echo "source $env-common.soar" >> $soarFile`;
print `$makeStatic greaterthan`;
print `$makeStatic lessthan`;
print `$makeStatic gtequal`;
print `$makeStatic plus`;
print `$makeStatic minus`;
print `$fixVar`;

foreach $line (`$findStatic`) {
  chomp $line;
  $line = lc($line);
  print "static elaboration: $line\n";
  print `$makeStatic $line`;
}
