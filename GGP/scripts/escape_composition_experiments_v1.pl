#!/usr/bin/perl

$agentDir = "../agents";
$source1 = "escape_composition_source1_v04";
$source2 = "escape_composition_source2_v04";
$target = "escape_composition_target_v04";

$source1Log = "$source1\.log";
$source2Log = "$source2\.log";
$targetLogWithSourceLog = "$target\_after_source.log";
$targetLogWithoutSourceLog = "$target\_no_source.log";

$goodThings = "$agentDir/goodthings.soar";
$goodThings = "$agentDir/goodthings_not_used_in_current_run.soar";

$genGT = "./genGoodThings.pl";

$runSoar = "./runSoar.py";

clearLog($source1Log);
clearLog($source2Log);
clearLog($targetLogWithSourceLog);
clearLog($targetLogWithoutSourceLog);
clearLog($goodThings);
clearLog($goodThingsBU);


print `touch $goodThings`;
print "Running $source2..\n";
print `$runSoar soar -l $agentDir/$source2.soar > $source2Log`;
print "done, log tail:\n";
print `tail $source2Log`;

print "Running $source1..\n";
print `$runSoar soar -l $agentDir/$source1.soar > $source1Log`;
print "done, log tail:\n";
print `tail $source1Log`;

print "Extracting goodThings..\n";
print `$genGT $source1Log >> $goodThings`;
print `$genGT $source2Log >> $goodThings`;
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


