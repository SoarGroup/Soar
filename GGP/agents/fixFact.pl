#!/usr/bin/perl

our $file = $ARGV[0];
if (not -e $file) {
  die "first arg is file\n";
}

$tmpFileName = "fixfact_tmp";
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
    $elabVar = "bad_fixfact";
    $factVar = "bad_fixfact";
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
    if ($line =~ /\^elaborations <(.*?)>/) {
      $elabVar = $1;
    }
    if ($line =~ /\^facts <(.*?)>/) {
      $factVar = $1;
    }

    if ($line =~ /<$elabVar> .* (\^$attribute [^\)^\s]*)/) {
      $ref = $1;
      $line =~ s/ \^$attribute [^\)^\s]*//;
      #$line =~ s/$ref//; ?? won't work
      print $OUT "$line\n   (<$factVar> $ref)\n";
    }
    else {
      print $OUT "$line\n";
    }
  }
}

print `mv $tmpFileName $file`;
