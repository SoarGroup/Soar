#!/usr/bin/perl

die unless (-e $ARGV[0]);
$file = $ARGV[0];
print `mv $file $file.tmp`;

die unless (-e "$file.tmp");

open ORIG, "<$file.tmp" or die;
open NEW, ">$file" or die;

$seed = int(rand(999999));
$condition = 0;
while (<ORIG>) {
  if ($condition == 1) {
    $condition = 0;
    print NEW "$seed\n";
  }
  else {
    if ($_ =~ /^random-seed\W*$/) {
      $condition = 1;
    }
    print NEW $_;
  }
}

print `rm $file.tmp`;
 
