use Cwd;
use strict;


#my $file1 = "build.core.manual.kif";
my $file1 = "build.core.manual.7-20.kif";
#my $file2 = "build.bridge.kif";
#my $file2 = "build.simple.kif";
#my $file2 = "build.test.compound.kif";
#my $file2 = "build.test.rotate.unsupport.kif";
#my $file2 = "build.test.heavy.crush.kif";
#my $file2 = "build.test.bridge.shape.kif";
#my $file2 = "scenarios/build.composing.src1.kif";
#my $file2 = "scenarios/build.composing.src2.kif";
my $file2 = "build.test.stack.high.kif";
my $merged_file = "build.merge.kif";
my $agent_file = "../../agents/build_final.soar";

my $process_math_perl = "./handle_numerical-operations.pl";
#my $translator_dir = "../../pysrc";
#my $translator_dir = "../../old_translator";
my $translator_dir = "../../old_translator";
my $translator = "python LoadKif.py";
my $handle_header = "./handle_build_header.pl";

my $dir = cwd;
print $dir, "\n";
#exit;
if(not -e $file1){
	die "cannot find $file1";
}
if(not -e $file2){
	die "cannot find $file2";
}
system ("cat $file1 $file2 > $merged_file");
#system ("perl $process_math_perl ".$merged_file);
chdir ($translator_dir);
#system($translator." ".$dir."/".$merged_file.".modified ".$dir."/tmp.soar");

system($translator." ".$dir."/".$merged_file." ".$dir."/tmp.soar");

chdir ($dir);
system("perl ".$handle_header." tmp.soar > tmp2.soar");

system ("cat tmp2.soar  > $agent_file");