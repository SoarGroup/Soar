#!/usr/bin/perl
#
# New installer generation script.
#
# Assumptions:
# Run in Cygwin, svn in path

# Overview:
# Builds everything in release mode in the current tree.
# Exports everything in -source (up one dir and using distdir variable) for Source component
# Removes files from -source that are not to be included in the release
# Moves files from -source to -core for Core component
# Copies binaries from working tree to -core for Core component

use strict;
use File::Find::Rule;
use File::Copy::Recursive qw/rcopy rmove/;
use File::Copy;
use File::Path;

##################
# Variables

# SVN url
my $soarurl = "https://winter.eecs.umich.edu/svn/soar/trunk/SoarSuite";

# Destination
my $distdir = "SoarSuite-8.6.2-r1"; # will actually append -source and -core to this for the two components

# File globs to completely remove from the tree (not distributed at all)
my @remove = qw/.cvsignore *.so *.so.2 *.jnilib java_swt make-mac-app.sh *.plist *.doc *.ppt *.pl *.am *.ac *.m4/;

# Globs to copy from working copy to Core component
# WORKING --copy-to-> CORE
my @copyglobs = qw/*.dll *.exe *.jar *.lib Tcl_sml_ClientInterface/;

# Globs to MOVE from Source component to Core component
# SOURCE --move-to-> CORE
my @moveglobs = qw//;
##################

my $core = "../$distdir-core";
my $source = "../$distdir-source";

# Parse command line options
my $build = 1;
my $checkout = 1;

foreach (@ARGV) {
	if ($_ eq "-nobuild") {
		$build = 0;
	}
	if ($_ eq "-nocheckout") {
		$checkout = 0;
	}
}

if ($build == 1) {
	print "Step 1: Rebuild everything...\n";
	system "rebuild-all.bat";
}

my @files;

if ($checkout == 1) {
	print "Step 2: Check out source from SVN...\n";
	print "Removing tree $distdir-source...\n";
	rmtree("../$distdir-source");
	system "svn export $soarurl ../$distdir-source";
	
	print "Step 3: Remove globs from source that are not to be distributed with the release...\n";
	@files = (File::Find::Rule->directory()->name(@remove)->in($source), File::Find::Rule->file()->name(@remove)->in($source));

	foreach (@files) {
		print "Removing from source: $_\n";
		unlink $_ or die "Unable to remove $_: $!";
	}
}


print "Step 4: Copy globs from working tree to core...\n";
print "Removing tree $core...\n";
rmtree($core, 1);
@files = (File::Find::Rule->directory()->name(@copyglobs)->in("."), File::Find::Rule->file()->name(@copyglobs)->in("."));
foreach(@files) {
	# This creates destination if it doesn't exist.
	print "Copying to core: $_\n";
	rcopy($_, "$core/$_");
}

print "Step 5: Move globs from source tree to core...\n";
@files = (File::Find::Rule->directory()->name(@moveglobs)->in($source), File::Find::Rule->file()->name(@moveglobs)->in($source));
foreach(@files) {
	# This creates destination if it doesn't exist.
	print "Moving from source to core: $_\n";
	rcopy($_, "$core/$_");
}

