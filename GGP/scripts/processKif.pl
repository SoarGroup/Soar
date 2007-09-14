#!/usr/bin/perl
# change the format of a kif file to something easier to parse in perl

$lastLine = "";
foreach $line (`cat $ARGV[0]`) {
  # strip comments
  $line =~ s/\s*;.*//;
  $line =~ s/^\s*//; # remove leading space
  if (not $lastLine eq "") {
    $line = "$lastLine $line"; # merge unused previous line with this line
    $lastLine = "";
  }
  if ($line =~ /\?[^)]+$/) { # if line ends with a variable (no close paren)
    # lines can end w/o paren, so we must check for variable eg (<= terminal
    $lastLine = $line;
    chomp $lastLine;
    $lastLine =~ s/\s*$//; # remove trailing spaces for better appending
    next; # skip this line and prepend it to the next
  }
  if ($line =~ /\(or (.*)\)\s*$/) {
    # ignore disjunctions: treat (or (statement) (statement)) as (statement) (statement)
    # FIXME could cause errors in findGroundings, as it will think that both
    # conditions must hold
    $line = $1;
  }
  while ($line =~ /^(\s*\(.*?\))\s*\(/) {
    # multiple statements in a line: (something) (something)
    # break into two lines
    $part = $1;
    $line =~ s/^\s*\(.*?\)\s*\(/\(/;
    handleLine($part);
  }

  handleLine($line);
}
sub handleLine() {
  my $line = shift;
  $line=~ s/\s*#.*//;
  $line =~ s/^\s*//;
  $line =~ s/\s*$//;
  if ($line =~ /^$/) { next; }
  $line = "$line\n";
  $line =~ s/ \)/\)/g;
  $line =~ s/ /:/g;

  # $line = lc $line;
  
  if ($line =~ /^\(<=:/) {
    $line =~ s/^\(<=://;
    print "BEGIN ";
  }


  if ($line =~ /^\(/) {
    $line =~ s/^\(//;
    $line =~ s/\)$//;
  }
  if ($line =~ /true:\(/) {
    $line =~ s/true:\(/TRUE:/;
    $line =~ s/\)$//;
  }
  if ($line =~ /not:\(/) {
    $line =~ s/not:\(/NOT /;
    $line =~ s/\)$//;
  }
  $line =~ s/^true:/TRUE:/; # for cases where true isn't followed by paren
  $line =~ s/not:/NOT /;
 

  if ($line =~/:\(/) {
    $line =~ s/:\(/:/;
    $line =~ s/\)//;
  }

  $line =~ s/\)$/\nEND/g;

  $line =~ s/goal:[^:]*:(\d+)/TRUEgoal$1/;

  $line =~ s/ legal:[^:]*:/ AXN/;
  $line =~ s/^legal:[^:]*:/AXN/;
  $line =~ s/ next:/ TRUE:/;
  $line =~ s/^next:/TRUE:/;
  $line =~ s/ does:[^:]*:/ AXN/;
  $line =~ s/^does:[^:]*:/AXN/;
  $line =~ s/\?/V/g;
  print $line;
}
