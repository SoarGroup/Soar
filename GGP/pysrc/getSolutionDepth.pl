#!/usr/bin/perl

die unless -e "$ARGV[0]";

$multi = $#ARGV; 
foreach $file (@ARGV) {
  $max = -1;
  @depths = `grep Depth $file`;
  foreach $depth (@depths) {
    chomp $depth;
    $depth =~ /Depth: (\d+),/ or die;
    if ($1 > $max) {
      $max = $1;
    }
  }
  if ($multi > 0) {
    print "$file: $max\n";
  }
  else {
    print "$max\n";
  }
}
