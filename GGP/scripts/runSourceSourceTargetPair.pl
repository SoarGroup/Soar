#!/usr/bin/perl

die unless ($#ARGV == 2);
$source1Kif = $ARGV[0];
$source2Kif = $ARGV[1];
$targetKif = $ARGV[2];

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
$targetWithSourceLog = "$target\_after_source.log";
$targetWithoutSourceLog = "$target\_no_source.log";

$goodThings = "$agentDir/goodthings.soar";
$mappingFile1 = "tmp.mappings1";
$mappingFile2 = "tmp.mappings2";
$timeFile = "tmp.time";
$timeCommand = '/usr/bin/time -o ' . $timeFile . ' -f \'%E real,%U user,%S sys\' ';

$genGT = "./genGoodThings.pl";

$runSoar = "./runSoar.py";

$buildKif = "./buildKif.pl";
$canvasOff = "./canvasOff.pl";

$gtOn = "./goodthingsOn.pl";
$gtOff = "./goodthingsOff.pl";

print "Building Soar agents from kif..\n";
print `$buildKif $source1Kif`;
print `$buildKif $source2Kif`;
print `$buildKif $targetKif`;

checkFor("$agentDir/$source1.soar");
checkFor("$agentDir/$source2.soar");
checkFor("$agentDir/$target.soar");
clearLog($source1Log);
clearLog($source2Log);
clearLog($targetWithSourceLog);
clearLog($targetWithoutSourceLog);
#clearLog($goodThings);
if (-e $goodThings) {
  print `rm $goodThings`;
}

print "Extracting mappings (untimed)..\n";
print `./runMapper.pl $source1Kif $targetKif 2>/dev/null > $mappingFile1`; 
print "Extracting mappings (untimed)..\n";
print `./runMapper.pl $source2Kif $targetKif 2>/dev/null > $mappingFile2`; 

print `$canvasOff`;

print `$gtOff`;
print "Running $source1..\n";
print `$timeCommand $runSoar -w1 $agentDir/$source1.soar > $source1Log`;
foreach $line (`cat $timeFile`) {
  chomp $line;
  print `echo '# UNIX TIME $line' >> $source1Log`;
}
lastDecision($source1Log);

if (`grep '1700000 decisions' $source1Log`) {
  print "Aborting scenario, timeout on source\n";
  print `touch $goodThings`;
  print `touch $source2Log`;
  print `touch $targetWithSourceLog`;
  print `touch $targetWithoutSourceLog`;
  print `rm $mappingFile1`;
  print `rm $mappingFile2`;
  print `rm $timeFile`;
  exit;
}
print "Running $source2..\n";
print `$timeCommand $runSoar -w1 $agentDir/$source2.soar > $source2Log`;
foreach $line (`cat $timeFile`) {
  chomp $line;
  print `echo '# UNIX TIME $line' >> $source2Log`;
}
lastDecision($source2Log);

if (`grep '1700000 decisions' $source2Log`) {
  print "Aborting scenario, timeout on source\n";
  print `touch $goodThings`;
  print `touch $targetWithSourceLog`;
  print `touch $targetWithoutSourceLog`;
  print `rm $mappingFile1`;
  print `rm $mappingFile2`;
  print `rm $timeFile`;
  exit;
}

print `$gtOn`;
print `touch $goodThings`;
print "Extracting goodThings..\n";
print `$timeCommand $genGT $source1Log $source1Kif $targetKif 0 $mappingFile1 >> $goodThings`;
foreach $line (`cat $timeFile`) {
  chomp $line;
  print `echo '# GEN TIME $line' >> $goodThings`;
}
print "found this many goodThings:\n";
print `grep 'sp {' $goodThings | wc -l`;
print `$timeCommand $genGT $source2Log $source2Kif $targetKif 100 $mappingFile2 >> $goodThings`;
foreach $line (`cat $timeFile`) {
  chomp $line;
  print `echo '# GEN TIME $line' >> $goodThings`;
}
print `cp $goodThings $source2.goodthings.soar`;
print "found this many goodThings:\n";
print `grep 'sp {' $goodThings | wc -l`;
print "found the following mappings:\n";
print `grep MAPPING $goodThings`;

print "Running $target with source..\n";
print `$timeCommand $runSoar -w1 $agentDir/$target.soar > $targetWithSourceLog`;
foreach $line (`cat $timeFile`) {
  chomp $line;
  print `echo '# UNIX TIME $line' >> $targetWithSourceLog`;
}

lastDecision($targetWithSourceLog);

print `$gtOff`;
print `echo 'excise elaborate*start-depth' >> $agentDir/$target.soar`;
print "Running $target without source..\n";
print `$timeCommand $runSoar -w1 $agentDir/$target.soar > $targetWithoutSourceLog`;
foreach $line (`cat $timeFile`) {
  chomp $line;
  print `echo '# UNIX TIME $line' >> $targetWithoutSourceLog`;
}
lastDecision($targetWithoutSourceLog);

print `rm $mappingFile1`;
print `rm $mappingFile2`;
print `rm $timeFile`;

sub clearLog() {
  $log = shift;
  if (-e $log) {
#    print "$log exists, ctrl-c now or it gets deleted!\n";
#    $foo = <STDIN>;
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

