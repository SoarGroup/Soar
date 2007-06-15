#!/usr/bin/perl

our $file = $ARGV[0];
if (not -e $file) {
  die "first arg is file\n";
}

$tmpFileName = "mkstatic_tmp";
die if (-e $tmpFileName);
open $FILE, "<$file";
open $OUT, ">$tmpFileName";

die if ($#ARGV != 1);

$attribute = $ARGV[1];

$inLhs = 0;
$usProduction = 0;
foreach $line (<$FILE>) {
  chomp $line;
  if ($inLhs == 0) {
    print $OUT "$line\n";
    if ($line =~ /^\s*sp\s*\{(.*)$/) {
      $inLhs = 1;
      $prodName = $1;
    }
  }
  elsif ($line =~ /\s*-->/) {
    print $OUT "$line\n";
    $inLhs = 0;
  }
  else { # in lhs
    if ($prodName =~ /_$attribute\_/) { # creates the att
      if ($line =~ /(.*\(state <s>)(.*\).*)/) {
        print $OUT "$1 ^superstate nil $2\n";
      }
      else {
        print $OUT "$line\n";
      }
    }
    else { # might read the att
      $line =~ s/\^$attribute /\^top-state-elaborations.$attribute /g;
      print $OUT "$line\n";
    }
  }
}

print `mv $tmpFileName $file`;
