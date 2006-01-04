#!/usr/bin/perl

use strict;

open NAMES, "../command-names" or die "Could not open command-names: $!";
my @names = <NAMES>;

open POSTPROCESSFILE, "html-process.pl" or die "Could not find required file html-process.pl: $!";
close POSTPROCESSFILE;

#make the dir, 
if (!mkdir "../help") {
  if ($! ne "File exists") {
    die "Couldn't create directory: $!";	
  }
}

#print "total: " . $#links . "\n";
for (my $i = 0; $i <= $#names; $i++) {
  chomp @names[$i];
  #print @names[$i] . ": " . $i . "\n";
  #next;
  print "Processing @names[$i]\n";
  
  `wget -q -O ../help/@names[$i].wiki.html http://winter.eecs.umich.edu/soarwiki/@names[$i]`;
  
  `./html-process.pl ../help/@names[$i].wiki.html > ../help/@names[$i].html`;
  unlink "../help/@names[$i].wiki.html" or die "Could not remove ../help/@names[$i].wiki.html: $!";
  
  `elinks -dump -no-numbering -no-references ../help/@names[$i].html > ../help/@names[$i]`;

  chdir "../help" or die "Could not change to ../help directory: $!";
  `html2latex --nopar @names[$i].html`;
  `../help-scripts/latex-post-process.pl < @names[$i].tex > @names[$i].tex.new`;
  rename "@names[$i].tex.new", "@names[$i].tex" or die "Could not rename @names[$i].tex.new: $!";
  #`pdflatex --interaction=nonstopmode @names[$i].tex`;
  chdir "../help-scripts" or die "Could not change back to ../help-scripts directory: $!";
}
