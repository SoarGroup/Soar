#!/usr/bin/perl

use strict;
use Errno;

if (!open LINKS, "command-links") {
	die "Couldn't find command-links file.";
}

my @links = <LINKS>;

if (!open NAMES, "command-names") {
	die "Couldn't find command-names file.";
}
my @names = <NAMES>;

if ($#links != $#names) {
	die "command-links and command-names are different sizes: $#links/$#names";
}

#make the dir, 
if (!mkdir "help") {
	if ($! ne "File exists") {
		die "Couldn't create directory: " . $!;	
	}
}

for (my $i = 0; $i < $#links; $i++) {
	print "Processing @names[$i]";
	`elinks -dump -no-numbering -no-references "@links[$i]" > help/@names[$i]`;
}


#post process
