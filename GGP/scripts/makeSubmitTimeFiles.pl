#!/usr/bin/perl

@logs = `cat $ARGV[0]`;
@numbers = `cat $ARGV[1]`;
@entry0 = split /\s+/, $numbers[0];
@entry1 = split /\s+/, $numbers[1];
@entry2 = split /\s+/, $numbers[2];
@entry3 = split /\s+/, $numbers[3];

$i=0;
foreach $log (@logs) {
  $timeFile = $log;
  $timeFile =~ s/log$/submit/;
  print `echo $entry0[$i] >> $timeFile`;
  print `echo $entry1[$i] >> $timeFile`;
  print `echo $entry2[$i] >> $timeFile`;
  print `echo $entry3[$i] >> $timeFile`;
  $i++;
}
