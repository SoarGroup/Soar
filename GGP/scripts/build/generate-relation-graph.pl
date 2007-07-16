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


open(IF, "<$input_kif");
my $content = "";
while(my $line = <IF>){
	$content .= $line;
}


#first pass
my $rule_count = 0;
my $apply_hash = {};
my $legal_hash = {};
my $goal_predicates = ["collapsed", "bridge-complete"];

my $elab_hash = {};

my $printed_node_hash = {};

my $child_parent_hash = {};
my $parent_child_hash = {};

while($content =~ /(.+?)(?=(\n\s*\(<=|$|;{2,}))/gis){
	
	my $chunk = $1;
	my $kif_rule;
	if($chunk =~ /\n\s*\(<=/){
		$kif_rule = $chunk;
		++ $rule_count;

		#(<= (number ?var)
	    #(+ ?var 0 ?var))

		if($kif_rule =~ /<=\s*(.*?)\n(.*)\)/gis){
			my ($head, $body) = ($1, $2);
			my @head_segs = split(/[\s\(\)]+/, $head);
			my $head_pred = $head_segs[0];
			#print $head_pred,"\n";
			#exit;
			#if($body =~ /^\s*\(([^\(\)]*)\)\s*$/){
				#print "Special definition rules\n";
				#print $kif_rule,"\n";
			#}
			#print $head,"\n";
			if($head =~ /\s*\(next\s*/i){
				#print $kif_rule,"\n";
				my $rule_info = {};
				my $act;
				if($body =~ /\(does\s+\S+\s*\((.+?)\)+(\n|$)/i){#apply rule
					my $action = $1;
					my $type = "apply";
					#print $action,"\n";
					my @action_arg = split(/\s+/, $action);
					$act = $action_arg[0];
					#print $act,"\n";
					if(!exists ($apply_hash->{$act})){
						$apply_hash->{$act} = [];
					}
				}
				else{#next without does is frame axiom, which is not useful for this purpose
					next;
				}
				my $body_info = parse_body($body);
				$body_info->{"rule_id"} = $rule_count;
				#multiple does rules
				push(@{$apply_hash->{$act}}, $body_info);
			}
			elsif($head =~ /\s*\(legal\s+(\S+)\s+\((.+?)\)+/i){#legal rule
				my ($role, $action) = ($1, $2);
				#print $action,"\n";
				my @action_arg = split(/\s+/, $action);
				my $act = $action_arg[0];
				my $body_info = parse_body($body);
				$body_info->{"action"} = $action;
				$body_info->{"rule_id"} = $rule_count;
				#may have multiple legal rules
				if(!exists ($legal_hash->{$act})){
					$legal_hash->{$act} = [];
				}
				push(@{$legal_hash->{$act}}, $body_info);
			}
			else{ #elaboration rules
				my @head_args = split(/[\s\)\(]+/, $head);
				shift@head_args while($head_args[0] eq"");
				my $predicate = $head_args[0];
				print $predicate,"\n";

				my $body_info = parse_body($body);
				if(!exists ($elab_hash->{$predicate})){
					$elab_hash->{$predicate} = [];
				}
				$body_info->{"head"} = $head;
				$body_info->{"rule_id"} = $rule_count;
				push(@{$elab_hash->{$predicate}}, $body_info);
			}

		}
		else{
			print "somethign wrong\n";
			print $kif_rule,"\n";
			exit;
		}
	}
	else{
		#print $chunk,"\n";
	}
}
close IF;

my $OUT;
open($OUT, ">out.dot");
print $OUT "digraph G {\n";


my $goal_count = 0;
my $goal_id = "goal";
print $OUT $goal_id," [shape=triangle];\n";
if(scalar(@$goal_predicates) > 0){
	
	foreach  my$goal_predicate(@$goal_predicates) {
		$goal_count ++;
		my $goal_rule_id = "goal_rule_node".$goal_count;
		print $OUT $goal_rule_id," [shape=box, label=goal];\n";
		print $OUT $goal_rule_id," -> ", $goal_id,";\n";;
		print_predicate_recursive($goal_predicate, $goal_rule_id);
	}
}

print $OUT "}\n";

exit;

my $i = 0;
my $condition_index = 0;
foreach  my $action(keys%$apply_hash) {
	
	if(!exists $legal_hash->{$action}){
		print "No legal rules for $action\n";
		exit;
	}
	my $apply_rules_info = $apply_hash->{$action};
	my $legal_rules_info = $legal_hash->{$action};
	
	
	my $action_id = make_dot_id($action);
	print $OUT $action_id," [shape=triangle];\n";
	foreach  my$apply_rule(@$apply_rules_info) {
		my $predicates = $apply_rule->{"predicates"};
		my $gs_conditions = $apply_rule->{"gs_conditions"};
		if(scalar(@$predicates) == 0 and scalar(@$gs_conditions) == 0){
			next;
		}
		++$i;
		#my $rule_id = make_dot_id("apply-$action-$i");
		#my $rule_id = make_dot_id("apply-$i");
		my $rule_id = "rule_node_".$apply_rule->{"rule_id"};
		print $OUT $action_id, "[label=\"$action\"];\n", $action_id," -> ", $rule_id,";\n";
		print $OUT $rule_id," [shape=Mdiamond, sides=5, label=apply];\n";
		
		foreach  my$predicate(@$predicates) {
			print_predicate_recursive($predicate, $rule_id);
			#my @segs = split(/[\(\)\s]+/, $predicate);
			#shift@segs while($segs[0] eq"");
			#my ($pred, $color) = ($segs[0], "black");
			#if($segs[0] eq "not"){
			#	$pred = $segs[1];
			#	$color = "red";
			#}
			#my $predicate_id = make_dot_id($pred);
			#print $OUT $predicate_id,"[color=$color];\n",$predicate_id," -> ", $rule_id,";\n";
			
			#my $elab_rules = $elab_hash->{$pred};
	
			#foreach  my$elab_rule(@$elab_rules) {
			#	print $pred,"\n";	
			#	my $gs_conditions = $elab_rule->{"gs_conditions"};
			#	my $predicates = $elab_rule->{"predicates"};
			#	my $elab_rule_id = "rule_node_".$elab_rule->{"rule_id"};
				
			#	if(!exists $printed_node_hash->{$elab_rule_id}){
			#		$printed_node_hash->{$elab_rule_id} = 1;
			#		print $OUT $elab_rule_id,"[shape=box, label=elab];\n", $elab_rule_id," -> ", $predicate_id,";\n";

			#	}
			#}
		}
		
		my $condition_count = scalar(@$gs_conditions);
		if($condition_count > 0){
			$condition_index++;
			my $condition_id = "condition".$condition_index;
			print $OUT $condition_id,"[label=$condition_count];\n",$condition_id," -> ", $rule_id,";\n";
		}
	}

	foreach  my$legal_rule(@$legal_rules_info) {
		my $predicates = $legal_rule->{"predicates"};
		my $gs_conditions = $legal_rule->{"gs_conditions"};
		if(scalar(@$predicates) == 0 and scalar(@$gs_conditions) == 0){
			next;
		}
		++$i;
		#my $rule_id = make_dot_id("apply-$action-$i");
		#my $rule_id = make_dot_id("legal-$i");
		my $rule_id = "rule_node_".$legal_rule->{"rule_id"};

		print $OUT $rule_id," [shape=Mdiamond, sides=5, label=legal];\n";
		print $OUT $rule_id," -> ", $action_id,";\n";;
		foreach  my$predicate(@$predicates) {
			print_predicate_recursive($predicate, $rule_id);
			#my @segs = split(/[\(\)\s]+/, $predicate);
			#shift@segs while($segs[0] eq"");
			#my ($pred, $color) = ($segs[0], "black");
			#if($segs[0] eq "not"){
			#	$pred = $segs[1];
			#	$color = "red";
			#}
			#my $predicate_id = make_dot_id($pred);
			#print $OUT $predicate_id,"[color=$color];\n",$predicate_id," -> ", $rule_id,";\n";
		}
		
		my $condition_count = scalar(@$gs_conditions);
		if($condition_count > 0){
			$condition_index++;
			my $condition_id = "condition".$condition_index;
			print $OUT $condition_id,"[label=$condition_count];\n",$condition_id," -> ", $rule_id,";\n";
		}
	}
	
		
}



print $OUT "}\n";
close $OUT;

sub make_dot_id{
	my($input) = @_;
	$input =~ s/\W/\_/gis;
	return $input;
}

sub parse_body{
	my($body) = @_;
	my $body_info;
	my $predicates = [];
	my $gs_conditions = [];
	my $math = [];
	while($body =~ /(.+?)(\n|$)/gis){
		my $line = $1;
		$line =~ s/^\s+|\s+$//gis;
		my @segs = split(/[\(\)\s]+/, $line);
		shift@segs while($segs[0] eq"");
		#print $line,"\n";
		#print "@segs\n";
		#print "$segs[0], $segs[1]\n";
		my $str = $segs[0];
		if($segs[0] eq "not"){
			$str = $segs[1];
		}
		if($str =~ /^\s*does\s*$/){
			$body_info->{"action"} = $line;
			#print $line,"\n";
			#exit;
		}
		elsif($str =~ /^\s*true\s*$/){
			push (@$gs_conditions, $line);
		}
		elsif($str =~ /^([\=\+\-\*\/<>]|<=|>=|distinct)$/){
			push (@$math, $line);
		}
		else{
			push(@$predicates, $line);
		}
	}
	$body_info->{"predicates"} = $predicates;
	$body_info->{"gs_conditions"} = $gs_conditions;
	$body_info->{"math"} = $math;
	return $body_info;
}


sub print_predicate_recursive{
	my($predicate, $rule_id) = @_;

	my @segs = split(/[\(\)\s]+/, $predicate);
	shift@segs while($segs[0] eq"");
	my ($pred, $style) = ($segs[0], "filled");
	if($segs[0] eq "not"){
		$pred = $segs[1];
		$style = "dotted";
	}
	my $predicate_id = make_dot_id($pred);
	print $OUT $predicate_id,"[label=\"$pred\"];\n",$predicate_id," -> ", $rule_id,"[style=$style];\n";

	my $elab_rules = $elab_hash->{$pred};

	foreach  my$elab_rule(@$elab_rules) {
		print $pred,"\n";	
		my $gs_conditions = $elab_rule->{"gs_conditions"};
		my $predicates = $elab_rule->{"predicates"};
		my $elab_rule_id = "rule_node_".$elab_rule->{"rule_id"};
		
		if(!exists $printed_node_hash->{$elab_rule_id}){ #the rule could have been printed from other path
			$printed_node_hash->{$elab_rule_id} = 1;
			print $OUT $elab_rule_id,"[shape=box, label=elab];\n", $elab_rule_id," -> ", $predicate_id,";\n";
			foreach  my$child_pred(@$predicates) {
				print_predicate_recursive($child_pred, $elab_rule_id);
			}
			my $condition_count = scalar(@$gs_conditions);
			if($condition_count > 0){
				$condition_index++;
				my $condition_id = "condition".$condition_index;
				print $OUT $condition_id,"[label=$condition_count];\n",$condition_id," -> ", $elab_rule_id,";\n";
			}
		}
	}

}