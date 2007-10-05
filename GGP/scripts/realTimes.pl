#!/usr/bin/perl

@row1 = ();
@row2 = ();
foreach $line (`cat $ARGV[0]`) {
  chomp $line;
  $indicFile = $line;
  $indicFile =~ s/log$/goodthings.soar/;
  if (-e $indicFile) {
    @results = `grep 'GEN TIME' $indicFile`;
    $total = 0;
    foreach $result (@results) {
      chomp $result;
      
      $result =~ /(\S+?) real,(\S+?) user,(\S+?) sys/ or die "!$result";
      $real = $1;
      $user = $2;
      $sys = $3;

      $real =~ /(\d+):(\S+)/ or die "$real!";
      $total += $1*60 + $2;
    }

    push @row1, $total;
  }
  else {
    push @row1, "no indicators generated";
  }

  $result = `grep 'UNIX TIME' $line`;
  chomp $result;
  
  $result =~ /(\S+?) real,(\S+?) user,(\S+?) sys/ or die "!$result";
  $real = $1;
  $user = $2;
  $sys = $3;

  $real =~ /(\d+):(\S+)/ or die "$real!";
  $real = $1*60 + $2;

  push @row2, $real;
  
}

print "creating scenario-real-times file\n";

open $OUT, ">scenario-real-times";

for ($i=0; $i<=$#row1; $i++) {
  print $OUT "$row1[$i]\t";
}
print $OUT "\n";
for ($i=0; $i<=$#row2; $i++) {
  print $OUT "$row2[$i]\t";
}
print $OUT "\n";
