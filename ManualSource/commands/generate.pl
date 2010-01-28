#!/usr/bin/perl

# Pulls pages for all Soar interface commands off the wiki and turns them into
# tex files to be included in the manual. You need to get the HTML::Latex perl
# package from cpan to run the html2latex script.
#
# Jonathan Voigt
# 2006
# Modified Joseph Xu Aug 2008

use strict;

my $cliSource = "../../../Core/CLI/src/cli_Commands.cpp";
my $commandNames = "command-names";
my $html2latex = "./html2latex --nopar";
my $htmlProcess = "./html-process.pl";
my $latexProcess = "./latex-post-process.pl";

# Generate command-names from source
open COMMANDS, "<$cliSource" or die "Could not open $cliSource: $!";
my @commandsSource = <COMMANDS>;
close COMMANDS;

open NAMES, ">$commandNames" or die "Could not open $commandNames: $!";

print "Commands found: ";
my @commands;

foreach (@commandsSource) {
	if (/char\s+const\s*\*\s*Commands::kCLI\w+\s*=\s*"([\w-]+)"/) {
		push(@commands, $1);
		print $1 . " ";
		print NAMES $1 . "\n";
	}
}
print "\n";

close NAMES;

open POSTPROCESSFILE, "$htmlProcess" or die "Could not find required file $htmlProcess: $!";
close POSTPROCESSFILE;

foreach my $c (@commands) {
	chomp $c;
	print "Processing $c\n";

  `wget -q -O $c.wiki.html http://winter.eecs.umich.edu/soarwiki/Documentation/CLI/$c`;

  `$htmlProcess $c.wiki.html | ./strip_verbatim.py > $c.html`;
  unlink "$c.wiki.html" or die $!;

  `$html2latex $c.html`;
  rename ("$c.tex", "$c.tex.orig") or die $!;
	`cat $c.tex.orig | $latexProcess | uniq > $c.tex`;
}

# This is a huge hack. Basically, it's not easy to automatically make the wiki
# page for the watch command to translate to nice looking tex, so I just did it
# manually and kept the file around
`cp watch.tex.backup watch.tex`
