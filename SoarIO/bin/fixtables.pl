#!/usr/bin/perl
#

use strict;

while (<>) {

  if (/\\begin\{tabular\}\{\|c\|c\|\}/) {
    print "\\begin{tabular}{|l|p{4.8in}|}\n";
  } else {
    print;
  }
}
