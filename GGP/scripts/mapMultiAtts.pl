#!/usr/bin/perl

%done = ();

die unless ($#ARGV == 1);
$sourceKif = $ARGV[0];
$targetKif = $ARGV[1];
$headerFile = "../agents/header.soar";

$mapper = "./runMapper.pl";
$targetMAs = $targetKif;
$targetMAs =~ s/\.kif$/\.ma.soar/; 
clearLog($targetMAs);

checkFor($sourceKif);
checkFor($targetKif);
checkFor($mapper);
open $OUT, ">$targetMAs" or die;

%mappings = ();
foreach $mapping (`$mapper $sourceKif $targetKif`) {
  next if ($mapping =~ /\*/);
  print "LINE: $mapping\n";
  chomp $mapping;
  $mapping =~ /^map \w+ (\S*)\s+(\S*)$/ or die "can't parse: $mapping\n";
  $orig = $1;
  $new = $2;
  $mappings{$orig} = $new;
  
  @multiAtts = `grep "multi-attributes $orig " $headerFile`;

  if ($#multiAtts > 0) {
    die;
  }

  if ($#multiAtts == 0) {
    $multiAtt = $multiAtts[0];
    chomp $multiAtt;
    $multiAtt =~ /multi-attributes $orig (.*)$/;
    print $OUT "multi-attributes $new $1\n";
  }
}


sub checkFor() {
  $file = shift;
  die "$file does not exist" unless (-e $file);
}

sub clearLog() {
  $log = shift;
  if (-e $log) {
    print "$log exists, ctrl-c now or it gets deleted!\n";
    $foo = <STDIN>;
    print `rm $log`;
  }
}

