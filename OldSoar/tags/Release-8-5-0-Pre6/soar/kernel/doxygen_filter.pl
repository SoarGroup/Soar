#!/usr/bin/perl


$file = $ARGV[0];
if ( $ARGV[0] =~ /(.*)soarBuildOptions\.h$/ ) {
 $file = "$1BuildOptions";
}

#print stderr "FILTERING --> $ARGV[0] --> $file\n";

open( INP, "<$file" );
while( <INP> ) { print "$_"; }
close( INP );
