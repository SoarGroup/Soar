#!/usr/bin/perl
#
# Builds everything

use strict;

# Get top level directory:
# First, get working directory
my $tld = `pwd`;
chomp($tld);

# Next, chop off /SoarIO/dist if applicable, means update all was executed inside
# of SoarIO/dist
$_ = $tld;
if (/(.+)\/SoarIO\/dist\/?$/) {
  $tld = $1;
}

chdir $tld or die "Couldn't change to $tld";

sub process {
  print "$_[0]...";
  my $ret = system "$_[1]";
  if ($ret != 0) {
    die "$_[1] returned failure!";
  }
  print "\n";
}

process "Top level make", "make";

chdir "$tld/SoarIO/examples/TestJavaSML" or die;
process "TestJavaSML", "./buildJava.sh";

chdir "$tld/SoarJavaDebugger" or die;
process "SoarJavaDebugger", "./buildDebugger.sh";

chdir "$tld/JavaMissionaries" or die;
process "JavaMissionaries", "./buildmac.sh";

chdir "$tld/JavaTOH" or die;
process "JavaTOH", "./buildtoh.sh";

chdir "$tld/JavaBaseEnvironment" or die;
process "JavaBaseEnvironment", "./build.sh";

chdir "$tld/JavaTanksoar" or die;
process "JavaTanksoar", "./build.sh";

chdir "$tld/JavaEaters" or die;
process "JavaEaters", "./build.sh";

chdir "$tld/visualsoar" or die;
process "VisualSoar", "./buildVisualSoar.sh";

