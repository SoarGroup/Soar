#!/usr/bin/perl

%done = ();
$i = 0;
foreach $line (`cat $ARGV[0]`) {
  chomp $line;
  if ($line =~ s/^ADDED //) {
    if (defined $done{$line}) {
      next;
    }
    $done{$line} = 1;
    if ($line =~ /(\S+) \^p1 (\S+)$/) {
      $att = $1;
      $p1 = $2;
      $i++;
      print "sp {elaborate*goodthing*$i\n";
      print "   (state <s> ^good-things <gt>)\n";
      print "-->\n";
      print "   (<gt> ^$1 <id>)\n";
      print "   (<id> ^p1 $p1)\n";
      print "}\n";
    }
    elsif ($line =~ /(\S+) \^p1 (\S+) \^p2 (\S+)$/) {
      $att = $1;
      $p1 = $2;
      $p2 = $3;
      $i++;
      print "sp {elaborate*goodthing*$i\n";
      print "   (state <s> ^good-things <gt>)\n";
      print "-->\n";
      print "   (<gt> ^$1 <id>)\n";
      print "   (<id> ^p1 $p1 ^p2 $p2)\n";
      print "}\n";
    }
    elsif ($line =~ /(\S+) \^p1 (\S+) \^p2 (\S+) \^p3 (\S+)$/) {
      $att = $1;
      $p1 = $2;
      $p2 = $3;
      $p3 = $4;
      $i++;
      print "sp {elaborate*goodthing*$i\n";
      print "   (state <s> ^good-things <gt>)\n";
      print "-->\n";
      print "   (<gt> ^$1 <id>)\n";
      print "   (<id> ^p1 $p1 ^p2 $p2 ^p3 $p3)\n";
      print "}\n";
    }
    else {
      die "can't parse ADDED: $line\n";
    }
  }
}
