use strict;

#Input file should be the soar file from translator
# In order to enforce unique math query, each query must be send to top-state via operator
# Therefore, need to modify all the rules involving numerical-operations

my ($input_file) = @ARGV;

open(IF, "<$input_file");
my $content;
while(my $line = <IF>){
	chomp($line);
	# Only for RHS of the rule
	 #replace  (<e> ^object-coordinates <o> ^numerical-result <n> ^numerical-result <n1>)
	 # with (<e> ^object-coordinates <o>)(<s> ^top-state.numerical-results.query <n> ^top-state.numerical-results.query <n1>)
	 # assuming <s> shuold always be state <s>

	 #replace (<e> ^numerical-result <n> ^numerical-result <n1>)
	 #with (<s> ^top-state.numerical-results.query <n> ^top-state.numerical-results.query <n1>)

	 #(<e> ^primitive-object <p> ^object-coordinates <o> ^soar-hack-numerical-result-plus <s1> ^clear <c> ^bridge-segment <b>)
	 #(...) (<s> ^top-state.numerical-results.plus <s1>)
	 if($line =~ /\^soar-hack-numerical-result/i){
		 my $matched_chunks = [];
		 while($line =~ /(\^soar-hack-numerical-result-(\S+)\s+(<\S+?>))/gi){
			 push(@$matched_chunks, [$1, $2, $3]);
		 }
		 my $new_line = "";
		 my $i = 0;
		 foreach  (@$matched_chunks) {
			 #print $chunk,"\n";
			 my ($chunk, $op, $var) = @$_;
			 ++ $i;
			 #$new_line .= " ^top-state.numerical-results.<query$i> ".$var;
			 $new_line .= " ^top-state.numerical-results.$op ".$var;
			 $chunk = "\\".$chunk;
			 $line =~ s/$chunk//;
			# print $line,"\n";
		 }
		 $new_line = "(<s> ".$new_line.")";
		 if($line =~ /\^/){
			 print $line,"\n";
		 }
		 print $new_line,"\n";
	 }
	 else{
		 print $line,"\n";
	 }
}
close IF;

#print $content,"\n";

#while($content =~ /\b(sp\s+\{.+?\})\s*(?=sp\s+{)/gis){
#	my $soar_rule = $1;

#	print "###rule###\n",$soar_rule,"\n";
#}
