#!/usr/bin/perl
#use Cwd 'abs_path';

die unless ($#ARGV == 1);

$kifOne = $ARGV[0];
$kifTwo = $ARGV[1];

$ENV{"GGP_PATH"}="../";
$ENV{"PYTHONPATH"}="./pyparser/:.";

#$mapper = "python ../analogy/const_match/const_match3.py";
$mapper = "./newConstantMapper.pl";

checkFor($kifOne);
checkFor($kifTwo);
checkFor($mapper);

print lc `$mapper $kifOne $kifTwo | grep "map predicate\\|map constant"`;


sub checkFor() {
  $file = shift;
  die "$file does not exist" unless (-e $file);
}

