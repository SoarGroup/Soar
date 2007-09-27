#!/usr/bin/perl
# change the format of a kif file to something easier to parse in perl
# this is a HORRIBLE script, it really needs replacing.

$lastLine = "";
$levelBeforeLine = 0;
$levelAfterLine = 0;
foreach $line (`cat $ARGV[0]`) {
  # strip comments
  $line =~ s/\s*;.*//;
  $line =~ s/^\s*//; # remove leading space

  $levelBeforeLine = $levelAfterLine;
  $levelAfterLine += parenOpenCount($line) - parenCloseCount($line);
  #print "l: $line bc: $levelBeforeLine ac: $levelAfterLine\n";

  if ($levelBeforeLine == 0 and $levelAfterLine > 0) {
    print "BEGIN ";
    $line =~ s/\(//; # remove the first open paren
  }
  if ($levelAfterLine == 0 and $levelBeforeLine > 0) {
    $line =~ s/\)[^\)]*?$//;
    # remove the last paren (and everything following) (print END later)
    if ($line =~ /^\s*$/) {
      # paren was alone, print end now and go on (the other code can't handle
      # this case)
      print "END\n";
      next;
    }
  }
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
  if ($line =~ /\(or (.*)$/) {
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
  #print "2 bc: $levelBeforeLine ac: $levelAfterLine\n";
  if ($levelAfterLine == 0 and $levelBeforeLine > 0) {
    print "END\n";
  }
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
  
  if ($line =~ /^<=:/) {
    $line =~ s/^<=://;
    # print "BEGIN ";
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

  #$line =~ s/\)$/\nEND/g;

  $line =~ s/goal:[^:]*:(\d+)/TRUEgoal$1/;

  $line =~ s/ legal:[^:]*:/ AXN/;
  $line =~ s/^legal:[^:]*:/AXN/;
  $line =~ s/ next:/ TRUE:/;
  $line =~ s/^next:/TRUE:/;
  $line =~ s/ does:[^:]*:/ AXN/;
  $line =~ s/^does:[^:]*:/AXN/;
  $line =~ s/\?/V/g;
  $line =~ s/\)//; # this can happen after an OR
  print $line;
}

sub parenOpenCount() {
  my $line = shift;
  $count = 0;
  while ($line =~ s/\(//) {
    $count++;
  }
  return $count;
}
sub parenCloseCount() {
  my $line = shift;
  $count = 0;
  while ($line =~ s/\)//) {
    $count++;
  }
  return $count;
}
