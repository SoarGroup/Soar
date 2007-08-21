#!/usr/bin/perl

%done = ();
$i = 0;
$rand =int( rand()*21515151551);

die unless ($#ARGV == 2);
$logFile = $ARGV[0];
$sourceKif = $ARGV[1];
$targetKif = $ARGV[2];
$headerFile = "../agents/header.soar";

$mapper = "./runMapper.pl";

checkFor($logFile);
checkFor($sourceKif);
checkFor($targetKif);
checkFor($mapper);

%mappings = ();
foreach $mapping (`$mapper $sourceKif $targetKif`) {
  chomp $mapping;
  $mapping =~ /^map \w+ (\S*)\s+(\S*)$/ or die "can't parse: $mapping\n";
  $orig = $1;
  $new = $2;
  $mappings{$orig} = $new;
  print "# MAPPING: $orig -> $new\n";
  
  #@multiAtts = `grep "multi-attributes $orig " $headerFile`;

  #if ($#multiAtts > 0) {
  #  die;
  #}

  #if ($#multiAtts == 0) {
  #  $multiAtt = $multiAtts[0];
  #  chomp $multiAtt;
  #  $multiAtt =~ /multi-attributes $orig (.*)$/;
  #  print "multi-attributes $new $1\n";
  #}
}

foreach $line (`cat $logFile`) {
  chomp $line;
  if ($line =~ s/^ADDED //) {
    if (defined $done{$line}) {
      next;
    }
    $done{$line} = 1;
    if ($line =~ /^step /) {
      next;
    }
    if ($line =~ /(\S+) \^p1 (\S+)$/) {
      $att = $1;
      $p1 = $2;
      if (defined $mappings{$att}) {
        $att = $mappings{$att};
      }
      if (defined $mappings{$p1}) {
        $p1 = $mappings{$p1};
      }
      $i++;
      print "sp {elaborate*goodthing*$rand$i\n";
      print "   (state <s> ^good-things <gt>)\n";
      print "-->\n";
      print "   (<gt> ^$att <id>)\n";
      print "   (<id> ^p1 $p1)\n";
      print "}\n";
    }
    elsif ($line =~ /(\S+) \^p1 (\S+) \^p2 (\S+)$/) {
      $att = $1;
      $p1 = $2;
      $p2 = $3;
      if (defined $mappings{$att}) {
        $att = $mappings{$att};
      }
      if (defined $mappings{$p1}) {
        $p1 = $mappings{$p1};
      }
      if (defined $mappings{$p2}) {
        $p2 = $mappings{$p2};
      }
      $i++;
      print "sp {elaborate*goodthing*$rand$i\n";
      print "   (state <s> ^good-things <gt>)\n";
      print "-->\n";
      print "   (<gt> ^$att <id>)\n";
      print "   (<id> ^p1 $p1 ^p2 $p2)\n";
      print "}\n";
    }
    elsif ($line =~ /(\S+) \^p1 (\S+) \^p2 (\S+) \^p3 (\S+)$/) {
      $att = $1;
      $p1 = $2;
      $p2 = $3;
      $p3 = $4;
      if (defined $mappings{$att}) {
        $att = $mappings{$att};
      }
      if (defined $mappings{$p1}) {
        $p1 = $mappings{$p1};
      }
      if (defined $mappings{$p2}) {
        $p2 = $mappings{$p2};
      }
      if (defined $mappings{$p3}) {
        $p3 = $mappings{$p3};
      }
      $i++;
      if (not $att =~ /location/) {
        print "sp {elaborate*goodthing*$rand$i\n";
        print "   (state <s> ^good-things <gt>)\n";
        print "-->\n";
        print "   (<gt> ^$att <id>)\n";
        print "   (<id> ^p1 $p1 ^p2 $p2 ^p3 $p3)\n";
        print "}\n";
      }
    }
    else {
      die "can't parse ADDED: $line\n";
    }
  }
}

sub checkFor() {
  $file = shift;
  die "$file does not exist" unless (-e $file);
}
