#!/usr/bin/perl

die "args: gets random problems that have solutions from the output of genProbs.py, where the problem is solved in both file1 and file2, and has a length of at least min_length.\nparams:file1 file2 min_length num_problems" 
unless ($#ARGV == 3);

$randomizelines = "/home/swinterm/GGP/pysrc/randomize-lines";

$file1 = $ARGV[0];
$file2 = $ARGV[1];
$minLength = $ARGV[2];
$numProblems = $ARGV[3];

die unless (-e $file1);
die unless (-e $file2);

$tempFile1 = "tmp_grpws1";
$tempFile2 = "tmp_grpws2";
$tempFile3 = "tmp_grpws3";

die if (-e $tempFile1);
die if (-e $tempFile2);
die if (-e $tempFile3);

open $IN, "<$file1";
open $OUT, ">$tempFile1";

foreach $line (<$IN>) {
  next if ($line =~ /unsolvable/);
  next if ($line =~ /depth/);
#eg: /home/swinterm/GGP/kif/R02_mod/mummymaze1p-horiz.kif 1 1 2 6 - 1 north

  $line =~ s/^\S* // or die;
  $line =~ /\d+ \d+ \d+ \d+ - (\d+)/ or die;

  if ($1 >= $minLength) {
    print $OUT $line;
  }
}

close $IN;
close $OUT;

open $IN, "<$file2";
open $OUT, ">$tempFile2";

foreach $line (<$IN>) {
  next if ($line =~ /unsolvable/);
  next if ($line =~ /depth/);
#eg: /home/swinterm/GGP/kif/R02_mod/mummymaze1p-horiz.kif 1 1 2 6 - 1 north

  $line =~ s/^\S* // or die;
  $line =~ /\d+ \d+ \d+ \d+ - (\d+)/ or die;

  if ($1 >= $minLength) {
    print $OUT $line;
  }
}

close $IN;
close $OUT;

open $IN, "<$tempFile1";
open $OUT, ">$tempFile3";

foreach $line (<$IN>) {
  $line =~ /(^\d+ \d+ \d+ \d+)/ or die;
  $instance = $1;
  if (`grep \'$instance\' $tempFile2`) { # instance is in both files
    print $OUT "$instance\n";
  }
}

close $IN;
close $OUT;

print `cat $tempFile3 | $randomizelines | head -n $numProblems`;

print `rm $tempFile1`;
print `rm $tempFile2`;
print `rm $tempFile3`;
