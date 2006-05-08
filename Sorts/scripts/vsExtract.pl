#!/usr/bin/perl

# vsExtract.pl
# extracts a single .soar file from a visualsoar project

die "arg: agent file\n" unless ($#ARGV == 0 and ($ARGV[0] =~ /.*soar$/));

die "$ARGV[0] does not exist" unless (-e $ARGV[0]);

$file = $ARGV[0];
if ($file =~ /(.*)\/(\S+?)$/) {
  our $path = $1;
  $file = $2;
}
else {
  our $path = "";
}

processFile($file);

sub processFile {
  $file = shift;
  print "\n## vsExtract.pl: extracting from file $file\n";
  if ($path ne "") {
    $file = "$path/$file";
  }
  die "$file not found" unless (-e $file);

  my $fh;
  open ($fh, "<$file"); 
  
  foreach $line (<$fh>) {
    if ($line =~ /^pushd (\S+)$/) {
      $path .= "/$1";
    }
    elsif ($line =~ /^source (\S*)$/) {
      processFile($1);
    }
    elsif ($line =~ /^popd$/) {
      $path =~ /.*(\/\S+?)$/ or die "bad popd";
      $path =~ s/$1$//;
    }
    else {
      print $line;
    }
  }
}
