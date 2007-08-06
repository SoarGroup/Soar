#!/usr/bin/perl

die unless ($#ARGV == 2);
$environment = $ARGV[0];
$sourceKif = $ARGV[1];
$targetKif = $ARGV[2];

checkFor($sourceKif);
checkFor($targetKif);

$agentDir = "../agents";

$sourceKif =~ /([^\/]*)\.kif/;
$source = $1;

$targetKif =~ /([^\/]*)\.kif/;
$target = $1;

$source1Log = "$source\.log";
$targetWithSourceLog = "$target\_after_source.log";
$targetWithoutSourceLog = "$target\_no_source.log";

$goodThings = "$agentDir/goodthings.soar";

$genGT = "./genGoodThings.pl";

$runSoar = "./runSoar.py";

$buildKif = "./buildKif.pl";
$canvasOff = "./canvasOff.pl";

$gtOn = "./goodthingsOn.pl";
$gtOff = "./goodthingsOff.pl";

print "Building Soar agents from kif..\n";
#print `$buildKif $environment $sourceKif`;
print `$buildKif $environment $targetKif`;

checkFor("$agentDir/$source.soar");
checkFor("$agentDir/$target.soar");
#clearLog($source1Log);
clearLog($targetWithSourceLog);
clearLog($targetWithoutSourceLog);
clearLog($goodThings);

print `$canvasOff`;

#print `$gtOff`;
#print "Running $source..\n";
#print `$runSoar -w1 $agentDir/$source.soar > $source1Log`;
#lastDecision($source1Log);

print `$gtOn`;
print `touch $goodThings`;
print "Extracting goodThings..\n";
print `$genGT $source1Log $sourceKif $targetKif >> $goodThings`;
print "found this many goodThings:\n";
print `grep 'sp {' $goodThings | wc -l`;
print "found the following mappings:\n";
print `grep MAPPING $goodThings`;

print "Running $target with source..\n";
print `$runSoar -w1 $agentDir/$target.soar > $targetWithSourceLog`;
lastDecision($targetWithSourceLog);

print `$gtOff`;
print "Running $target without source..\n";
print `$runSoar -w1 $agentDir/$target.soar > $targetWithoutSourceLog`;
lastDecision($targetWithoutSourceLog);


sub clearLog() {
  $log = shift;
  if (-e $log) {
    print "$log exists, ctrl-c now or it gets deleted!\n";
    $foo = <STDIN>;
    print `rm $log`;
  }
}

sub checkFor() {
  $file = shift;
  die "$file does not exist" unless (-e $file);
}
sub lastDecision() {
  $file = shift;
  print "run done, last decision:\n";
  print `tail -n 100 $file | grep 'O:' | tail -n 1`;
}

