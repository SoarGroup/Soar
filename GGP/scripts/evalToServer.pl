#!/usr/bin/perl

$timeFile = "tmp.timer";
$timeCommand = '/usr/bin/time -o ' . $timeFile . ' -f \'%E real,%U user,%S sys\' ';

@row1 = ();
@row2 = ();
@row3 = ();
@row4 = ();
foreach $line (`cat $ARGV[0]`) {
  chomp $line;
  print "sending log $line\n";
  @results = `$timeCommand ./GMSubmit.pl $line | grep Create`;
  chomp $results[0];
  print "$results[0]\n";
  $results[0] =~ /Created match (\d+)/ or die "!$results[0]";
  push @row1, $1;
  
  $timeLine = `cat $timeFile | head -n 1`;
  $timeLine =~ /(\S+?) real,(\S+?) user,(\S+?) sys/ or die "!$timeLine";

  $real = $1;
  $user = $2;
  $sys = $3;

  print "real time: $real";
  $real =~ /(\d+):(\S+)/ or die "$real!";
  $real = $1*60 + $2;

  print " = $real sec\n";

  push @row2, $user;
  push @row3, $user + $sys;
  push @row4, $real;
}

print `rm $timeFile`;
print "creating eval-server-results file\n";

open $OUT, ">eval-server-results";

for ($i=0; $i<=$#row1; $i++) {
  print $OUT "$row1[$i]\t";
}
print $OUT "\n";
for ($i=0; $i<=$#row2; $i++) {
  print $OUT "$row2[$i]\t";
}
print $OUT "\n";
for ($i=0; $i<=$#row3; $i++) {
  print $OUT "$row3[$i]\t";
}
print $OUT "\n";
for ($i=0; $i<=$#row4; $i++) {
  print $OUT "$row4[$i]\t";
}
print $OUT "\n";



