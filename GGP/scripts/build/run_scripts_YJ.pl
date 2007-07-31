use strict;

$ENV{"PATH"} .= ";C:/YJ/OtherSoarVersions/trunk/SoarLibrary/bin;C:/YJ/OtherSoarVersions/trunk/SoarLibrary/lib;C:/Python24/libs";

#my $runSoar = "runSoar_cpp.exe";

my $runSoar = "C:\\YJ\\TestProjects\\SML_Console\\debug\\SML_Console.exe";

my $source = $ARGV[0];

my $output_file = "out";
open(OF, ">$output_file");
my $t = localtime;
print OF $t,"\n";
close OF;
system ("$runSoar soar -l $source >> $output_file");
open(OF, ">>$output_file");
$t = localtime;
print OF $t,"\n";
close OF;