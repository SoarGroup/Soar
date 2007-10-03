#!/usr/bin/perl
use Cwd 'abs_path';
use File::stat;
use Time::localtime;

$path = abs_path($ARGV[0]);

die unless ($#ARGV == 0);
$logFile = $ARGV[0];
die unless (-e $logFile);

$gtFile = $logFile;

$gtFile =~ s/log/goodthings.soar/;

#print "last decision:\n";
lastDecision($logFile);
$date_string = ctime(stat($logFile)->ctime);
print "$date_string\n";

$machine = `uname -n`;
chomp $machine;
print "$machine:$path\n";

#print "total states visited: ";
print `grep update-state $logFile | wc -l`;

$line = `grep "msec/decision" $logFile | tail -n 1`;
$line =~ /\((.*) msec/;
print "$1\n";

if (-e $gtFile) {
  $time = 0;
  foreach $line (`grep 'GEN TIME' $gtFile`) {
    chomp $line;
    $line =~ /,(\S+?) user,/;
    $time += $1;
  }
  print "$time\n";
}
else {
  print "no indicators generated\n";
}

#$time = `grep 'UNIX TIME' $logFile`;
#chomp $time;
#$time =~ /,(\S+?) user,/;
#$time = $1;
#print "$time\n";

print `svn info | grep Revision`;
#print `./GDLDebuggerScore.pl $logFile`;
print "score not checked.\n";

#print "soln length:\n";
$length = `grep ACTION $logFile | wc -l`;
chomp $length;

if ($length =~ /^\D*0\D*$/) {
  print `grep -A 2 increment $logFile | tail -n 1`;
  print `grep increment $logFile`;
}
elsif (`tail $logFile | grep fail`) {
  print "FAILED\n";
}
else {
  print "$length\n";
}
#print "actions taken:\n";
print `grep ACTION $logFile`;


sub lastDecision() {
  $file = shift;
  $line = `tail -n 100 $file | grep 'O:' | tail -n 1`;
  $line =~ /(\d+).*/;
  print "$1\n";
}
