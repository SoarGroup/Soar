#!/usr/bin/perl
#

use strict;

while (<>) {

  if (/\\begin\{tabular\}\{\|c\|c\|\}/) {
    print "\\begin{tabular}{|p{1in}|p{5in}|}\n";
  } else {
    if (/\\begin\{tabular\}\{\|c\|c\|c\|\}/) {
      print "\\begin{tabular}{|p{1in}|p{1in}|p{4in}|}\n";
    } else {
      print;
    }
  }
}
