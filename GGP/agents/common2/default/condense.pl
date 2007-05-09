#!/usr/bin/perl

# condense.pl
# extracts a single .soar file from a visualsoar project

our $progName = "condense.pl";

unless ($#ARGV >= 0 and ($ARGV[0] =~ /.*soar$/)) {
  print "$progName: extract a single Soar agent from a VisualSoar project.\n";
  print "productions are written to stdout\n";
  print "files can be ommited in the extracted agent either by command-line\n";
  print "flags to $progName, or by giving them a prefix of \"x_\"\n\n";
  print "args: agent_file.soar -disable_file.soar -disable_file.soar..\n";
  exit(1);
}

die "$ARGV[0] does not exist" unless (-e $ARGV[0]);

$file = shift @ARGV;

our @disabledFiles = ();
foreach $file (@ARGV) {
  $file =~ s/^-// or die "bad flag: $file";
  push @disabledFiles, $file;
}

if ($file =~ /(.*)\/(\S+?)$/) {
  our $path = $1;
  $file = $2;
}
else {
  our $path = ".";
}

processFile($file, 0);

sub processFile {
  my $file = shift;
  my $disabled = shift;
  print "\n## $progName: extracting from file $file\n";
  
  if ($disabled) {
    print "## $progName: higher-level file has been disabled by command-line flag\n"; 
    $prefix = "# ";
  }
  elsif (grep /^$file$/, @disabledFiles) {
    $prefix = "# ";
    print "## $progName: file has been disabled by command-line flag\n";
    $disabled = 1;
  }
  elsif ($file =~ /^x_/) {
    print "## $progName: file has been disabled with x_ prefix\n";
    $prefix = "# ";
    $disabled = 1;
  }
  else {
    $prefix = "";
  }
  
  if ($path ne "") {
    $file = "$path/$file";
  }
  die "$file not found" unless (-e $file);

  my $fh;
  open ($fh, "<$file"); 
  
  foreach $line (<$fh>) {
    if ($line =~ /^pushd (\S+)\s*$/) {
      $path .= "/$1";
    }
    elsif ($line =~ /^source (\S*)\s*$/) {
      processFile($1, $disabled);
    }
    elsif ($line =~ /^popd\s*$/) {
      $path =~ /.*(\/\S+?)$/ or die "bad popd";
      $path =~ s/$1$//;
    }
    else {
      print $prefix.$line;
    }
  }
}
