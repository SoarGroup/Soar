#!/usr/bin/perl

our $file = $ARGV[0];
if (not -e $file) {
  die "first arg is file\n";
}

$tmpFileName = "fakemath_out";
die if (-e $tmpFileName);
open $FILE, "<$file";
open $OUT, ">$tmpFileName";

die if ($#ARGV != 0);

foreach $line (<$FILE>) {
  $line =~ s/\(\+/\(plus/g;
  $line =~ s/\(\-/\(minus/g;
#  $line =~ s/\(\>\=/\(gtequal/g;
#  $line =~ s/\(\>/\(greaterthan/g;
  $line =~ s/\(\< /\(lessthan /g;
  print $OUT $line;
}
#print `cp $tmpFileName fakemath_int.kif`;
print `mv $tmpFileName $file`;
