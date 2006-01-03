#!/usr/bin/perl
#
# Generate distribution file
# Author: Jonathan Voigt voigtjr@gmail.com
# Date: November 2005
# 

use strict;
use File::Path;
use File::Copy;
use File::Remove qw(remove);

if ($#ARGV != 4) {
  print "Usage: sf_user dev dist version tag\n";
  print "  sf_user   username on sourceforge.net\n";
  print "  dev       full path to development installation\n";
  print "  dist      full path to destination for distribution, contents overwritten!\n";
  print "            dist path will be destroyed! rm -rf dist!\n";
  print "  version   foo-bar produces Soar-Suite-foo-bar\n";
  print "  tag       cvs tag to export.  HEAD is for latest\n";
  print "\n";
  print "I do development in /home/voigtjr/soar/dev, so I would pass that for dev\n";
  print "I'd like the dist files out of that same soar directory, so I would pass\n";
  print "/home/voigtjr/soar/dist for dist.\n";
  exit 0;
}

my $sf_user = $ARGV[0];	# sourceforge username
my $dev     = $ARGV[1]; # dev directory
my $dist    = $ARGV[2]; # dist directory
my $version = $ARGV[3]; # version string
my $tag     = $ARGV[4]; # cvs revision

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
);

my @distfiles = 
(
  "Makefile.am",
  "build-everything.pl",
  "configure.ac",
  "install-sh",
  "missing",
);

# Flush dir
rmtree "$dist/Soar-Suite-$version";
mkdir "$dist/Soar-Suite-$version" or die "Could not create new package dir: $dist/Soar-Suite-$version";
chdir "$dist/Soar-Suite-$version" or die "Could not change to package dir: $dist/Soar-Suite-$version";

# Check out modules
foreach my $module (@modules) {
  my $ret = system "cvs -Q -d /cvsroot/soar export -r $tag $module";
  if ($ret != 0) {
    die "Error: cvs returned error for module $module";
  }
}

my $ret = system "cvs -Q -d :ext:$sf_user\@cvs.sourceforge.net:/cvsroot/soar export -r $tag visualsoar";
if ($ret != 0) {
  die "Error: cvs returned error for module visualsoar";
}

# Move dist files to top level
foreach my $distfile (@distfiles) {
  move("$dist/Soar-Suite-$version/SoarIO/dist/$distfile", "$dist/Soar-Suite-$version") or die;
}
rmtree "$dist/Soar-Suite-$version/SoarIO/dist";

# Copy top level docs
copy("$dist/Soar-Suite-$version/SoarIO/COPYING", "$dist/Soar-Suite-$version/license.txt");
copy("$dist/Soar-Suite-$version/Documentation/announce.txt", "$dist/Soar-Suite-$version");

# Initialize configure scripts (this recurses)
chdir "$dist/Soar-Suite-$version" or die;
$ret = system "libtoolize --copy --force 2>/dev/null";
if ($ret != 0) { die "libtoolize failed!"; }
$ret = system "autoreconf 2>/dev/null";
if ($ret != 0) { die "Autoreconf failed!"; }
      
# Add binaries and data
mkdir "$dist/Soar-Suite-$version/soar-library/mac" or die;
copy("$dev/soar-library/mac/mac.soar", "$dist/Soar-Suite-$version/soar-library/mac") or die;

# Remove unwanted files
chdir "$dist/Soar-Suite-$version/soar-library" or die;
remove qw( *.dll java_swt libswt-carbon-* libswt-pi-carbon-* libswt-webkit-carbon-* swt-carbon.jar swt-pi-carbon.jar );

# Create the tarball
chdir "$dist" or die;
$ret = system "tar cfj Soar-Suite-$version.tar.bz2 Soar-Suite-$version";
