#!/usr/bin/perl

use strict;

my $state = "header";

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
      $state = "remove-dash";
      next;
    }
  }

  if ($state eq "remove-dash") {
    $state = "body";
    if (/ \- (.+)/) {
      print $+ . "\n";
    }
    next;
  }
  
  if (/^\s$/) {
    next;
  }
  print $_;
}
