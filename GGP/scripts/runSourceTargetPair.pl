#!/usr/bin/perl

die unless ($#ARGV == 1);
$sourceKif = $ARGV[0];
$targetKif = $ARGV[1];

checkFor($sourceKif);
checkFor($targetKif);

$agentDir = "../agents";

$sourceKif =~ /^(.*?)([^\/]+)\.kif$/;
$kifPath = $1;
$source = $2;

$source =~ /^(\w+)-/;
$environment = $1;

$targetKif =~ /([^\/]*)\.kif/;
$target = $1;

$coreKif = "$kifPath$environment.core.kif";
$tmpSource = "";
$tmpTarget = "";
$merged = 0;
if (-e $coreKif) {
  print "kif files will be merged with core: $coreKif\n";
  $merged = 1;
  $tmpSource = "$kifPath$source.merge.kif";
  $tmpTarget = "$kifPath$target.merge.kif";
	system("cat $coreKif $sourceKif | sed 's/\$//' > $tmpSource");
	system("cat $coreKif $targetKif | sed 's/\$//' > $tmpTarget");
}
else {
  $tmpSource = "$kifPath$source.unix.kif";
  $tmpTarget = "$kifPath$target.unix.kif";
	system("cat $sourceKif | sed 's/\$//' > $tmpSource");
	system("cat $targetKif | sed 's/\$//' > $tmpTarget");
}

$sourceKif = $tmpSource;
$targetKif = $tmpTarget;
$source1Log = "$source\.log";
$targetWithSourceLog = "$target\_after_source.log";
$targetWithoutSourceLog = "$target\_no_source.log";

$goodThings = "$agentDir/goodthings.soar";

$mappingFile = "tmp.mappings";
$timeFile = "tmp.time";
$timeCommand = '/usr/bin/time -o ' . $timeFile . ' -f \'%E real,%U user,%S sys\' ';

$genGT = "./genGoodThings.pl";

$runSoar = "./runSoar.py";

$buildKif = "./buildKif.pl";
$canvasOff = "./canvasOff.pl";

$gtOn = "./goodthingsOn.pl";
$gtOff = "./goodthingsOff.pl";

print "Building Soar agents from kif..\n";
print `$buildKif $sourceKif`;
print `$buildKif $targetKif`;

if ($merged) {
  print `mv $agentDir/$source.merge.soar $agentDir/$source.soar`;
  print `mv $agentDir/$target.merge.soar $agentDir/$target.soar`;
}
else {
  print `mv $agentDir/$source.unix.soar $agentDir/$source.soar`;
  print `mv $agentDir/$target.unix.soar $agentDir/$target.soar`;
}

checkFor("$agentDir/$source.soar");
checkFor("$agentDir/$target.soar");
clearLog($source1Log);
clearLog($targetWithSourceLog);
clearLog($targetWithoutSourceLog);
if (-e $goodThings) {
  print `rm $goodThings`;
}

print "Extracting mappings (untimed)..\n";
print `./runMapper.pl $sourceKif $targetKif 2>/dev/null > $mappingFile`; 

print `$canvasOff`;

print `$gtOff`;
print "Running $source..\n";
print `$timeCommand $runSoar -w1 $agentDir/$source.soar > $source1Log`;
foreach $line (`cat $timeFile`) {
  chomp $line;
  print `echo '# UNIX TIME $line' >> $source1Log`;
}
lastDecision($source1Log);

if (`grep '1700000 decisions' $source1Log`) {
  print "Aborting scenario, timeout on source\n";
  print `touch $goodThings`;
  print `touch $targetWithSourceLog`;
  print `touch $targetWithoutSourceLog`;
  print `rm $tmpSource`;
  print `rm $tmpTarget`;
  print `rm $mappingFile`;
  print `rm $timeFile`;
  exit;
}

print `$gtOn`;
print `touch $goodThings`;
print "Extracting goodThings..\n";

print `$timeCommand $genGT $source1Log $sourceKif $targetKif 0 $mappingFile >> $goodThings`;
foreach $line (`cat $timeFile`) {
  chomp $line;
  print `echo '# GEN TIME $line' >> $goodThings`;
}

print `cp $goodThings $source.goodthings.soar`;
print "found this many goodThings:\n";
print `grep 'sp {' $goodThings | wc -l`;
print "found the following mappings:\n";
print `grep MAPPING $goodThings`;

print `rm $tmpSource`;
print `rm $tmpTarget`;

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

print `rm $mappingFile`;
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

