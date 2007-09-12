#!/usr/bin/perl

die unless ($#ARGV == 0);

$tmpLC = "bk_tmp";
$tmp2 = "bk_tmp2";
die if (-e $tmpLC);
die if (-e $tmp2);

$file = $ARGV[0];
die "can't find $ARGV[0]" unless (-e $ARGV[0]);
die "$file not a kif" unless ($file =~ /\.kif$/);

$file =~ /([^\/]*)\.kif/;
$rootName = $1;

$compiler = "python ../old_translator/LoadKif.py";
$soarFile = "../agents/$rootName\.soar";
$lowerCase = "./lowerCase.pl";

$postProcess = "./postProcessAgent.pl $file $soarFile";

print "building $soarFile\n";

if (-e $soarFile) {
  print `mv $soarFile buildKifBackup.soar`;
}
print `$lowerCase $file > $tmpLC`;
print `mv $file $tmp2`;
print `mv $tmpLC $file`;

print `$compiler $file`;
if (not -e $soarFile) {
  print `mv $tmp2 $file`;
  die "kif compile failed!";
}

print `$postProcess`;

print `mv $tmp2 $file`;
