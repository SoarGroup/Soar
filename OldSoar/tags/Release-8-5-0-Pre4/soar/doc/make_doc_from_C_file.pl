#!/local/bin/perl

while (<>) {
  &gen_doc_page if (/\f/);
}

sub gen_doc_page {
  while (<>) {	
    if (/\s+\*\s+(\S+)Cmd\s+--/) {
      open(DOCFILE, ">doc/$1") || die "Can't open file doc/$1!";
      while (<>) {
	if (/\*\//) 
	  {
	    close (DOCFILE);
	    return;
	  }
	elsif (/\s+\*-/) 
	  {
	    # skip *--- lines
	  }
	elsif (/\s+\*(.*)/) 
	  {
	    print DOCFILE "$1\n";
	  }
      }
    }
  }
}
