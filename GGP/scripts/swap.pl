#!/usr/bin/perl

$prev = "";
$parity = 0;
foreach $line (<>) {
  if ($line =~ /-source/) {
    print $line;
    next;
  }
  if ($parity == 0) {
    $parity = 1;
    $prev = $line;
  }
  else {
    $parity = 0;
    print $line;
    print $prev;
  }
}
