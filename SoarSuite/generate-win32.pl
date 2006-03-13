#!/usr/bin/perl
#
# New installer generation script.
#
# Assumptions:
# Run in Cygwin, svn in path

use strict;
use File::Find::Rule;
use File::Copy::Recursive "dircopy";
use File::Copy;

# Variables
my $soarurl = "https://winter.eecs.umich.edu/svn/soar/trunk/SoarSuite";
my $distdir = "SoarSuiteDist";
my @removed = qw/.cvsignore *.so *.so.2 *.jnilib java_swt make-mac-app.sh *.plist *.doc *.ppt/;
my @bincopydirs = qw/Tcl_sml_ClientInterface/;
my @bincopyfiles = qw/*.dll *.exe *.jar *.lib/;

# Step 1: Rebuild everything
system "rebuild-all.bat";

# Step 2: Check out source from SVN
`svn export $soarurl ../$distdir`;

# Step 3: Remove files that are not to be distributed with the release
my @files = File::Find::Rule->file()->name( @removed )->in('../$distdir');

foreach (@files) {
	unlink $_ or die "Unable to remove $_: $!";
}

# Step 4: Copy stuff over to dist
foreach (@bincopydirs) {
	dircopy("SoarLibrary/bin/$_", "../$distdir/SoarLibrary/bin/Tcl_sml_ClientInterface");
}

my @copylist = File::Find::Rule->file()->name( @bincopyfiles )->in('SoarLibrary/bin');

foreach (@copylist) {
	copy("$_", "../$distdir/$_") or die "Unable to copy $_: $!";
}

