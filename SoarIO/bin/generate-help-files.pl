#!/usr/bin/perl

use strict;

open LINKS, "command-links" or die "Could not open command-links: $!";
my @links = <LINKS>;

open NAMES, "command-names" or die "Could not open command-names: $!";
my @names = <NAMES>;

open POSTPROCESSFILE, "help-text-post-process.pl" or die "Could not find required file help-text-post-process.pl: $!";
close POSTPROCESSFILE;

if ($#links != $#names) {
  die "command-links and command-names are different sizes: $#links/$#names";
}

#make the dir, 
if (!mkdir "help") {
  if ($! ne "File exists") {
    die "Couldn't create directory: $!";	
  }
}

for (my $i = 0; $i < $#links; $i++) {
  chomp @names[$i];
  print "Processing @names[$i]\n";
  `elinks -dump -no-numbering -no-references "@links[$i]" > help/@names[$i]`;

  `./help-text-post-process.pl < help/@names[$i] > help/@names[$i].new`;
  unlink "help/@names[$i]" or die "Could not remove help/@names[$i]: $!";
  rename "help/@names[$i].new", "help/@names[$i]" or die "Could not rename help/@names[$i].new: $!";
}
