#!/usr/bin/perl

our $rand =int( rand()*21515151551);
our $productionCount = 0;
our $currentIndex = 0;

our %additions = ();
our %removals = ();

die unless ($#ARGV == 3);
$logFile = $ARGV[0];
$sourceKif = $ARGV[1];
$targetKif = $ARGV[2];
$currentIndex = $ARGV[3];

$timeFile = "map-times";
$mapper = "/usr/bin/time -o $timeFile -f '%E real,%U user,%S sys' ./runMapper.pl";


checkFor($logFile);
checkFor($sourceKif);
checkFor($targetKif);

our %mappings = ();
foreach $mapping (`$mapper $sourceKif $targetKif 2>/dev/null`) {
  chomp $mapping;
  $mapping =~ /^map \w+ (\S*)\s+(\S*)$/ or die "can't parse: $mapping\n";
  $orig = $1;
  $new = $2;
  push @{ $mappings{$orig} }, $new;
  unless ($orig =~ /^$new$/) {
    print "# MAPPING: $orig -> $new\n";
  }
}

foreach $line (`cat $timeFile`) {
  print "# MAPPER TIME $line";
}

print `rm $timeFile`;

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
                printGtHeader();
                print "   (<in> ^index $currentIndex\n";
                print "         ^type change\n";
                print "         ^predicate-name $possiblePredicate\n";
                print "         ^number-position p$numericArgNum\n";
                print "         ^from $removedNum\n";
                print "         ^to $addedNum)\n";
                $possibleAddition =~ s/^\^//;
                @groundingPairs = split '\^', $possibleAddition;
                @positionWords = ("first", "second", "third");
                $groundingNum = 0;
                foreach $groundingPair (@groundingPairs) {
                  $groundingPair =~ /(\S+) (\S+)/ or die;
                  $position = $1;
                  if ($position eq "p$numericArgNum") {
                    next;
                  }
                  $value = $2;
                  print "   (<in> ^$positionWords[$groundingNum]-grounding <g$groundingNum>)\n";
                  print "   (<g$groundingNum> ^position $position ^value $value)\n";
                  $groundingNum++;
                }

                print "}\n";
              }
            }

            if ($addedNum > $removedNum) {
              foreach $possiblePredicate (expandMappings($predicate)) {
                foreach $possibleAddition (expandMappings($additionInstance)) {
                  # increase indicator
                  printGtHeader();
                  print "   (<in> ^index $currentIndex\n";
                  print "         ^type increase\n";
                  print "         ^predicate-name $possiblePredicate\n";
                  print "         ^number-position p$numericArgNum)\n";
                  $possibleAddition =~ s/^\^//;
                  @groundingPairs = split '\^', $possibleAddition;
                  @positionWords = ("first", "second", "third");
                  $groundingNum = 0;
                  foreach $groundingPair (@groundingPairs) {
                    $groundingPair =~ /(\S+) (\S+)/ or die;
                    $position = $1;
                    if ($position eq "p$numericArgNum") {
                      next;
                    }
                    $value = $2;
                    print "   (<in> ^$positionWords[$groundingNum]-grounding <g$groundingNum>)\n";
                    print "   (<g$groundingNum> ^position $position ^value $value)\n";
                    $groundingNum++;
                  }

                  print "}\n";
                  
                }
              }
              $anyIncDec = 1;
            }
            elsif ($removedNum > $addedNum) {
              foreach $possiblePredicate (expandMappings($predicate)) {
                foreach $possibleAddition (expandMappings($additionInstance)) {
                  # decrease indicator
                  printGtHeader();
                  print "   (<in> ^index $currentIndex\n";
                  print "         ^type decrease\n";
                  print "         ^predicate-name $possiblePredicate\n";
                  print "         ^number-position p$numericArgNum)\n";
                  $possibleAddition =~ s/^\^//;
                  @groundingPairs = split '\^', $possibleAddition;
                  @positionWords = ("first", "second", "third");
                  $groundingNum = 0;
                  foreach $groundingPair (@groundingPairs) {
                    $groundingPair =~ /(\S+) (\S+)/ or die "bad GP: $groundingPair\n";
                    $position = $1;
                    if ($position eq "p$numericArgNum") {
                      next;
                    }
                    $value = $2;
                    print "   (<in> ^$positionWords[$groundingNum]-grounding <g$groundingNum>)\n";
                    print "   (<g$groundingNum> ^position $position ^value $value)\n";
                    $groundingNum++;
                  }

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
          printGtHeader();
          print "   (<in> ^index $currentIndex\n";
          print "         ^type addition\n";
          print "         ^what <w>)\n";
          print "   (<w>  ^$possiblePredicate <id>)\n";
          print "   (<id> $possibleAddition)\n";
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
            printGtHeader();
            print "   (<in> ^index $currentIndex\n";
            print "         ^type removal\n";
            print "         ^what <w>)\n";
            print "   (<w>  ^$possiblePredicate <id>)\n";
            print "   (<id> $possibleRemoval)\n";
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

