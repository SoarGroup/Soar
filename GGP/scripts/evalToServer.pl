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
  $matchID = $1;

  $timeLog = $line;
  $timeLog =~ s/log$/submit/;

  push @row1, $matchID;
  print `echo $matchID >> $timeLog`;
  
  $timeLine = `cat $timeFile | head -n 1`;
  $timeLine =~ /(\S+?) real,(\S+?) user,(\S+?) sys/ or die "!$timeLine";

  $real = realSeconds($1);
  $user = $2;
  $sys = $3;

  push @row2, $user;
  print `echo $user >> $timeLog`;
  $userSys = $user + $sys;
  push @row3, $userSys;
  print `echo $userSys >> $timeLog`;
  push @row4, $real;
  print `echo $real >> $timeLog`;
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

sub realSeconds() {
  # the format for all calls to unix time command is '%E real,%U user,%S sys' 
  # real times are reported in (hours:)minutes:seconds format, 
  # need to convert to seconds (I should have used a %e instead of %E)
  # all other times are reported in seconds (see man time)
  $string = shift;
  if ($string =~ /(\d+):(\d+):(\d+)/) {
    return 3600*$1 + 60*$2 + $3;
  }
  elsif ($string =~ /(\d+):(\S+)/) {
    return 60*$1 + $2;
  }
  die;
}
