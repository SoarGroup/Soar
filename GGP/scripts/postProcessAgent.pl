#!/usr/bin/perl
$ENV{"GGP_PATH"}="../";
$ENV{"PYTHONPATH"}="./pyparser/:.";

die unless ($#ARGV == 1);

our $kifFile = $ARGV[0];
if (not -e $kifFile) {
  die;
}

our $soarFile = $ARGV[1];
if (not -e $soarFile) {
  die;
}

$maFile = $kifFile;
$maFile =~ s/\.kif$/\.ma.soar/;
$maFile =~ s/\.unix//;

$makeStatic = "./makeStatic.pl $soarFile";
$analyzeKif = "./analyzeKif.pl $kifFile";
$arbGS = "./findArbitraryGSConstants.pl $kifFile";

if (-e $maFile) {

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
    push @bkProdContents, "$1";
    print "timeout counter: $1\n";
  }
}

@arbConstants = ();
foreach $line (`$arbGS`) {
  chomp $line;
  $line =~ /arbitrary (.*)/ or die "can't parse: $line\n";
  print "arbitrary game-state constant: $1\n";
  push @arbConstants, $1;
}

if ($#bkProdContents >= 0) {
  print `echo "sp {top-state*bk-counters" >> $soarFile`;
  print `echo "  (state <s> ^superstate nil ^facts <f>)" >> $soarFile`;
  print `echo "-->" >> $soarFile`;
  foreach $line (@bkProdContents) {
    print `echo "  (<f> ^bookkeeping-state $line)" >> $soarFile`;
  }
  print `echo "}" >> $soarFile`;
} 

if ($#bkProdContents >= 0) {
  if ($#bkProdContents > 0) {
    die "ERROR: state hashing can't handle multiple bk counters.\n";
  }
  print `echo "sp {top-state*counters" >> $soarFile`;
  print `echo "  (state <s> ^superstate nil ^facts <f>)" >> $soarFile`;
  print `echo "-->" >> $soarFile`;
  foreach $line (@bkProdContents) {
    print `echo "  (<f> ^counter $line)" >> $soarFile`;
  }
  print `echo "}" >> $soarFile`;
} 

#print `echo "sp {top-state*arbitrary-gs-constants" >> $soarFile`;
#print `echo "  (state <s> ^superstate nil)" >> $soarFile`;
#print `echo "-->" >> $soarFile`;
#print `echo "  (<s> ^arbitrary-gs-constants <ags>)" >> $soarFile`; 
#foreach $line (@arbConstants) {
#  print `echo "  (<ags> ^constant $line)" >> $soarFile`;
#}
#print `echo "}" >> $soarFile`;

#if ($kifFile =~ /build-/) {
  # env is build
#  print `python ./build_heuristic.py $kifFile >> $soarFile`;
#}
