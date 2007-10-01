#!/usr/bin/perl
#use Cwd 'abs_path';

die unless ($#ARGV == 1);

$kifOne = $ARGV[0];
$kifTwo = $ARGV[1];

$ENV{"GGP_PATH"}="../";
$ENV{"PYTHONPATH"}="./pyparser/:.";
$tmp1 = "$kifOne.tmp.map";
$tmp2 = "$kifTwo.tmp.map";

print `grep -v init $kifOne > $tmp1`;
print `grep -v init $kifTwo > $tmp2`;

#$mapper = "python ../analogy/const_match/const_match3.py";
$mapper = "./newConstantMapper.pl";

checkFor($tmp1);
checkFor($tmp2);
checkFor($mapper);

print lc `$mapper $tmp1 $tmp2 | grep "map predicate\\|map constant"`;

print `rm $tmp1`;
print `rm $tmp2`;

sub checkFor() {
  $file = shift;
  die "$file does not exist" unless (-e $file);
}

