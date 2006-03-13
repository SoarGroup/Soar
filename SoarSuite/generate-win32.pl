#!/usr/bin/perl
#
# New installer generation script.
#
# Assumptions:
# Run in Cygwin, svn in path

use strict;
use File::Find::Rule;
use File::Copy;

# Variables
my $soarurl = "https://winter.eecs.umich.edu/svn/soar/trunk/SoarSuite";
my $distdir = "SoarSuiteDist";
my @removed = qw/.cvsignore *.so *.so.2 *.jnilib java_swt make-mac-app.sh *.plist *.doc *.ppt/;
my @bincopies = qw/*.dll *.exe *.jar *.lib Tcl_sml_ClientInterface/;

# Step 1: Rebuild everything
system "rebuild-all.bat";

# Step 2: Check out source from SVN
`svn export $soarurl ../$distdir`;

# Step 3: Remove files that are not to be distributed with the release
my @files = File::Find::Rule->file()->name( @removed )->in('../$distdir');

foreach (@files) {
	unlink $_ or die "Unable to remove $_: $!";
}

# Step 4: Copy binaries over to dist
foreach (@bincopies) {
	copy("SoarLibrary/bin/$_", "../%distdir%/SoarLibrary/bin/$_") or die "Unable to copy $_: $!";
}

