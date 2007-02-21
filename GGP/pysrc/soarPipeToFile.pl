#!/usr/bin/perl

die unless ($#ARGV == 1);

open $OUT, ">$ARGV[1]" or die;
while ($line = `cat $ARGV[0]`) {
  print $OUT $line;
  if ($line =~ /An agent halted during the run/) {
    last;
  }
}
