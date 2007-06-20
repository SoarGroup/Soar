#!/usr/bin/perl

die unless ($#ARGV == 1);

$hackScript = "";
if ($ARGV[0] =~ "escape") {
  $hackScript = "./hackEscape.pl";
}
elsif ($ARGV[0] =~ "mm") {
  $hackScript = "./hackMM.pl";
}
elsif ($ARGV[0] =~ "rogue") {
  $hackScript = "./hackRogue.pl";
}
else { die; }

$file = $ARGV[1];
die "can't find $ARGV[1]" unless (-e $ARGV[1]);
die "$file not a kif" unless ($file =~ /\.kif$/);

$file =~ /([^\/]*)\.kif/;
$rootName = $1;

$compiler = "python ../pysrc/LoadKif.py";
$soarFile = "../agents/$rootName\.soar";
print "building $soarFile\n";

if (-e $soarFile) {
  print `mv $soarFile buildKifBackup.soar`;
}

print `$compiler $file`;
die "kif compile failed!" unless (-e $soarFile);

print `$hackScript $soarFile`;
