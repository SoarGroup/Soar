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

print `echo "sp {top-state*arbitrary-gs-constants" >> $soarFile`;
print `echo "  (state <s> ^superstate nil)" >> $soarFile`;
print `echo "-->" >> $soarFile`;
print `echo "  (<s> ^arbitrary-gs-constants <ags>)" >> $soarFile`; 
foreach $line (@arbConstants) {
  print `echo "  (<ags> ^constant $line)" >> $soarFile`;
}
print `echo "}" >> $soarFile`;

#if ($kifFile =~ /build-/) {
  # env is build
#  print `python ./build_heuristic.py $kifFile >> $soarFile`;
#}

%src_depths = ("../agents/build-7-1-target.merge.soar", 7,
  "../agents/build-7-2-target.merge.soar", 5,
  "../agents/build-7-3-target.merge.soar", 7,
  "../agents/build-8-1-target.merge.soar", 5,
  "../agents/differing-10-11-target.unix.soar", 14,
  "../agents/differing-10-12-target.unix.soar", 14,
  "../agents/differing-10-13-target.unix.soar", 14,
  "../agents/differing-10-14-target.unix.soar", 13,
  "../agents/differing-10-15-target.unix.soar", 16,
  "../agents/differing-10-16-target.unix.soar", 14,
  "../agents/differing-10-17-target.unix.soar", 16,
  "../agents/differing-10-18-target.unix.soar", 11,
  "../agents/differing-10-1-target.unix.soar", 14,
  "../agents/differing-10-2-target.unix.soar", 14,
  "../agents/differing-10-3-target.unix.soar", 14,
  "../agents/differing-10-4-target.unix.soar", 13,
  "../agents/differing-10-5-target.unix.soar", 16,
  "../agents/differing-10-6-target.unix.soar", 14,
  "../agents/differing-10-7-target.unix.soar", 16,
  "../agents/differing-10-8-target.unix.soar", 11,
  "../agents/escape-6-1-target.soar", 17,
  "../agents/escape-6-2-target.soar", 18,
  "../agents/escape-7-1-target.unix.soar", 14,
  "../agents/escape-7-2-target.unix.soar", 14,
  "../agents/escape-8-1-target.unix.soar", 16,
  "../agents/escape-8-2-target.unix.soar", 16,
  "../agents/escape-9-1-target.unix.soar", 19,
  "../agents/mrogue-6-1-target.soar", 12,
  "../agents/mrogue-6-2-target.soar", 12,
  "../agents/mrogue-6-3-target.soar", 14,
  "../agents/mrogue-8-1-target.unix.soar", 14,
  "../agents/mrogue-8-2-target.unix.soar", 13,
  "../agents/mrogue-9-1-target.unix.soar", 12,
  "../agents/mrogue-9-2-target.unix.soar", 12,
  "../agents/wargame-6-1-target.soar", 10,
  "../agents/wargame-6-2-target.soar", 14,
  "../agents/wargame-7-1-target.unix.soar", 13,
  "../agents/wargame-7-2-target.unix.soar", 18,
  "../agents/wargame-8-1-target.unix.soar", 14,
  "../agents/wargame-8-2-target.unix.soar", 13,
  "../agents/wargame-9-1-target.unix.soar", 13,
  "../agents/wargame-9-2-target.unix.soar", 18);

%tgt_depths = ("build-7-1-target.merge.soar", 7,
"../agents/build-7-2-target.merge.soar", 5,
"../agents/build-7-3-target.merge.soar", 7,
"../agents/build-8-1-target.merge.soar", 5,
"../agents/differing-10-11-target.unix.soar", 11,
"../agents/differing-10-12-target.unix.soar", 10,
"../agents/differing-10-13-target.unix.soar", 14,
"../agents/differing-10-14-target.unix.soar", 7,
"../agents/differing-10-15-target.unix.soar", 12,
"../agents/differing-10-16-target.unix.soar", 14,
"../agents/differing-10-17-target.unix.soar", 13,
"../agents/differing-10-18-target.unix.soar", 8,
"../agents/differing-10-1-target.unix.soar", 11,
"../agents/differing-10-2-target.unix.soar", 10,
"../agents/differing-10-3-target.unix.soar", 14,
"../agents/differing-10-4-target.unix.soar", 7,
"../agents/differing-10-5-target.unix.soar", 12,
"../agents/differing-10-6-target.unix.soar", 14,
"../agents/differing-10-7-target.unix.soar", 13,
"../agents/differing-10-8-target.unix.soar", 8,
"../agents/escape-6-1-target.soar", 20,
"../agents/escape-6-2-target.soar", 20,
"../agents/escape-7-1-target.unix.soar", 14,
"../agents/escape-7-2-target.unix.soar", 15,
"../agents/escape-8-1-target.unix.soar", 14,
"../agents/escape-8-2-target.unix.soar", 8,
"../agents/escape-9-1-target.unix.soar", 19,
"../agents/mrogue-6-1-target.soar", 11,
"../agents/mrogue-6-2-target.soar", 9,
"../agents/mrogue-6-3-target.soar", 11,
"../agents/mrogue-8-1-target.unix.soar", 12,
"../agents/mrogue-8-2-target.unix.soar", 13,
"../agents/mrogue-9-1-target.unix.soar", 12,
"../agents/mrogue-9-2-target.unix.soar", 12,
"../agents/wargame-6-1-target.soar", 7,
"../agents/wargame-6-2-target.soar", 11,
"../agents/wargame-7-1-target.unix.soar", 11,
"../agents/wargame-7-2-target.unix.soar", 11,
"../agents/wargame-8-1-target.unix.soar", 8,
"../agents/wargame-8-2-target.unix.soar", 9,
"../agents/wargame-9-1-target.unix.soar", 13,
"../agents/wargame-9-2-target.unix.soar", 18);

$depth_prod="
sp {elaborate*start-depth
   (state <s> ^superstate nil)
-->
   (<s> ^start-depth $tgt_depths{$soarFile})}";

open(AGENT, ">>$soarFile");
print AGENT $depth_prod;
close AGENT;
