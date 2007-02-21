#!/usr/bin/perl

die unless ($#ARGV == 1);
$name = $ARGV[0];
$baseSoarFile = $ARGV[1];

our $scriptDir = "./pysrc"; 
die "$baseSoarFile does not exist" unless (-e $baseSoarFile);
$probFile = "$name\_probs";

if (not -e $probFile) {
  print "generating problems..\n";
  print `$scriptDir/getRandomProblemsWithSolutions.pl $scriptDir/locs-h $scriptDir/locs-v 6 20 > $probFile`;
}
else {
  print "using existing problems: $probFile\n";
}

$dirName = "$name\_logs";

$i = 0;
while (-d $dirName) {
  $dirName = "$dirName$i";
  $i++;
}

print "will store results in directory $dirName\n";

print `mkdir $dirName`;
print `cd $dirName; ln -s ../common`;

@probs = `cat $probFile`;

$i = 0;
foreach $prob (@probs) {
  chomp $prob;
  print "running problem: $prob\n";
  $prob =~ /(\d+) (\d+) (\d+) (\d+)/ or die "bad: $prob";
  $explorerX = $1;
  $explorerY = $2;
  $mummyX = $3;
  $mummyY = $4;
  
  $trialString = "e$explorerX\_$explorerY\_m$mummyX\_$mummyY";
  $trialPrefix = "$dirName/$trialString";

  # run each problem three times: source, target w/o transfer, target
  # w/transfer
  
  # source problem: horizontal mummy
  $probType = "source";
  $soarFile = "$trialPrefix\_$probType.soar";
  print `cp $baseSoarFile $soarFile`;

  setSoarFileFields($soarFile, "mummy_type", "horizontal");
  setSoarFileFields($soarFile, "explorer_location", $explorerX, $explorerY);
  setSoarFileFields($soarFile, "mummy_location", $mummyX, $mummyY);
  setSoarFileFields($soarFile, "chunk_file", "empty");

  $postRunCommand = "command-to-file $trialPrefix\_chunks.soar print --chunks --full";
  runTrial($soarFile, "$trialPrefix\_$probType.log", $postRunCommand);

  # target w/o chunks
  $probType = "nosource_target";
  $soarFile = "$trialPrefix\_$probType.soar";
  print `cp $baseSoarFile $soarFile`;

  setSoarFileFields($soarFile, "mummy_type", "vertical");
  setSoarFileFields($soarFile, "explorer_location", $explorerX, $explorerY);
  setSoarFileFields($soarFile, "mummy_location", $mummyX, $mummyY);
  setSoarFileFields($soarFile, "chunk_file", "empty");

  runTrial($soarFile, "$trialPrefix\_$probType.log", "");

  # target w/chunks
  $probType = "target";
  $soarFile = "$trialPrefix\_$probType.soar";
  print `cp $baseSoarFile $soarFile`;
    
  processChunks("$trialPrefix\_chunks.soar");
  setSoarFileFields($soarFile, "mummy_type", "vertical");
  setSoarFileFields($soarFile, "explorer_location", $explorerX, $explorerY);
  setSoarFileFields($soarFile, "mummy_location", $mummyX, $mummyY);
  setSoarFileFields($soarFile, "chunk_file", "$trialPrefix\_chunks");

  runTrial($soarFile, "$trialPrefix\_$probType.log", "");
}

sub setSoarFileFields {
  # eg setSoarFileFields(mm.soar, "explorer_location", 0, 0);
  $soarFile = shift;
  $fieldKey = shift;

  $valString = "";
  foreach $val (@_) {
    $valString = "$valString $val";
  }

  print `$scriptDir/subLine.pl $soarFile $fieldKey$valString > tmp_ssff`;
  print `mv tmp_ssff $soarFile`;
}

sub processChunks {
  $chunkFile = shift;

  die "no chunk file: $chunkFile" unless (-e $chunkFile);

  print `$scriptDir/removeMummy.pl $chunkFile > tmp_pc`;
  print `$scriptDir/con

sub runTrial {
  $agent = shift;
  $log = shift;
  die unless (-e $agent);
  $cmd = shift;
  print `echo "$cmd" > $log`;
}

