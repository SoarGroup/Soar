#!/usr/bin/perl

die unless ($#ARGV == 1);

$env = $ARGV[0];

$tmpLC = "bk_tmp";
die if (-e $tmpLC);

$file = $ARGV[1];
die "can't find $ARGV[1]" unless (-e $ARGV[1]);
die "$file not a kif" unless ($file =~ /\.kif$/);

$file =~ /([^\/]*)\.kif/;
$rootName = $1;

$compiler = "python ../old_translator/LoadKif.py";
$soarFile = "../agents/$rootName\.soar";
$lowerCase = "./lowerCase.pl";

$postProcess = "./postProcessAgent.pl $tmpLC $soarFile $env";

print "building $soarFile\n";

if (-e $soarFile) {
  print `mv $soarFile buildKifBackup.soar`;
}
print `$lowerCase $file > $tmpLC`;

print `$compiler $tmpLC`;
if (not -e $soarFile) {
  print `rm $tmpLC`;
  die "kif compile failed!";
}

print `$postProcess`;

print `rm $tmpLC`;
