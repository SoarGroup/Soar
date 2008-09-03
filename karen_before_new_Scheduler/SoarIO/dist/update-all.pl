#!/usr/bin/perl
#
# Update everything, developer script
# Author: Jonathan Voigt voigtjr@gmail.com
# Date: November 2005
#
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

my @modules =
(
  "SoarKernel",
  "gSKI",
  "SoarIO",
  "soar-library",
  "SoarJavaDebugger",
  "JavaMissionaries",
  "JavaTOH",
  "JavaBaseEnvironment",
  "JavaTanksoar",
  "JavaEaters",
  "visualsoar",
);

# update all modules
foreach my $module (@modules) {
  print $module . " ...";
  chdir $tld . "/" . $module or die " Error: Couldn't change to $module";
  my $ret = system "cvs -q update";
  if ($ret != 0) {
    die " Error: cvs returned error for $module";
  }
  print "\n";
}
