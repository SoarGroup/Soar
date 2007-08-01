#!/usr/bin/perl

our $file = $ARGV[0];
if (not -e $file) {
  die "first arg is file\n";
}

$tmpFileName = "fakemath_out";
die if (-e $tmpFileName);
open $FILE, "<$file";
open $OUT, ">$tmpFileName";

die if ($#ARGV != 0);

@ints = ();

foreach $line (<$FILE>) {
  chomp $line;
  $line = lc $line;
  $line =~ s/\(\+/\(plus/g;
  $line =~ s/\(\-/\(minus/g;
  $line =~ s/\(\>\=/\(gtequal/g;
  $line =~ s/\(\>/\(greaterthan/g;
  $line =~ s/\(\< /\(lessthan /g;
  print $OUT "$line\n";

  if ($line =~ /\s*\(int (\d+)\)/) {
    push @ints, $1;
   # print "found an int: $1\n";
  }
}
#print `cp $tmpFileName fakemath_int.kif`;

foreach $a (@ints) {
  foreach $b (@ints) {
    $c = $a + $b;
    if (grep /^$c$/, @ints) {
      print $OUT "(plus $a $b $c)\n";
    }
    $c = $a - $b;
    if (grep /^$c$/, @ints) {
      print $OUT "(minus $a $b $c)\n";
    }
    if ($a < $b) {
      print $OUT "(lessthan $a $b)\n";
    }
    if ($a > $b) {
      print $OUT "(greaterthan $a $b)\n";
    }
    if ($a >= $b) {
      print $OUT "(gtequal $a $b)\n";
    }
  }
}

print `mv $tmpFileName $file`;
