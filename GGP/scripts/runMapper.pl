#!/usr/bin/perl
#use Cwd 'abs_path';

die unless ($#ARGV == 1);

$kifOne = $ARGV[0];
$kifTwo = $ARGV[1];

$ENV{"GGP_PATH"}="../";
$ENV{"PYTHONPATH"}="./pyparser/:.";
$tmp1 = "$kifOne.tmp.map";
$tmp2 = "$kifTwo.tmp.map";

print `sed 's/^M\$//' $kifOne |grep -v init > $tmp1`;
print `sed 's/^M\$//' $kifTwo |grep -v init > $tmp2`;

#$mapper = "python ../analogy/const_match/const_match3.py";
$mapper = "./newConstantMapper.pl";

checkFor($tmp1);
checkFor($tmp2);
checkFor($mapper);

print lc `$mapper $tmp1 $tmp2 | grep "map predicate\\|map constant"`;


sub checkFor() {
  $file = shift;
  die "$file does not exist" unless (-e $file);
}

