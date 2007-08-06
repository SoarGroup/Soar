#!/usr/bin/perl
use Cwd 'abs_path';
use File::stat;
use Time::localtime;

$path = abs_path($ARGV[0]);

die unless ($#ARGV == 0);
$logFile = $ARGV[0];
die unless (-e $logFile);

#print "last decision:\n";
lastDecision($logFile);
$date_string = ctime(stat($logFile)->ctime);
print "$date_string\n";

$machine = `uname -n`;
chomp $machine;
print "$machine:$path\n";

#print "total states visited: ";
print `grep update-state $logFile | wc -l`;

#print "soln length:\n";
print `grep ACTION $logFile | wc -l`;

#print "actions taken:\n";
print `grep ACTION $logFile`;


sub lastDecision() {
  $file = shift;
  $line = `tail -n 100 $file | grep 'O:' | tail -n 1`;
  $line =~ /(\d+).*/;
  print "$1\n";
}
