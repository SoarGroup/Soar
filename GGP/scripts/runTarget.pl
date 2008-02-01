#!/usr/bin/perl

# Just runs the target with and without good things, by copying the good things
# file from a specified location

die unless ($#ARGV == 0);
$targetKif = $ARGV[0];

checkFor($targetKif);

$agentDir = "../agents";

$targetKif =~ /^(.*?)([^\/]+)\.kif$/;
$kifPath = $1;
$target = $2;

$target =~ /^(\w+)-/;
$environment = $1;

$coreKif = "$kifPath$environment.core.kif";
$tmpTarget = "";
$merged = 0;
if (-e $coreKif) {
  print "kif files will be merged with core: $coreKif\n";
  $merged = 1;
  $tmpTarget = "$kifPath$target.merge.kif";
	system("cat $coreKif $targetKif | sed 's/\$//' > $tmpTarget");
}
else {
  $tmpTarget = "$kifPath$target.unix.kif";
	system("cat $targetKif | sed 's/\$//' > $tmpTarget");
}

$targetKif = $tmpTarget;
$targetWithSourceLog = "$target\_after_source.log";
$targetWithoutSourceLog = "$target\_no_source.log";

$goodThings = "$agentDir/goodthings.soar";

$timeFile = "tmp.time";
$timeCommand = '/usr/bin/time -o ' . $timeFile . ' -f \'%E real,%U user,%S sys\' ';

$runSoar = "./runSoar.py";

$buildKif = "./buildKif.pl";
$canvasOff = "./canvasOff.pl";

$gtOn = "./goodthingsOn.pl";
$gtOff = "./goodthingsOff.pl";

print "Building Soar agents from kif..\n";
print `$buildKif $targetKif`;

if ($merged) {
  print `mv $agentDir/$target.merge.soar $agentDir/$target.soar`;
}
else {
  print `mv $agentDir/$target.unix.soar $agentDir/$target.soar`;
}

checkFor("$agentDir/$target.soar");
clearLog($targetWithSourceLog);
clearLog($targetWithoutSourceLog);

# here we're going to copy the good things file over
if (-e $goodThings) {
  print `rm $goodThings`;
}

$target =~ /^(.*)-target$/;

# copy the appropriate file, depending on if you want to bump or not
#$savedGT="../good_things/$1.goodthings.soar";
$savedGT="../good_things/$1.goodthings.bump.soar";

checkFor($savedGT);
print `cp $savedGT $goodThings`;
checkFor($goodThings);

# run the targets with and without good things

print `$canvasOff`;

print `rm $tmpTarget`;

print "Running $target with source..\n";

print `$gtOn`;
#print `$gtOff`;

print `$timeCommand $runSoar -w1 $agentDir/$target.soar > $targetWithSourceLog`;
foreach $line (`cat $timeFile`) {
  chomp $line;
  print `echo '# UNIX TIME $line' >> $targetWithSourceLog`;
}

lastDecision($targetWithSourceLog);

##print `$gtOff`;
##print `echo 'excise elaborate*start-depth' >> $agentDir/$target.soar`;
##print "Running $target without source..\n";
##print `$timeCommand $runSoar -w1 $agentDir/$target.soar > $targetWithoutSourceLog`;
##foreach $line (`cat $timeFile`) {
##  chomp $line;
##  print `echo '# UNIX TIME $line' >> $targetWithoutSourceLog`;
##}
##lastDecision($targetWithoutSourceLog);

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

