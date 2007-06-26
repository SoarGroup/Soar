use strict;

my $input_kif = $ARGV[0];
if(scalar @ARGV == 0){
	print "No input kif file\n";
}

my $operator_mapping_table = {"+"=>"plus", "-"=>"minus", "*"=>"multiply", "/"=>"division", 
">"=>"gt", "<"=>"lt", ">="=>"gteq", "<="=>"lteq"
#, "distinct"=>"distinct"
};


open (OF2, ">$input_kif.dos");
open(IF, "<$input_kif");
my $content = "";
while(my $line = <IF>){
	$content .= $line;
}


#first pass
while($content =~ /(.*?)(?=(\(<=|$|;{2,}))/gis){
	
	print OF2 $1;
	my $chunk = $1;
	my $kif_rule;
	if($chunk =~ /^\s*\(<=/){
		$kif_rule = $chunk;

		#(<= (number ?var)
	    #(+ ?var 0 ?var))

		if($kif_rule =~ /<=\s*(.*?)\n(.*)\)/gis){
			my ($head, $body) = ($1, $2);
			if($body =~ /^\s*\(([^\(\)]*)\)\s*$/){
				print "Special definition rules\n";
				print $kif_rule,"\n";
			}
		}
	}
}
close IF;
close OF2;

#exit;
open (OF, ">$input_kif.modified");
# ?=, the effect is that (<=. still include for next regex match
#while($content =~ /(\(<=.*?)(?=\(<=|$|;;)/gis){
while($content =~ /(.*?)(?=(\(<=|$|;{2,}))/gis){
	
	my $chunk = $1;
	my $kif_rule;
	if($chunk =~ /^\s*\(<=/){
		$kif_rule = $chunk;
	}
	else{
		print OF $chunk;
		next;
	}
	print "#######rule########\n";
	#print $kif_rule,"\n";
	my $replacing_rules = translate_rule($kif_rule);

	if(scalar(@$replacing_rules) == 0){
		print OF $kif_rule;
	}
	else{
		print OF ";;;;;;Replacing rules\n";
		foreach  my$new_rule(@$replacing_rules) {
			print OF $new_rule,"\n";
		}
		my $old_rule = $kif_rule;
		$old_rule =~ s/\n/\n;;;;;;/gis;
		$old_rule =~ s/^/;;;;;;/gis;
		if(scalar(@$replacing_rules) == 1 and $replacing_rules->[0] eq ""){
			print OF ";;;;;;The following rule is removed\n";
		}
		else{
			print OF ";;;;;;The following rule is replaced by the above rules\n",$old_rule,"\n";
		}
	}
}
close OF;


sub translate_rule{
	my ($rule) = @_;
	my $replacing_rules = [];
	if($rule =~ /^(\(*<=.*?)\n(.*)/gis){
		my($head, $body) = ($1, $2);
		#print $head,"\n";
		#print $body, "\n";
		my $math_sentences = [];
		my $comparison_sentences = [];

		while($body =~ /(\(([\+\-\*\/])\s+([^\s\(\)]*)\s+([^\s\(\)]*)\s+([^\s\(\)]*)\))/gis){
			my($sentence, $operator, $arg1, $arg2, $arg3) = ($1, $2, $3, $4, $5);
			#print $operator, ",", $arg1, ",", $arg2, ",", $arg3,"\n";
			#print $`,"\n";
			print "Found math sentence: ",$sentence,"\n";
			push(@$math_sentences, $sentence);
		}

		while($body =~ /(\((>|<|>=|<=)\s+([^\s\(\)]*)\s+([^\s\(\)]*)\))/gis){
			my($sentence, $operator, $arg1, $arg2) = ($1, $2, $3, $4);
			#print $operator, ",", $arg1, ",", $arg2, ",", $arg3,"\n";
			#print $`,"\n";
			print "Found comparison sentence: ",$sentence,"\n";
			push(@$comparison_sentences, $sentence);
		}

		if(scalar(@$math_sentences) == 0 and scalar(@$comparison_sentences) == 0){
			return [];
		}
		my $condition = $body;
		foreach  my$sentence((@$math_sentences, @$comparison_sentences)) {
			my $tmp = $sentence;
			$tmp =~ s/([\?\)\(\+\-\*\/])/\\$1/gis;
			#print $sentence,"\n";
			$condition =~ s/$tmp//is;
		}
		print "Rest of the condition excluding math sentences:\n", $condition;
		#(<= (number ?var)
		# (+ ?var 0 ?var))
		#The above kif rule is testing whether a constant is number, need to replace all rules containing (number ?var) into (+ ?var 0 ?var)
		
		# should not be replaced, but removed
		#if($condition =~ /^[\s\(\)]*$/gis){
		#	return [""];
		#}

		#print $body,"\n";
		
		my $new_body = $body;

		foreach  my$sentence(@$math_sentences) {
			
			if($sentence =~ /\((\S+)\s+(\S+)\s+(\S+)\s+(\S+)\)/){
				my($operator, $arg1, $arg2, $arg3) = ($1, $2, $3, $4);
				#print $operator, ",", $arg1, ",", $arg2, ",", $arg3,"\n";
				if(!exists $operator_mapping_table->{$operator}){
					print "Cannot find mapping for ", $operator;
				}
				my $math_rule_head = "(<= (numerical-operation ".$operator_mapping_table->{$operator}." $arg1 $arg2)";
				my $new_rule_math = $math_rule_head."\n".$condition;
				print "\nMath sentence $sentence --> math rule head:\n $math_rule_head\n";
				#print $new_rule_math,"\n";
				
				push(@$replacing_rules, $new_rule_math);
				my $tmp = $sentence;
				$tmp =~ s/([\?\)\(\+\-\*\/])/\\$1/gis;
				$new_body =~ s/$tmp/\(numerical-result $operator_mapping_table->{$operator} $arg1 $arg2 $arg3)/;

				print "New body After replacing math sentence $sentence:\n", $new_body,"\n";
			}
			else{
				print $sentence," cannot be parsed\n";
				exit;
			}
		}
	
		
		foreach  my$sentence(@$comparison_sentences) {
			
			if($sentence =~ /\((\S+)\s+(\S+)\s+(\S+)\)/){
				my($operator, $arg1, $arg2) = ($1, $2, $3);
				#print $operator, ",", $arg1, ",", $arg2, ",", $arg3,"\n";
			if(!exists $operator_mapping_table->{$operator}){
					print "Cannot find mapping for ", $operator;
				}
				my $comparison_rule_head = "(<= (numerical-operation ".$operator_mapping_table->{$operator}." $arg1 $arg2)";
				my $new_rule_comparison = $comparison_rule_head."\n".$condition;
				print "\nComparison sentence $sentence --> comparison rule head:\n $comparison_rule_head\n";
				#print $new_rule_math,"\n";
				
				push(@$replacing_rules, $new_rule_comparison);
				my $tmp = $sentence;
				$tmp =~ s/([\?\)\(\+\-])/\\$1/gis;
				$new_body =~ s/$tmp/\(numerical-result $operator_mapping_table->{$operator} $arg1 $arg2 true)/;

				print "New body After replacing math sentence $sentence:\n", $new_body,"\n";
			}
			else{
				print $sentence," cannot be parsed\n";
				exit;
			}
		}
		
		

		my $main_rule = $head."\n".$new_body,"\n";
		print "Replacing main rule:\n", $main_rule,"\n";
		push(@$replacing_rules, $main_rule);
	}
	else{
		print "Not recognized\n";
		exit;
	}

	return $replacing_rules;
}