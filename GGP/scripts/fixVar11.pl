#!/usr/bin/perl

our $file = $ARGV[0];
if (not -e $file) {
  die "first arg is file\n";
}

$tmpFileName = "fixv11_tmp";
die if (-e $tmpFileName);
open $FILE, "<$file";
open $OUT, ">$tmpFileName";

die if ($#ARGV != 0);

foreach $line (<$FILE>) {
  $line =~ s/var11/var1/g;
  print $OUT $line;
}

print `mv $tmpFileName $file`;
