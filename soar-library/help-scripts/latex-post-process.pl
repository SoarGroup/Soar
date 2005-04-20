#!/usr/bin/perl

use strict;

my $state = "header";
my $removed_dash = "no";

my $command;

while (<>) {

  if (/\\documentclass/) {
    next;
  }

  if (/\\usepackage/) {
    next;
  }
  
  if (/\\title/) {
    next;
  }
  
  if (/\\begin\{tabular\}\{\|c\|c\|\}/) {
    print "\\begin{tabular}{|l|l|}\n";
    next;
  } else {
    if (/\\begin\{tabular\}\{\|c\|c\|c\|\}/) {
      print "\\begin{tabular}{|l|l|l|}\n";
      next;
    }
  }

  if (/^ \\\\/) {
    next;
  }
  
  if (/\\end\{document\}/) {
    next;
  }
  
  if ($state eq "header") {
    if (/\\begin.*/) {
      next;
    }

    if (/\\section\*\{(.+)\}/) {
      $command = lc $+;
      print "\\subsection{\\soarb{$command}}\n";

      print "\\label{$command}\n";
      print "\\index{$command}\n";
      
      $state = "body";
      next;
    }
  }

  if ($state eq "body") {

    if (/\\section\*\{ Name \}/) {
      next;
    }
    
    if (/\\section\*\{ ([A-Za-z\-0-9_ ]+) \}/) {
      $_ = $+;
      if (/[Ss]tructured [Oo]utput/) {
        $_ = "Structured Output:";
      } 
      if (/[Ee]rror [Vv]alues/) {
        $_ = "Error Values:";
      }
      if (/[Ss]ee [Aa]lso/) {
	$state = "seealso";
      }
      print "\\subsubsection\*\{$_\}\n";
      next;
    }

    if (/\\subsection\*\{ ([A-Za-z\-0-9_ ]+) \}/) {
      print "\\paragraph\*\{$+\}\n";
      next;
    }
    
    if (/\\section\*\{ Name \}/) {
      next;
    }
    
    if (/\\textbf\{$command\}/) {
      if ($removed_dash eq "no") {
        $state = "remove-dash";
        next;
      }
    }
  }

  if ($state eq "remove-dash") {
    $state = "body";
    $removed_dash = "yes";
    if (/ \- (.+)/) {
      print $+ . "\n";
    }
    next;
  }
  
  # remove whitespace
  if (/^\s$/) {
    next;
  }

  if ($state eq "seealso") {
    if (/[A-Za-z]+/) {
      my @seealsos = split /[\, ]/, $_;
      foreach (@seealsos) {
        my $element = $_;
	chomp $element;
	if ($element) {
          print "\\hyperref[$element]{$element}" . " ";
	}
      }
    }
    $state = "body";
    next;
  }

  print $_;
}
