#!/usr/bin/perl

die unless ($#ARGV == 0);
die unless (-e "$ARGV[0]");

foreach $line (`cat $ARGV[0]`) {
  print lc $line;
}
