#!/usr/bin/perl
# change the format of a kif file to something easier to parse in perl

foreach $line (`cat $ARGV[0]`) {
  $line=~ s/\s*#.*//;
  $line=~ s/\s*;.*//;
  $line =~ s/^\s*//;
  $line =~ s/\s*$//;
  if ($line =~ /^$/) { next; }
  $line = "$line\n";
  $line =~ s/ /_/g;
  
  if ($line =~ /^\(<=_/) {
    $line =~ s/^\(<=_//;
    print "BEGIN ";
  }


  if ($line =~ /^\(/) {
    $line =~ s/^\(//;
    $line =~ s/\)$//;
  }
  if ($line =~ /true/) {
    $line =~ s/true_\(/TRUE_/;
    $line =~ s/\)$//;
  }
  if ($line =~ /not_\(/) {
    $line =~ s/not_\(/NOT /;
    $line =~ s/\)$//;
  }
  $line =~ s/not_/NOT /;
 

  if ($line =~/_\(/) {
    $line =~ s/_\(/_/;
    $line =~ s/\)//;
  }

  $line =~ s/\)$/\nEND/g;

  $line =~ s/ legal_/ A/;
  $line =~ s/^legal_/A/;
  $line =~ s/ next_/ TRUE_/;
  $line =~ s/^next_/TRUE_/;
  $line =~ s/ does_/ A/;
  $line =~ s/^does_/A/;
  print $line;
}
