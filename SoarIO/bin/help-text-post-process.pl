#!/usr/bin/perl

use strict;

while (<>) {
  if (/Link:/) {
    next;
  }
  if (/\[edit\]/) {
    next;
  }
  if (/Table of contents/) {
    my $blanklines = 0;
    while (<>) {
      if (/ +[0-9]/) {
        $blanklines = 0;
	next;
      }
      if (++$blanklines >= 2) {
        last;
      }
    }
    next;
  }
  if (/Retrieved from "http:\/\/winter\.eecs\.umich\.edu\/soarwiki/) {
    last;
  }
  print;
}
