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
  $line =~ s/var21/var2/g;
  $line =~ s/var31/var3/g;
  $line =~ s/var41/var4/g;
  $line =~ s/var51/var5/g;
  $line =~ s/var61/var6/g;
  $line =~ s/var71/var7/g;
  $line =~ s/var81/var8/g;
  $line =~ s/var91/var9/g;
  print $OUT $line;
}

print `mv $tmpFileName $file`;
