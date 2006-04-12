#!/usr/bin/perl
# Author: Jonathan Voigt voigtjr@gmail.com
# Date: April 2006
#
# Converts map files from old java tank soar format to new xml format
#
use strict;

open(MAPFILE, "$ARGV[0]");
my @mapfile = <MAPFILE>;
close(MAPFILE);
	
print "<tanksoar-world><cells world-size=\"16\">\n";
  
foreach (@mapfile) {
	if ($_ =~ /[0-9]/) {
		next;
	}
	print "<row>\n";
	my $output = $_;
	$output =~ s/\#/<cell type="wall"><\/cell>/g;
	$output =~ s/\./<cell type="empty"><\/cell>/g;
	print $output;
	print "<\/row>\n";
}

print "</cells></tanksoar-world>\n";
