use strict;


my $input_file = $ARGV[0];

my $level_info = [];
my $last_action = "";
my $current_depth;
open(IF, "<$input_file");
my $action_counts = 0;
while(my $line = <IF>){
	
	if($line =~ /[0-9]{2}\:[0-9]{2}\:[0-9]{2}/){
		print $line;
	}
	if($line =~ /Depth-from-top ([0-9]+)/){
		$current_depth = $1;
		while(scalar@$level_info > $current_depth){
			pop(@$level_info);
		}
		while(scalar@$level_info <= $current_depth){
			push(@$level_info, {});
		}
		
	}
	if($line =~ /Remaining depth limit ([\-0-9]+)/i){
		$level_info->[$current_depth]->{"remaining depth limit"} = $1;
	}

	if($line =~ /Branching factor ([0-9]+)/i){
		$level_info->[$current_depth]->{"branching factor"} = $1;
	}

	if($line =~ /Remaining branches ([0-9]+)/i){
		$level_info->[$current_depth]->{"remaning branches"} = $1;
	}
	
	if($line =~ /^\s*Operator (.+)/i){
		$level_info->[$current_depth]->{"current action"} = $1;
	}
	#S10393: nudge
	if($line =~ /(S[0-9]+)\: (.+)/i){
		my ($state_name, $op_name) = ($1, $2);
		if($state_name ne "S1" and $op_name =~ /nudge|reinforce|add-to-compound|place-adjacent|brace|rotate|stack/){
			#$level_info->[$current_depth]->{"current action"} = $op_name;
			++$action_counts;
		}
	}
}

close IF;


print $action_counts, " total actions\n";
foreach  my$level(1..(scalar(@$level_info)-1)) {

	my $branching_factor = $level_info->[$level]->{"branching factor"};
	my $remaining_branches = $level_info->[$level]->{"remaning branches"};
	my $current_action = $level_info->[$level]->{"current action"};
	my $depth_limit = $level_info->[$level]->{"remaining depth limit"};

	print "Depth ",$level,"\t", $remaining_branches,"\tof\t",$branching_factor,"\t",$depth_limit," more steps\t",$current_action,"\n";
}
