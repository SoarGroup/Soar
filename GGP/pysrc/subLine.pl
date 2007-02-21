#!/usr/bin/perl

if ($#ARGV < 2) {
  print "subLine.pl: substitute a line in a file. Given file, token and N words,\n";  
  print "look for the token in the file, and, on every line it occurs, \n";
  print "substitute in the words following the specified pattern.\n";
  print "this is an example line:\n";
  print "(<location1> ^p1 explorer ^p2 2 ^p3 3) # explorer_location p2 p3 \n";
  print "if token is explorer_location and words are 5 and 6, the line becomes\n";
  
  print "(<location1> ^p1 explorer ^p2 5 ^p3 6) # explorer_location p2 p3 \n";
  print "usage: subLine.pl file token word1 word2..\n";
}

$file = shift @ARGV;
$token = shift @ARGV;

open $IN, "<$file" or die;

foreach $line (<$IN>) {
  if ($line =~ /$token/) {
    $line =~ /^(.*)$token(.*)$/ or die;
    $content = $1;
    $pattern = $2;
    $index = 0;

    while ($pattern =~ /^\s*?(\w+)/) {
      $hook = $1;
      $pattern =~ s/^\s*?\w+//;
      $content =~ /$hook\s+(\w+)/ or die "bad pattern.";
      if ($index > $#ARGV) {
        die "not enough args for pattern: $line\n";
      }
      $content =~ s/($hook\s+)\w+/$1$ARGV[$index]/;
      $index++;
    }

    $line =~ s/^.*($token.*)/$content$1/;

    print $line;
  }
  else {
    print $line;
  }
}
