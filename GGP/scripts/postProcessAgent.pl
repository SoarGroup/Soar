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

%src_depths = ("build-7-1", 7,
  "build-7-2", 5,
  "build-7-3", 7,
  "build-8-1", 5,
  "differing-10-11", 14,
  "differing-10-12", 14,
  "differing-10-13", 14,
  "differing-10-14", 13,
  "differing-10-15", 16,
  "differing-10-16", 14,
  "differing-10-17", 16,
  "differing-10-18", 11,
  "differing-10-1", 14,
  "differing-10-2", 14,
  "differing-10-3", 14,
  "differing-10-4", 13,
  "differing-10-5", 16,
  "differing-10-6", 14,
  "differing-10-7", 16,
  "differing-10-8", 11,
  "escape-6-1", 17,
  "escape-6-2", 18,
  "escape-7-1", 14,
  "escape-7-2", 14,
  "escape-8-1", 16,
  "escape-8-2", 16,
  "escape-9-1", 19,
  "mrogue-6-1", 12,
  "mrogue-6-2", 12,
  "mrogue-6-3", 14,
  "mrogue-8-1", 14,
  "mrogue-8-2", 13,
  "mrogue-9-1", 12,
  "mrogue-9-2", 12,
  "wargame-6-1", 10,
  "wargame-6-2", 14,
  "wargame-7-1", 13,
  "wargame-7-2", 18,
  "wargame-8-1", 14,
  "wargame-8-2", 13,
  "wargame-9-1", 13,
  "wargame-9-2", 18);

%tgt_depths = ("build-7-1", 7,
"build-7-2", 5,
"build-7-3", 7,
"build-8-1", 5,
"differing-10-11", 11,
"differing-10-12", 10,
"differing-10-13", 14,
"differing-10-14", 7,
"differing-10-15", 12,
"differing-10-16", 14,
"differing-10-17", 13,
"differing-10-18", 8,
"differing-10-1", 11,
"differing-10-2", 10,
"differing-10-3", 14,
"differing-10-4", 7,
"differing-10-5", 12,
"differing-10-6", 14,
"differing-10-7", 13,
"differing-10-8", 8,
"escape-6-1", 20,
"escape-6-2", 20,
"escape-7-1", 14,
"escape-7-2", 15,
"escape-8-1", 14,
"escape-8-2", 8,
"escape-9-1", 19,
"mrogue-6-1", 11,
"mrogue-6-2", 9,
"mrogue-6-3", 11,
"mrogue-8-1", 12,
"mrogue-8-2", 13,
"mrogue-9-1", 12,
"mrogue-9-2", 12,
"wargame-6-1", 7,
"wargame-6-2", 11,
"wargame-7-1", 11,
"wargame-7-2", 11,
"wargame-8-1", 8,
"wargame-8-2", 9,
"wargame-9-1", 13,
"wargame-9-2", 18);

##$soarFile =~ /((build|differing|escape|mrogue|wargame)-\d+-\d+)/;
##
##if (defined $src_depths{$1}) {
##  $depth = $src_depths{$1};
##  $depth_prod="
##sp {elaborate*start-depth
##  (state <s> ^superstate nil)
##-->
##(<s> ^start-depth $depth)}\n";
##
##  open(AGENT, ">>$soarFile");
##  print AGENT $depth_prod;
##  close AGENT;
##}
##else {
##  `echo "Depth for $1 not found" >> /tmp/GGP-errors`;
##}
