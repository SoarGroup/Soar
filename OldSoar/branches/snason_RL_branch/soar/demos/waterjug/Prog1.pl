while (defined($line = <STDIN>)) {
	chomp($line);
	if (substr($line, 0, 15) eq '|RL-ConfuseAll|'){ 
		@fields = split /\s+/, $line;
		print "$fields[4]\n";
	}
	#if ($line eq '**** log closed ****'){
#		print "\n";
#	}
}