#!/usr/bin/perl

use strict;

my $blanklines = 0;

while (<>) {
  if (/Link:/) {
    next;
  }
  if (/\[edit\]/) {
    next;
  }
  if (/Table of contents/) {
    $blanklines = 0;
    while (<>) {
      if (/ +[0-9]/) {
        $blanklines = 0;
	next;
      }
      if (++$blanklines >= 2) {
	$blanklines = 0;
        last;
      }
    }
    next;
  }
  if (/Retrieved from "http:\/\/winter\.eecs\.umich\.edu\/soarwiki/) {
    last;
  }
  
  chomp;
  if ($_ eq "") {
    if (++$blanklines >= 2) {
      --$blanklines;
      next;
    }
  } else {
    $blanklines = 0;
  }
  
  print;
  print "\n";
}
