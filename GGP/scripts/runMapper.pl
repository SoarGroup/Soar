#!/usr/bin/perl
use Cwd 'abs_path';

die unless ($#ARGV == 1);

$kifOne = abs_path($ARGV[0]);
$kifTwo = abs_path($ARGV[1]);

$mapperDir = "../analogy/src";
$mapper = "./mapper";
$preProcessor = "xkif_gen.py";
#$fakeMath = "../../scripts/fakeMath.pl";

#$origOne = "mapper_bu1";
#$origTwo = "mapper_bu2";

$tempOne = "mapper_tmp1";
$tempTwo = "mapper_tmp2";

chdir($mapperDir);

checkFor($kifOne);
checkFor($kifTwo);
checkFor($mapper);
checkFor($preProcessor);
#checkFor($fakeMath);

die if (-e $tempOne);
die if (-e $tempTwo);
#die if (-e $origOne);
#die if (-e $origTwo);

#print `cp $kifOne $origOne`;
#print `cp $kifTwo $origTwo`;

#print `$fakeMath $kifOne`;
#print `$fakeMath $kifTwo`;

print `python $preProcessor $kifOne > $tempOne`;
print `python $preProcessor $kifTwo > $tempTwo`;

print `$mapper $tempOne $tempTwo`;

print `rm $tempOne`;
print `rm $tempTwo`;

#print `mv $origOne $kifOne`;
#print `mv $origTwo $kifTwo`;

sub checkFor() {
  $file = shift;
  die "$file does not exist" unless (-e $file);
}

