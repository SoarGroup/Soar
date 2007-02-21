#!/usr/bin/perl

die "$ARGV[0] does not exist" unless (-e $ARGV[0]);

$file = $ARGV[0];


my $fh;
open ($fh, "<$file"); 

@buffer = ();
$inProd = 0;
foreach $line (<$fh>) {
  if ($line =~ /^\w*sp/) {
    if ($#buffer != 0) {
      # print out the buffer, if applicable
      handleProduction(@buffer);
      @buffer = ();
      push @buffer, $line;
    }
  }
  else {
    push @buffer, $line;
  }
}

sub handleProduction {
  #if (grep(/depth-exceeded/, @_) != 0) {
  #  return;
  #}
  #if (grep(/failure/, @_) != 0) {
  #  return;
  #}
  if (grep(/success/, @_) == 0 and grep(/failure/, @_) == 0) {
     # and grep(/depth-exceeded/, @_) == 0) {
    return;
  }
  else {
    foreach $line (@_) { 
      $line =~ s/\^current-evaluation-depth \d+//;
      print $line; 
    }
  }
}
