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

# Name and version
my $nameandversion = "Soar Suite 8.6.2-r3";

# File globs to completely remove from the tree (not distributed at all)
my @remove = qw/.cvsignore *.so *.so.2 *.jnilib java_swt make-mac-app.sh *.plist *.doc *.ppt *.pl *.am *.ac *.m4 ManualSource *.tex/;

# Globs to copy from working copy to Core component
# WORKING --copy-to-> CORE
my @copyglobs = qw(*.pdf *.dll *.exe *.jar ClientSML.lib ElementXML.lib ConnectionSML.lib Tcl_sml_ClientInterface mac towers-of-hanoi-SML.soar);

# Globs to MOVE from Source component to Core component
# SOURCE --move-to-> CORE
my @moveglobs = qw/COPYING INSTALL Documentation Resources SoarLibrary/;

# Nullsoft installer script input file
my $nsiinput = "8.6.2.nsi.in";

# Nullsoft installer script output file
my $nsioutput = "Soar-Suite-8.6.2.nsi";

# Location of NSIS executable (makensis.exe)
my $makensis = "/cygdrive/c/Program\\ Files\\ \\(x86\\)/NSIS/makensis.exe";
##################

my $namedashes = $nameandversion;
$namedashes =~ tr/ /-/;
my $core = "../$namedashes-core";
my $source = "../$namedashes-source";
my $msprogramsname = $nameandversion;
$msprogramsname =~ s/Suite //;

# Parse command line options
my $build = 1;
my $checkout = 1;
my $nsionly = 0;
my @dirstodelete;
my @filestodelete;

foreach (@ARGV) {
	if ($_ eq "-nobuild") {
		$build = 0;
	} elsif ($_ eq "-nocheckout") {
		$checkout = 0;
	} elsif ($_ eq "-nsionly") {
		$nsionly = 1;
	}
}

if ($nsionly) {
	&nsi_step;
	exit(0);
}

if ($build == 1) {
	&build_step;
}

if ($checkout == 1) {
	&checkout_step;
}

&copy_step;
&move_step;
&nsi_step;
exit(0);

################################
# subroutines

sub build_step {
	print "Step 1: Rebuild everything...\n";
	system "rebuild-all.bat";
}

sub checkout_step { 
	print "Step 2: Check out source from SVN...\n";
	rmtree($source);
	system "svn export $soarurl $source";
	
	print "Step 3: Remove globs from source that are not to be distributed with the release...\n";

	foreach (File::Find::Rule->file()->name(@remove)->in($source)) {
		print "Removing from source: $_\n";
		unlink $_ or die "Unable to remove $_: $!";
	}

	foreach (File::Find::Rule->directory()->name(@remove)->in($source)) {
		print "Removing from source: $_\n";
		rmtree($_) or die "Unable to remove $_: $!";
	}
}

sub copy_step {
	print "Step 4: Remove old core tree...\n";
	rmtree($core, 1);
	print "Step 5: Copy globs from working tree to core...\n";
	foreach (File::Find::Rule->directory()->name(@copyglobs)->in("."), File::Find::Rule->file()->name(@copyglobs)->mindepth(2)->in(".")) {
		# This creates destination if it doesn't exist.
		print "Copying to core: $_\n";
		rcopy($_, "$core/$_");
	}
}

sub move_step {
	print "Step 6: Move globs from source tree to core...\n";
	foreach(File::Find::Rule->directory()->name(@moveglobs)->in($source), File::Find::Rule->file()->name(@moveglobs)->in($source)) {
		# This creates destination if it doesn't exist.
		print "Moving from source to core: $_\n";
		/$source(.*)/;
		my $outputdir = $1;
		rcopy($_, "$core/$outputdir");
	}
	
	print "Step 6.1: Rename INSTALL and COPYING...\n";
	rmove("$core/INSTALL", "$core/Install.txt");
	rmove("$core/COPYING", "$core/License.txt");
}

sub nsi_step {
	print "Step 7: Generate NSI installer script...\n";
	open(NSIINPUT, $nsiinput) or die "Couldn't open nsi input file: $!";
	open(NSIOUTPUT, ">$nsioutput") or die "Couldn't open nsi output file: $!";

	while(<NSIINPUT>) {
		if (/(.*)nameandversion(.*)/) {
			print NSIOUTPUT $1 . $nameandversion . $2 . "\n";
		} elsif (/(.*)installdir(.*)/) {
			print NSIOUTPUT $1 . "Soar/" . $namedashes . $2 . "\n";
		} elsif (/(.*)outfile(.*)/) {
			print NSIOUTPUT $1 . $namedashes . ".exe" . $2 . "\n";
		} elsif (/(.*)msprogramsname(.*)/) {
			print NSIOUTPUT $1 . $msprogramsname . $2 . "\n";
		} elsif (/(.*)corefiles(.*)/) {
			&do_files ($core);
		} elsif (/(.*)sourcefiles(.*)/) {
			&do_files ($source);
		} elsif (/\s*CreateShortCut\s*\"(.+?)\"/) {
			push(@filestodelete, $1);
			print NSIOUTPUT $_;		
		} elsif (/\s*CreateDirectory\s*\"(.+?)\"/) {
			push(@dirstodelete, $1);
			print NSIOUTPUT $_;		
		} elsif (/(.*)deletefiles(.*)/) {
			# Delete "Filename"
			foreach (@filestodelete) {
				print NSIOUTPUT "\tDelete \"$_\"\n";
			}
		} elsif (/(.*)deletedirs(.*)/) {
			# RMDir "Dirname"
			for (my $i = @dirstodelete - 1; $i >= 0; --$i) {
				print NSIOUTPUT "\tRMDir \"@dirstodelete[$i]\"\n";
			}
		} else {
			print NSIOUTPUT $_;		
		}
	}
	close (NSIINPUT);
	close (NSIOUTPUT);

	print "Step 8: Convert forward slashes to backslashes...\n";
	open (NSIINPUT, $nsioutput);
	my @nsiinput = <NSIINPUT>;
	close (NSIINPUT);
	open (NSIOUTPUT, ">$nsioutput");
	foreach (@nsiinput) {
		my $output = $_;
		if ($_ !~ /http:\/\//) {
			$output =~ tr/\//\\/;
		}
		print NSIOUTPUT $output;
	}
	close (NSIOUTPUT);

	print "Step 9: Create installer...\n";
	system "$makensis $nsioutput";
}

sub do_files {
	foreach (File::Find::Rule->directory->in($_[0])) {
		my $currentdir = $_;
		/$_[0](.*)/;
		my $outputdir = $1;
		print NSIOUTPUT "\n\tSetOutPath \"\$INSTDIR$outputdir\"\n";
		push (@dirstodelete, "\$INSTDIR$outputdir");
				
		foreach (File::Find::Rule->file->maxdepth(1)->in("$currentdir")) {
			print NSIOUTPUT "\tFile \"$_\"" . "\n";
			/.*\/(.*)$/;
			push (@filestodelete, "\$INSTDIR$outputdir" . '/' . $1);
		}
	}
}
