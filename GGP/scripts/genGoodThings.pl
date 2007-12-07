#!/usr/bin/perl

our $rand =int( rand()*21515151551);
our $productionCount = 0;
our $currentIndex = 0;

our %additions = ();
our %removals = ();

die unless ($#ARGV == 4);
$logFile = $ARGV[0];
$sourceKif = $ARGV[1];
$targetKif = $ARGV[2];
$currentIndex = $ARGV[3];
$mapFile = $ARGV[4];
our $currentKey = $currentIndex + 1000;

#$timeFile = "map-times";
#$mapper = "/usr/bin/time -o $timeFile -f '%E real,%U user,%S sys' ./runMapper.pl";


checkFor($logFile);
#checkFor($sourceKif);
#checkFor($targetKif);

our %mappings = ();
#foreach $mapping (`$mapper $sourceKif $targetKif 2>/dev/null`) {
foreach $mapping (`cat $mapFile`) {  
  chomp $mapping;
  $mapping =~ /^map \w+ (\S*)\s+(\S*)$/ or die "can't parse: $mapping\n";
  $orig = $1;
  $new = $2;
  push @{ $mappings{$orig} }, $new;
  print "# MAPPING: $orig -> $new\n";
}

#foreach $line (`cat $timeFile`) {
#  print "# MAPPER TIME $line";
#}

foreach $line (`./findArbitraryGSConstants.pl $targetKif`) {
  $line =~ /arbitrary (\S+)/ or die "!$line!\n";
  push @arbitraryInTarget, $1;
}

foreach $line (`./findArbitraryGSConstants.pl $sourceKif`) {
  $line =~ /arbitrary (\S+)/;
  $arbitraryInSource = $1;
  foreach $constant (@arbitraryInTarget) {
    push @{ $mappings{$arbitraryInSource} }, $constant;
    print "# MAPPING: $arbitraryInSource -> $constant\n";
  }
}

#print `rm $timeFile`;

# build the additions and removals hashes for each set of additions and removals
# adjacent to one another
$changeCount = 0;
foreach $line (`cat $logFile`) {
  chomp $line;

  if ($line =~ /^ADDED (\S+) (.*)$/) {
    unless ($1 eq "location") {
      push @{ $additions{$1} }, $2;
      $changeCount++;
    }
  }
  elsif ($line =~ /^REMOVED (\S+) (.*)$/) {
    unless ($1 eq "location") {
      push @{ $removals{$1} }, $2;
      $changeCount++;
    }
  }
  elsif ($changeCount > 0) {
    buildIndicators();
    %additions = ();
    %removals = ();
    $changeCount = 0;
  }
}

# create indicators from additions and removals hashes
sub buildIndicators() {
  # if a predicate has a removal and addition differing only in numeric
  # parameters, increase / decrease indicators are generated
  #
  # if any non-numeric parameters differ, normal addition/removal indicators are
  # generated

  #$currentIndex++;
  #print "\n# INDEX $currentIndex:\n\n";
  foreach $predicate (keys %additions) {
    ADDITIONS: foreach $additionInstance (@{ $additions{$predicate} }) {
      $removalInstance =~ s/\. //; #fix some num problems: 68. == 68 (handle decimal in general?)
      $additionInstance =~ s/\. //;
      $removalInstance =~ s/\.$//; #fix some num problems: 68. == 68 (handle decimal in general?)
      $additionInstance =~ s/\.$//;
      foreach $removalInstance (@{ $removals{$predicate} }) {
        if (nonNumericEqual($removalInstance, $additionInstance)) {
          $anyIncDec = 0;
          foreach $numericArgNum (getNumericArgNums($additionInstance)) {      
            $currentIndex++;
            $removalInstance =~ /\^p$numericArgNum (\d+)/;
            $removedNum = $1;
            $additionInstance =~ /\^p$numericArgNum (\d+)/;
            $addedNum = $1;
            foreach $possiblePredicate (expandMappings($predicate)) {
              foreach $possibleAddition (expandMappings($additionInstance)) {
                # change indicator
                $currentKey++;
                $possibleAddition =~ s/^\^//;
                @groundingPairs = split '\^', $possibleAddition;
                printGtHeader();
                print "   (<in> ^index $currentIndex\n";
                print "         ^type change\n";
                print "         ^key $currentKey)\n";
                print "}\n";

                printDetectRuleHeader();
                print "    (<in> ^key $currentKey)\n";
                print "    (<gs> ^$possiblePredicate <predicate>)\n";
                print "    (<old-gs> ^$possiblePredicate <old-predicate>)\n";
                print "    (<predicate> ^p$numericArgNum $addedNum)\n";
                print "    (<old-predicate> ^p$numericArgNum $removedNum)\n";
                foreach $groundingPair (@groundingPairs) {
                  $groundingPair =~ /(\S+) (\S+)/ or die;
                  $position = $1;
                  $value = $2;
                  if ($position eq "p$numericArgNum") {
                    next;
                  }
                  print "    (<predicate> ^$position $value)\n";
                  print "    (<old-predicate> ^$position $value)\n";
                }
                printDetectRuleMiddle();
                print "   (write (crlf) |Indicator: change of $possiblePredicate p$numericArgNum from $removedNum to $addedNum|)\n";
                print "}\n";
              }
            }

            if ($addedNum > $removedNum) {
              foreach $possiblePredicate (expandMappings($predicate)) {
                foreach $possibleAddition (expandMappings($additionInstance)) {
                  # increase indicator
                  $currentKey++;
                  $possibleAddition =~ s/^\^//;
                  @groundingPairs = split '\^', $possibleAddition;
                  printGtHeader();
                  print "   (<in> ^index $currentIndex\n";
                  print "         ^type increase\n";
                  print "         ^key $currentKey)\n";
                  print "}\n";

                  printDetectRuleHeader();
                  print "    (<in> ^key $currentKey)\n";
                  print "    (<gs> ^$possiblePredicate <predicate>)\n";
                  print "    (<old-gs> ^$possiblePredicate <old-predicate>)\n";
                  print "    (<predicate> ^p$numericArgNum <new-num>)\n";
                  print "    (<old-predicate> ^p$numericArgNum {< <new-num> <old-num>})\n";
                  
                  foreach $groundingPair (@groundingPairs) {
                    $groundingPair =~ /(\S+) (\S+)/ or die;
                    $position = $1;
                    $value = $2;
                    if ($position eq "p$numericArgNum") {
                      next;
                    }
                    print "    (<predicate> ^$position $value)\n";
                    print "    (<old-predicate> ^$position $value)\n";
                  }
                  printDetectRuleMiddle();
                  print "   (write (crlf) |Indicator: increase of $possiblePredicate p$numericArgNum|)\n";
                  print "}\n";
                }
              }
              $anyIncDec = 1;
            }
            elsif ($removedNum > $addedNum) {
              foreach $possiblePredicate (expandMappings($predicate)) {
                foreach $possibleAddition (expandMappings($additionInstance)) {
                  # decrease indicator
                  $currentKey++;
                  $possibleAddition =~ s/^\^//;
                  @groundingPairs = split '\^', $possibleAddition;
                  printGtHeader();
                  print "   (<in> ^index $currentIndex\n";
                  print "         ^type decrease\n";
                  print "         ^key $currentKey)\n";
                  print "}\n";

                  printDetectRuleHeader();
                  print "    (<in> ^key $currentKey)\n";
                  print "    (<gs> ^$possiblePredicate <predicate>)\n";
                  print "    (<old-gs> ^$possiblePredicate <old-predicate>)\n";
                  print "    (<predicate> ^p$numericArgNum <new-num>)\n";
                  print "    (<old-predicate> ^p$numericArgNum {> <new-num> <old-num>})\n";
                  
                  foreach $groundingPair (@groundingPairs) {
                    $groundingPair =~ /(\S+) (\S+)/ or die;
                    $position = $1;
                    $value = $2;
                    if ($position eq "p$numericArgNum") {
                      next;
                    }
                    print "    (<predicate> ^$position $value)\n";
                    print "    (<old-predicate> ^$position $value)\n";
                  }
                  printDetectRuleMiddle();
                  print "   (write (crlf) |Indicator: decrease of $possiblePredicate p$numericArgNum|)\n";
                  print "}\n";
                }
              }
              $anyIncDec = 1;
            }
          }
          if ($anyIncDec) {
            # prevent the removal instance from being seen as its own indicator
            $removalInstance = "IS_ID";
            # similarly prevent the addition instance
            next ADDITIONS;
          }
        }
      }

      # addition instance is not an increment or decrement

      $currentIndex++;
      foreach $possiblePredicate (expandMappings($predicate)) {
        foreach $possibleAddition (expandMappings($additionInstance)) {
          $currentKey++;
          printGtHeader();
          print "   (<in> ^index $currentIndex\n";
          print "         ^type addition\n";
          print "         ^key $currentKey)\n";
          print "}\n";
          
          printDetectRuleHeader();
          print "    (<in> ^key $currentKey)\n";
          print "    (<gs> ^$possiblePredicate <predicate>)\n";
          print "    (<predicate> $possibleAddition)\n";
          print "  -{(<old-gs> ^$possiblePredicate <old-predicate>)\n";
          print "    (<old-predicate> $possibleAddition)}\n";
          printDetectRuleMiddle();
          print "   (write (crlf) |Indicator: appearence of $possiblePredicate $possibleAddition|)\n";
          print "}\n";
        }
      }
    }
  }

  foreach $predicate (keys %removals) {
    foreach $removalInstance (@{ $removals{$predicate} }) {
      unless ($removalInstance eq "IS_ID") {
        $currentIndex++;
        foreach $possiblePredicate (expandMappings($predicate)) {
          foreach $possibleRemoval (expandMappings($removalInstance)) {
            $currentKey++;
            printGtHeader();
            print "   (<in> ^index $currentIndex\n";
            print "         ^type removal\n";
            print "         ^key $currentKey)\n";
            print "}\n";
            
            printDetectRuleHeader();
            print "    (<in> ^key $currentKey)\n";
            print "  -{(<gs> ^$possiblePredicate <predicate>)\n";
            print "    (<predicate> $possibleRemoval)}\n";
            print "    (<old-gs> ^$possiblePredicate <old-predicate>)\n";
            print "    (<old-predicate> $possibleRemoval)\n";
            printDetectRuleMiddle();
            print "   (write (crlf) |Indicator: removal of $possiblePredicate $possibleRemoval|)\n";
            print "}\n";
          }
        }
      }
    }
  }
}

sub nonNumericEqual() {
  $first = shift;
  $second = shift;

  $first =~ s/\^p(\d+) (\d+)//g;
  $second =~ s/\^p(\d+) (\d+)//g;

  if ($first eq $second) {
    return 1;
  }
  return 0;
}

sub getNumericArgNums() {
  $list = shift;
  @results = ();

  for ($i=1; $i<10; $i++) {
    if ($list =~ /\^p$i (\d+)/) {
      push @results, $i;
    }
  }

  return @results;
}

sub printGtHeader() {
  $productionCount++;
  print "sp {elaborate*goodthing*$rand$productionCount\n";
  print "   (state <s> ^good-things <gt>)\n";
  print "-->\n";
  print "   (<gt> ^indicator <in>)\n";
}

sub printDetectRuleHeader() {
  $productionCount++;
  print "sp {apply*usi*indicator-detected*$rand$productionCount\n";
  print "   (state <s> ^name game\n";
#  print "   (state <s> ^crap crap\n";
  print "              ^operator.name update-search-info\n";
  print "              ^gs <gs>\n";
  print "              ^old-gs <old-gs>\n";
  print "              ^current-evaluation-depth <ced>\n";
  print "              ^top-state.good-things.indicator <in>\n";
  print "              -^used-goodthing <in>)\n";
}

sub printDetectRuleMiddle() {
  print "-->\n";
  print "   (<s> ^present-indicator <in>)\n";
  # take out this ced change to avoid bumping (but still detect)
  print "   (<s> ^current-evaluation-depth <ced> -\n";
  print "                                  (+ <ced> 1))\n";
}

sub expandMappings() {
  $string = shift;
  @results = ();
  our @originals = ();
  our @alternates = ();
  $i = 0;
  foreach $subString (split ' ', $string) {
    if ($subString =~ /\^/) {
      next;
    }

    if (defined $mappings{$subString}) {
      $originals[$i] = $subString;
      
      foreach $ssMapping (@{ $mappings{$subString} }) {
        push @{ $alternates[$i] }, $ssMapping;
      }
      $i++;
    }
  }

  foreach $mappingList (enumerateMappings(@alternates)) {
    @newNames = split ' ', $mappingList;
    die unless ($#newNames == ($i-1));
    $result = $string;
    for ($j=0; $j<$i; $j++) {
      #  print "s $originals[$j] | $newNames[$j] | $result\n";
      $result =~ s/$originals[$j]/$newNames[$j]/;
    }
    push @results, $result;
  }

  if ($#results == -1) {
    # there were no mappings
    push @results, $string;
  }
  return @results;
}

sub enumerateMappings() {
  my @results = ();
  if ($#_ == 0) {
    foreach $entry (@{ $_[0] }) {
      push @results, $entry;
    }
  }
  elsif ($#_ > 0) {
    my $firstList = shift @_;
    foreach $result (enumerateMappings(@_)) {
      foreach $firstEntry (@{ $firstList }) {
        push @results, "$firstEntry $result";
      }
    }
  }
  return @results;
}

sub checkFor() {
  $file = shift;
  die "$file does not exist" unless (-e $file);
}

