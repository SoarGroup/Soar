use strict;

#Input file should be the soar file from translator
# In order to enforce unique math query, each query must be send to top-state via operator
# Therefore, need to modify all the rules involving numerical-operations

my ($input_file) = @ARGV;
#print "source header.soar\n";
open(IF, "<$input_file");
my $content;
while(my $line = <IF>){
	$content .= $line;
}
while($content =~ /(sp \{.*?\}\s*)(?=\nsp \{|$)/gis){
	my $rule = $1;
	#print $rule,"\n####\n";
	
	my $matched_chunks = [];
	while($rule =~ /(\^soar-hack-numerical-result-(\S+)\s+(<\S+?>))/gi){
		 push(@$matched_chunks, [$1, $2, $3]);
	}
	my $changes = [];
	
	foreach  my$matched_chunk(@$matched_chunks) {
		my ($chunk, $op, $var) = @$matched_chunk;
		#(<s1> ^p1 { <x21> > <x12>} ^p3 <end-x>)
		if($rule =~ /\($var\s+\^p1 (.+?) \^/s){
			my $p1 = $1;
			push(@$changes, [$chunk, $op, $var, $p1]);
		}
		else{
			print "cannot fine var $var in rule\n$rule\n";
			exit 1;
		}
	}
	my $i = 0;
	while($rule =~ /(.+?)(\n|$)/gis){
		my $line = $1;
		#print $line,"\nXXX\n";
		if($line =~ /\^soar-hack-numerical-result-/){
			foreach  my$change(@$changes) {
				my ($chunk, $op, $var, $p1) = @$change;
				$chunk = "\\".$chunk;
				if($line =~ s/$chunk//){
					++ $i;
					my $new_var = "<hack-var-$i>";
					print "(<s> ^top-state.numerical-results.$op ".$new_var,")\n";
					print "($new_var ^p1 $p1 ^rest $var)\n";
				}
			}
			if($line =~ /\^/){
				print $line,"\n";
			}
		}
		else{
			print $line,"\n";
		}

	}
	#exit;
	next;
	my $line;
	# Only for RHS of the rule
	 #replace  (<e> ^object-coordinates <o> ^numerical-result <n> ^numerical-result <n1>)
	 # with (<e> ^object-coordinates <o>)(<s> ^top-state.numerical-results.query <n> ^top-state.numerical-results.query <n1>)
	 # assuming <s> shuold always be state <s>

	 #replace (<e> ^numerical-result <n> ^numerical-result <n1>)
	 #with (<s> ^top-state.numerical-results.query <n> ^top-state.numerical-results.query <n1>)

	 #(<e> ^primitive-object <p> ^object-coordinates <o> ^soar-hack-numerical-result-plus <s1> ^clear <c> ^bridge-segment <b>)
	 #(...) (<s> ^top-state.numerical-results.plus <s1>)
	 if($line =~ /\^soar-hack-numerical-result/i){
		 
		 my $new_line = "";
		 my $i = 0;
		 foreach  (@$matched_chunks) {
			 #print $chunk,"\n";
			 my ($chunk, $op, $var) = @$_;
			 ++ $i;
			 #$new_line .= " ^top-state.numerical-results.<query$i> ".$var;
			 $new_line .= " ^top-state.numerical-results.$op ".$var;
			 $chunk = "\\".$chunk; #change the symbol ^ to \^ in regex
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
print "source build_rules.soar\n";
close IF;

#print $content,"\n";

#while($content =~ /\b(sp\s+\{.+?\})\s*(?=sp\s+{)/gis){
#	my $soar_rule = $1;

#	print "###rule###\n",$soar_rule,"\n";
#}
