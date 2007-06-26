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

if ($ARGV[0] =~ "rogue" or
    $ARGV[0] =~ "mm") {
  print `cp $file math_backup`;
  print `./fakeMath.pl $file`;
}

print `$compiler $file`;
if (not -e $soarFile) {
  clean();
  die "kif compile failed!";
}

print `$hackScript $soarFile`;
clean();

sub clean() {
  if ($ARGV[0] =~ "rogue" or
      $ARGV[0] =~ "mm") {
    print `cp math_backup $file`;
  }
}
