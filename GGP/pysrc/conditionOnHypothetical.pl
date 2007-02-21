#!/usr/bin/perl

die "$ARGV[0] does not exist" unless (-e $ARGV[0]);

$file = $ARGV[0];

#print `./condense.pl $file > tmp_cog`;

my $fh;
open ($fh, "<$file"); 

#print "sp {elaborate*game-type\n";
#print "(state <s>)\n";
#print "-->\n";
#print "(<s> ^game-type $type)}\n";

foreach $line (<$fh>) {
  if ($line =~ /^\s*\(state \<(\S+?)\>(.+)$/) {
    $var = $1;
    $contents = $2;
    print "(state <$1> ^hypothetical\n";
    print "          $contents\n";
  }
  else {
    print "$line";
  }
}
