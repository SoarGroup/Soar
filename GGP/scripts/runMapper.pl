#!/usr/bin/perl
use Cwd 'abs_path';

die unless ($#ARGV == 1);

$kifOne = abs_path($ARGV[0]);
$kifTwo = abs_path($ARGV[1]);

$mapperDir = "../analogy/src";
$mapper = "./mapper";
$preProcessor = "../../scripts/pyparser/xkif_gen.py";

$tempOne = "mapper_tmp1";
$tempTwo = "mapper_tmp2";

chdir($mapperDir);

checkFor($kifOne);
checkFor($kifTwo);
checkFor($mapper);
checkFor($preProcessor);

die if (-e $tempOne);
die if (-e $tempTwo);

print `python $preProcessor $kifOne > $tempOne`;
print `python $preProcessor $kifTwo > $tempTwo`;

print `$mapper $tempOne $tempTwo`;

print `rm $tempOne`;
print `rm $tempTwo`;


sub checkFor() {
  $file = shift;
  die "$file does not exist" unless (-e $file);
}

