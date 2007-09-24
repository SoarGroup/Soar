use Cwd;
use strict;


#my $file1 = "build.core.manual.kif";
#my $file1 = "build.core.manual.7-20.kif";
#my $file1 = "build.core.simplified.manual.8-6.kif";
#my $file1 = "build.core.simplified.manual.revision.8-9.kif";
#my $file1 = "8-17-update/mBuild.core.manual.kif";
#my $file1 = "8-17-update/mBuild.core.manual.cheap.legal.kif";
#my $file2 = "8-17-update/mBuild.simple.test-target.kif";

#my $file1 = "8-29-update/mBuild.core.manual.kif";
#my $file1 = "9-5-update/mBuild.core.manual.kif";
#my $file1 = "9-5-update/9-5-core.kif";
my $file1 = "C:/YJ/OtherSoarVersions/GGP_new/kif/build/build.core.kif";
#my $file2 = "C:/YJ/OtherSoarVersions/GGP_new/kif/build/build-7-22-target.kif";
my $file2 = "C:/YJ/OtherSoarVersions/GGP_new/kif/build/build-2-blocks.kif";
#my $file2 = "C:/YJ/OtherSoarVersions/GGP_new/kif/build/build-7-24-source.kif";
#my $file2 = "9-5-update/7-2-source-instance.3stairway.kif";
#my $file2 = "9-5-update/7-2-target-instance.4stairway.kif";
#my $file2 = "9-5-update/8-1-target-instance.kif";
#my $file2 = "9-5-update/8-1-source-instance.kif";
#my $file2 = "9-5-update/7-1-source-instance.kif";
#my $file2 = "9-5-update/7-1-target-instance.kif";
#my $file2 = "9-5-update/7-3-target-instance.kif";
#my $file2 = "8-29-update/mBuild.tower.kif";
#my $file2 = "8-29-update/mBuild.8.1.src.kif";
#my $file2 = "8-29-update/mBuild.8.1.targ.kif";

#my $file2 = "build.bridge.kif";
#my $file2 = "build.simple.kif";
#my $file2 = "build.test.compound.kif";
#my $file2 = "build.test.rotate.unsupport.kif";
#my $file2 = "build.test.heavy.crush.kif";
#my $file2 = "build.test.bridge.shape.kif";
#my $file2 = "scenarios/build.composing.src1.kif";
#my $file2 = "scenarios/build.composing.src2.kif";
#my $file2 = "scenarios/build.composing.src2.revision.kif";
#my $file2 = "build.test.weight-contribution.kif";
#my $file2 = "build.test.stack.high.kif";
#my $file2 = "mini_tower.kif";
#my $file2 = "simple_scenarios/build.composing.src.kif";
#my $file2 = "simple_scenarios/build.composing.src.revision.kif";
#my $file2 = "simple_scenarios/build.composing.target.kif";
#my $file2 = "simple_scenarios/build.composing.target.revision.kif";
my $merged_file = "build.merge.kif";
my $agent_file = "../../agents/build_final.soar";

#my $process_math_perl = "./handle_numerical-operations.pl";
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