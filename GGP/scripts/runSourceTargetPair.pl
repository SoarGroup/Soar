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
$targetLogWithSourceLog = "$target\_after_source.log";
$targetLogWithoutSourceLog = "$target\_no_source.log";

$goodThings = "$agentDir/goodthings.soar";
$goodThingsBU = "$agentDir/goodthings_not_used_in_current_run.soar";

$genGT = "./genGoodThings.pl";

$runSoar = "./runSoar.py";

$buildKif = "./buildKif.pl";
$canvasOff = "./canvasOff.pl";

print "Building Soar agents from kif..\n";
print `$buildKif $environment $sourceKif`;
print `$buildKif $environment $targetKif`;

checkFor("$agentDir/$source.soar");
checkFor("$agentDir/$target.soar");
clearLog($source1Log);
clearLog($targetLogWithSourceLog);
clearLog($targetLogWithoutSourceLog);
clearLog($goodThings);
clearLog($goodThingsBU);

print `$canvasOff`;

print `touch $goodThings`;
print "Running $source..\n";
print `$runSoar soar -l $agentDir/$source.soar > $source1Log`;
print "done, log tail:\n";
print `tail $source1Log`;

print "Extracting goodThings..\n";
print `$genGT $source1Log >> $goodThings`;
print "found this many goodThings:\n";
print `grep 'sp {' $goodThings | wc -l`;

print "Running $target with source..\n";
print `$runSoar -l $agentDir/$target.soar > $targetLogWithSourceLog`;
print "done, log tail:\n";
print `tail $targetLogWithSourceLog`;

print `mv $goodThings $goodThingsBU`;
print `touch $goodThings`;
print "Running $target without source..\n";
print `$runSoar soar -l $agentDir/$target.soar > $targetLogWithoutSourceLog`;
print "done, log tail:\n";
print `tail $targetLogWithoutSourceLog`;
print `mv $goodThingsBU $goodThings`;

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
