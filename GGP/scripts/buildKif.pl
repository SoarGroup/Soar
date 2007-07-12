#!/usr/bin/perl

die unless ($#ARGV == 1);

$env = $ARGV[0];

$file = $ARGV[1];
die "can't find $ARGV[1]" unless (-e $ARGV[1]);
die "$file not a kif" unless ($file =~ /\.kif$/);

$file =~ /([^\/]*)\.kif/;
$rootName = $1;

$compiler = "python ../old_translator/LoadKif.py";
$soarFile = "../agents/$rootName\.soar";

$postProcess = "./postProcessAgent.pl $file $soarFile $env";

print "building $soarFile\n";

if (-e $soarFile) {
  print `mv $soarFile buildKifBackup.soar`;
}

print `cp $file math_backup`;
print `./fakeMath.pl $file`;

print `$compiler $file`;
if (not -e $soarFile) {
  clean();
  die "kif compile failed!";
}

print `$postProcess`;
clean();

sub clean() {
  print `cp math_backup $file`;
}
