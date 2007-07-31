#!/usr/bin/perl

die unless ($#ARGV == 3);
$environment = $ARGV[0];
$source1Kif = $ARGV[1];
$source2Kif = $ARGV[2];
$targetKif = $ARGV[3];

checkFor($source1Kif);
checkFor($source2Kif);
checkFor($targetKif);

$agentDir = "../agents";

$source1Kif =~ /([^\/]*)\.kif/;
$source1 = $1;

$source2Kif =~ /([^\/]*)\.kif/;
$source2 = $1;

$targetKif =~ /([^\/]*)\.kif/;
$target = $1;

$source1Log = "$source1\.log";
$source2Log = "$source2\.log";
$targetLogWithSourceLog = "$target\_after_source.log";
$targetLogWithoutSourceLog = "$target\_no_source.log";

$goodThings = "$agentDir/goodthings.soar";
$goodThingsBU = "$agentDir/goodthings_not_used_in_current_run.soar";

$genGT = "./genGoodThings.pl";

$runSoar = "./runSoar.py";

$buildKif = "./buildKif.pl";
$canvasOff = "./canvasOff.pl";

print "Building Soar agents from kif..\n";
print `$buildKif $environment $source1Kif`;
print `$buildKif $environment $source2Kif`;
print `$buildKif $environment $targetKif`;

checkFor("$agentDir/$source1.soar");
checkFor("$agentDir/$source2.soar");
checkFor("$agentDir/$target.soar");
clearLog($source1Log);
clearLog($source2Log);
clearLog($targetLogWithSourceLog);
clearLog($targetLogWithoutSourceLog);
clearLog($goodThings);
clearLog($goodThingsBU);

print `$canvasOff`;

print `touch $goodThings`;
print "Running $source1..\n";
print `$runSoar -w1 $agentDir/$source1.soar > $source1Log`;
print "done, log tail:\n";
print `tail $source1Log`;

print "Running $source2..\n";
print `$runSoar -w1 $agentDir/$source2.soar > $source2Log`;
print "done, log tail:\n";
print `tail $source2Log`;

print "Extracting goodThings..\n";
print `$genGT $source1Log $source1Kif $targetKif >> $goodThings`;
print "found this many goodThings:\n";
print `grep 'sp {' $goodThings | wc -l`;
print `$genGT $source2Log $source2Kif $targetKif >> $goodThings`;
print "found this many goodThings:\n";
print `grep 'sp {' $goodThings | wc -l`;
print "found the following mappings:\n";
print `grep MAPPING $goodThings`;

print "Running $target with source..\n";
print `$runSoar -w1 $agentDir/$target.soar > $targetLogWithSourceLog`;
print "done, log tail:\n";
print `tail $targetLogWithSourceLog`;

print `mv $goodThings $goodThingsBU`;
print `touch $goodThings`;
print "Running $target without source..\n";
print `$runSoar -w1 $agentDir/$target.soar > $targetLogWithoutSourceLog`;
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
