use strict;

my $range = [-5, 20];
#my $other_numbers = [41, 50, 1000, 100];
my $other_numbers = [10, 20, 25, 40, 80, 50, 1000];
#my $other_numbers = [10, 20, 25];

my $operators = ["plus", "minus", "multiply", "division", 
#"gt", "gteq", "lt", "lteq"
];

print "sp {init-numerical-results\n";
print "(state <s> ^superstate nil)\n";
print "-->\n";
print "(<s> ^numerical-results <r>)\n";
foreach  my$operator(@$operators) {
	foreach  my$i(($range->[0] .. $range->[1], @$other_numbers)) {
		print "(<r> ^$operator <r-$i-$operator>) (<r-$i-$operator> ^p1 $i)\n";
		foreach  my$j($range->[0] .. $range->[1], @$other_numbers) {
			#print "(<r> ^r-$i-$operator-$j <r-$i-$operator-$j>)";
			my $result;
			if($operator eq "plus"){
				$result = $i + $j;
			}
			elsif($operator eq "minus"){
				$result = $i - $j;
			}
			elsif($operator eq "multiply"){
				$result = $i * $j;
			}
			elsif($operator eq "division"){
				if($j == 0){
					$result = "NA";
				}
				else{
					$result = $i / $j;
					$result = $result." ^p4 ".int($result);
				}
			}
			elsif($operator eq "gt"){
				if($i > $j){
					$result = "true";
				}
				else{
					$result = "false";
				}
			}
			elsif($operator eq "gteq"){
				if($i >= $j){
					$result = "true";
				}
				else{
					$result = "false";
				}
			}
			elsif($operator eq "lt"){
				if($i < $j){
					$result = "true";
				}
				else{
					$result = "false";
				}
			}
			elsif($operator eq "lteq"){
				if($i <= $j){
					$result = "true";
				}
				else{
					$result = "false";
				}
			}
			print "(<r-$i-$operator> ^rest <r-$i-$operator-$j>)(<r-$i-$operator-$j> ^p1 ".$i." ^p2 ".$j." ^p3 ".$result.")\n";
			#print "(<r-$i-$operator-$j> ^p1 ".$operator." ^p2 ".$i." ^p3 ".$j." ^p4 ".$result.")\n";
		}
	}
}
print "}\n";