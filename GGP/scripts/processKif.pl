#!/usr/bin/perl
# change the format of a kif file to something easier to parse in perl

foreach $line (`cat $ARGV[0]`) {
  if ($line =~ /\(or (.*)\)\s*$/) {
    $line = $1;
#    print $line;
  }
  while ($line =~ /^(\s*\(.*?\))\s*\(/) {
    $part = $1;
    $line =~ s/^\s*\(.*?\)\s*\(/\(/;
    handleLine($part);
  }

  handleLine($line);
}
sub handleLine() {
  my $line = shift;
  $line=~ s/\s*#.*//;
  $line=~ s/\s*;.*//;
  $line =~ s/^\s*//;
  $line =~ s/\s*$//;
  if ($line =~ /^$/) { next; }
  $line = "$line\n";
  $line =~ s/ /_/g;

  # $line = lc $line;
  
  if ($line =~ /^\(<=_/) {
    $line =~ s/^\(<=_//;
    print "BEGIN ";
  }


  if ($line =~ /^\(/) {
    $line =~ s/^\(//;
    $line =~ s/\)$//;
  }
  if ($line =~ /true_\(/) {
    $line =~ s/true_\(/TRUE_/;
    $line =~ s/\)$//;
  }
  if ($line =~ /not_\(/) {
    $line =~ s/not_\(/NOT /;
    $line =~ s/\)$//;
  }
  $line =~ s/^true_/TRUE_/; # for cases where true isn't followed by paren
  $line =~ s/not_/NOT /;
 

  if ($line =~/_\(/) {
    $line =~ s/_\(/_/;
    $line =~ s/\)//;
  }

  $line =~ s/\)$/\nEND/g;

  $line =~ s/goal_[^_]*_(\d+)/NEXTgoal$1/;

  $line =~ s/ legal_[^_]*_/ A/;
  $line =~ s/^legal_[^_]*_/A/;
  $line =~ s/ next_/ TRUE_/;
  $line =~ s/^next_/TRUE_/;
  $line =~ s/ does_[^_]*_/ A/;
  $line =~ s/^does_[^_]*_/A/;
  $line =~ s/\?/V/g;
  print $line;
}
