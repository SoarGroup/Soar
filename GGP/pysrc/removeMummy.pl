#!/usr/bin/perl

die "$ARGV[0] does not exist" unless (-e $ARGV[0]);

$file = $ARGV[0];

#print `./condense.pl $file > tmp_cog`;

my $fh;
open ($fh, "<$file"); 

foreach $line (<$fh>) {
  if ($line =~ /p1 mummy/)
    next;
  }
  else {
    print "$line";
  }
}
