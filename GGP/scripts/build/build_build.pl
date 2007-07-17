use Cwd;
use strict;


my $file1 = "build.core.manual.kif";
my $file2 = "build.bridge.kif";
#my $file2 = "build.simple.kif";
#my $file2 = "build.test.compound.kif";
my $merged_file = "build.merge.kif";
my $agent_file = "../../agents/build_final.soar";

my $process_math_perl = "./handle_numerical-operations.pl";
#my $translator_dir = "../../pysrc";
my $translator_dir = "../../old_translator";
my $translator = "python LoadKif.py";
my $make_math_elab_top_state = "./handle_build_soar_new.pl";

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
system ("perl $process_math_perl ".$merged_file);
chdir ($translator_dir);
system($translator." ".$dir."/".$merged_file.".modified ".$dir."/tmp.soar");
chdir ($dir);
system("perl ".$make_math_elab_top_state." tmp.soar > tmp2.soar");

#system ("cat tmp2.soar frame_axiom_fix.soar > $agent_file");
system ("cat tmp2.soar  > $agent_file");