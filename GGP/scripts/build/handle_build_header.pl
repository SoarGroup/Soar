use strict;

my $input_file = $ARGV[0];
print "pushd build_common\n";
print "source build_header.soar\n";

open(IF, "<$input_file");
while(my $line = <IF>){
	if($line =~ /^\s*source header.soar/){
	}
	else{
		print $line;
	}
}
close IF;