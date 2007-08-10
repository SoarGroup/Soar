#!/usr/bin/perl

die unless ($#ARGV == 2);

our $kifFile = $ARGV[0];
if (not -e $kifFile) {
  die;
}

our $soarFile = $ARGV[1];
if (not -e $soarFile) {
  die;
}

$env = $ARGV[2];

$maFile = $kifFile;
$maFile =~ s/\.kif$/\.ma.soar/;

$makeStatic = "./makeStatic.pl $soarFile";
$analyzeKif = "./analyzeKif.pl $kifFile";

if (-e $maFile) {
  # print `echo "source $maFile" >> $soarFile`; must be at beginning

  $tmpFile = "bk_tmp";
  die if (-e $tmpFile);

  print `mv $soarFile $tmpFile`;
  print `echo "source $maFile" > $soarFile`;
  print `cat $tmpFile >> $soarFile`;
  print `rm $tmpFile`;
}

print `$makeStatic greaterthan`;
print `$makeStatic lessthan`;
print `$makeStatic gtequal`;
print `$makeStatic plus`;
print `$makeStatic minus`;
#print `$fixVar`;

@bkProdContents = ();
foreach $line (`$analyzeKif`) {
  chomp $line;
  if ($line =~ /static (.*)/) {
    print "static elaboration: $1\n";
    print `$makeStatic $1`;
  }
  elsif ($line =~ /fakegs (.*)/) {
    print "static game state: $1\n";
  #  print `$makeStatic $1`; FIXME
  }
  elsif ($line =~ /counter (.*)/) {
    push @bkProdContents, "^bookkeeping-state $1";
    print "timeout counter: $1\n";
  }
}

if ($#bkProdContents >= 0) {
  print `echo "sp {top-state*bk-counters" >> $soarFile`;
  print `echo "  (state <s> ^superstate nil ^facts <f>)" >> $soarFile`;
  print `echo "-->" >> $soarFile`;
  foreach $line (@bkProdContents) {
    print `echo "  (<f> $line)" >> $soarFile`;
  }
  print `echo "}" >> $soarFile`;
} 

