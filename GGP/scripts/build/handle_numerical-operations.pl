############################################
# IMPORTANT
# When parsing kif rules, it assumes the following are the terminator of a rule
# (<=		begining of another rule
# \n;{2,}		comment that start without indent in a new line, if there is indent, it's assumed to be comment
# $
#############################################

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
my $rule_count = 0;
while($content =~ /(.*?)(?=(\n\s*\(<=|$|;{2,}))/gis){
	
	print OF2 $1;
	my $chunk = $1;
	my $kif_rule;
	if($chunk =~ /^\s*\(<=/){
		$kif_rule = $chunk;
		++ $rule_count;

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
print $rule_count,"\n";
#exit;
open (OF, ">$input_kif.modified");
open(OF_unbound_var, ">$input_kif.modified.unbound");
# ?=, the effect is that (<=. still include for next regex match
#while($content =~ /(\(<=.*?)(?=\(<=|$|;;)/gis){
while($content =~ /(.*?)(?=(\n\s*\(<=|$|\n;{2,}))/gis){
	
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
		print OF "\n;;;;;;Replacing rules\n";
		foreach  my$new_rule(@$replacing_rules) {
			print OF $new_rule,"\n";
		}
		my $old_rule = $kif_rule;
		$old_rule =~ s/\n/\n;;;;;;/gis;
		$old_rule =~ s/^/;;;;;;/gis;
		if(scalar(@$replacing_rules) == 1 and $replacing_rules->[0] eq ""){
			print OF "\n;;;;;;The following rule is removed\n";
		}
		else{
			print OF "\n;;;;;;The following rule is replaced by the above rules\n",$old_rule,"\n";
		}
	}
}
close OF;
close OF_unbound_var;

sub check_bound_recursive{
	my ($num_var, $grounded_vars, $num_result_var_to_arg_var) = @_;
	if(scalar(keys%$grounded_vars) == 0){
		return 0;
	}
	if(exists$grounded_vars->{$num_var}){
		return 1;
	}
	if(exists $num_result_var_to_arg_var->{$num_var}){
		my $arg_vars = $num_result_var_to_arg_var->{$num_var};
		foreach  my$arg_var(@$arg_vars) {
			if(!check_bound_recursive($arg_var, $grounded_vars, $num_result_var_to_arg_var)){
				return 0;
			}
		}
		return 1;
	}

	return 0;

}

sub translate_rule{
	my ($rule) = @_;
	my $replacing_rules = [];
	if($rule =~ /^(\s*\(*<=.*?)\n(.*)/gis){
		my($head, $body) = ($1, $2);
		#print $head,"\n";
		#print $body, "\n";
		my $math_sentences = [];
		my $comparison_sentences = [];
		my $other_num_sentences = [];

		my $numerical_variables = {};
		my $num_result_var_to_arg_var = {};
		my $other_variables = {};

		# get rid of comment lines
		$body =~ s/\s*;{2,}.*?\n/\n/gis;

		while($body =~ /(\(number\s+([^\s\(\)]*)\))/gis){
			my($sentence, $arg1) = ($1, $2);
			$numerical_variables->{$arg1} = 1;
			print "Found number sentence: ",$sentence,"\n";
			push(@$other_num_sentences, $sentence);
		}
		
		#if-gt0-then-else
		while($body =~ /(\(if-gt0-then-else\s+([^\s\(\)]*)\s+([^\s\(\)]*)\s+([^\s\(\)]*)\s+([^\s\(\)]*)\))/gis){
			my($sentence, $arg1, $arg2, $arg3, $arg4) = ($1, $2, $3, $4, $5);
			if($arg1 =~ /^\?/){
				$numerical_variables->{$arg1} = 1;
			}
			if($arg2 =~ /^\?/){
				$numerical_variables->{$arg2} = 1;
			}
			if($arg3 =~ /^\?/){
				$numerical_variables->{$arg3} = 1;
			}
			if($arg4 =~ /^\?/){
				$numerical_variables->{$arg4} = 1;
			}
			print "Found if-gt0-then-else sentence: ",$sentence,"\n";
			push(@$other_num_sentences, $sentence);
		}

		while($body =~ /(\(([\+\-\*\/])\s+([^\s\(\)]*)\s+([^\s\(\)]*)\s+([^\s\(\)]*)\))/gis){
			my($sentence, $operator, $arg1, $arg2, $arg3) = ($1, $2, $3, $4, $5);
			if($arg1 =~ /^\?/){
				$numerical_variables->{$arg1} = 1;
			}
			if($arg2 =~ /^\?/){
				$numerical_variables->{$arg2} = 1;
			}
			#$arg3 is result, so doesn't need to be bounded elsewhere
			# actually $arg3 is bound as long as $arg1, $arg2 is bound, so $arg3 should be part of bound vars
			# Need to recursively check whether the corresponding $arg1, $arg2 is bounded.
			if($arg3 =~ /^\?/){
				#$numerical_result_variables->{$arg3} = 1;
				$numerical_variables->{$arg1} = 1;
				$num_result_var_to_arg_var->{$arg3} = [];
				if($arg1 =~ /^\?/){
					push(@{$num_result_var_to_arg_var->{$arg3}}, $arg1);
				}
				if($arg2 =~ /^\?/){
					push(@{$num_result_var_to_arg_var->{$arg3}}, $arg2);
				}
			}
			
			
			
			
			#print $operator, ",", $arg1, ",", $arg2, ",", $arg3,"\n";
			#print $`,"\n";
			print "Found math sentence: ",$sentence,"\n";
			push(@$math_sentences, $sentence);
		}

		while($body =~ /(\((>|<|>=|<=)\s+([^\s\(\)]*)\s+([^\s\(\)]*)\))/gis){
			my($sentence, $operator, $arg1, $arg2) = ($1, $2, $3, $4);
			
			if($arg1 =~ /^\?/){
				$numerical_variables->{$arg1} = 1;
			}
			if($arg2 =~ /^\?/){
				$numerical_variables->{$arg2} = 1;
			}

			#print $operator, ",", $arg1, ",", $arg2, ",", $arg3,"\n";
			#print $`,"\n";
			print "Found comparison sentence: ",$sentence,"\n";
			push(@$comparison_sentences, $sentence);


		}

		if(scalar(@$math_sentences) == 0 and scalar(@$comparison_sentences) == 0){
			return [];
		}
		my $condition = $body;
		
		

		foreach  my$sentence((@$math_sentences, @$comparison_sentences, @$other_num_sentences)) {
			my $tmp = $sentence;
			$tmp =~ s/([\?\)\(\+\-\*\/])/\\$1/gis;
			#print $sentence,"\n";
			$condition =~ s/$tmp//is;
		}
		print "Variables need binding in numerical predicates: ",keys%$numerical_variables,"\n";
		print "Rest of the condition excluding math sentences:\n", $condition,"\n";
		while($condition =~ /(\?[^\s\)\(\)]+)/gis){
			$other_variables->{$1} = 1;
		}
		print "Variables in rest of the conditions: ",keys%$other_variables,"\n";
		my $has_unbound_variable = 0;
		my $unbound_vars = [];
		foreach  my$num_var(keys%$numerical_variables) {
			#if(!exists $other_variables->{$num_var}){
			if(!check_bound_recursive($num_var, $other_variables, $num_result_var_to_arg_var)){
				print "$num_var unbound\n";
				$has_unbound_variable = 1;
				push(@$unbound_vars, $num_var);
			}
		}
		if($has_unbound_variable){
			print OF_unbound_var "\nThis rule has unbound vars @$unbound_vars\n";
			print OF_unbound_var $rule,"\n";
		}
		#(<= (number ?var)
		# (+ ?var 0 ?var))
		#The above kif rule is testing whether a constant is number, need to replace all rules containing (number ?var) into (+ ?var 0 ?var)
		
		# should not be replaced, but removed
		#if($condition =~ /^[\s\(\)]*$/gis){
		#	return [""];
		#}

		#print $body,"\n";
		
		my $new_body = $body;
		
		my $div_tmp_result_to_rounded_var = {};
		#while($new_body =~ /\(round\s+(\S+)\s+(\S+)\)/gis){
		while($new_body =~ s/\(round\s+(\S+)\s+(\S+)\)//is){
			my($div_tmp_result, $rounded_var) = ($1, $2);
			$div_tmp_result_to_rounded_var->{$div_tmp_result} = $rounded_var;
			print "Found round operator: $rounded_var is round of $div_tmp_result\n";
		}

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
				print $new_rule_math,"\n";
				

				#Don't need these math rules, if all possible combinations are precompute
				#push(@$replacing_rules, $new_rule_math);
				my $tmp = $sentence;
				$tmp =~ s/([\?\)\(\+\-\*\/])/\\$1/gis;
				if($operator eq "/" and exists $div_tmp_result_to_rounded_var->{$arg3}){
					my $rounded_var = $div_tmp_result_to_rounded_var->{$arg3};
					#$new_body =~ s/$tmp/\(numerical-result $operator_mapping_table->{$operator} $arg1 $arg2 $arg3 $rounded_var)/;
					$new_body =~ s/$tmp/\(soar-hack-numerical-result-$operator_mapping_table->{$operator} $arg1 $arg2 $arg3 $rounded_var)/;
				}
				else{
					$new_body =~ s/$tmp/\(soar-hack-numerical-result-$operator_mapping_table->{$operator} $arg1 $arg2 $arg3)/;
				}

				print "New body After replacing math sentence $sentence:\n", $new_body,"\n";
			}
			else{
				print $sentence," cannot be parsed\n";
				exit;
			}
		}
	
		
		#foreach  my$sentence(@$comparison_sentences) {
		foreach  my$sentence(()) {#don't replace comparison
			
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
				#Don't need these math rules, if all possible combinations are precompute
				#push(@$replacing_rules, $new_rule_comparison);
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