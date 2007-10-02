#!/usr/bin/perl

$file = $ARGV[0];

$file =~ /(\w+)-(\d+)-(\d+)-(\w+)/;
$sourceNum = "";
$domain = $1;
$level = $2;
$scenario = $3;
$run = $4;
$domain =~ /^(.)/;
$domain = $1;
$run =~ /^(.)/;
$run = $1;

if ($domain eq "m") { # mrogue -> r
  $domain = "r";
}

if ($file =~ /-source-(\d+)/) {
  $sourceNum = $1;
}

if ($domain eq "b") {
  # merge build
  if ($run eq "s") {
    $kifName = "../kif/build/build-$level-$scenario-source.kif";
    $mergeName = "../kif/build/build-$level-$scenario-source.server-merge.kif";
  }
  else {
    $kifName = "../kif/build/build-$level-$scenario-target.kif";
    $mergeName = "../kif/build/build-$level-$scenario-target.server-merge.kif";
  }
  print `cat ../kif/build/mBuild.core.kif $kifName > $mergeName`;
}

$tmpFile = "tmp_gdlchk";

print `echo \"$domain $level $scenario $run $sourceNum\" > $tmpFile`;
print `grep ACTION $file | sed -e 's/ACTION //' >> $tmpFile`;

$ENV{"GGP_PATH"}="../";
if ($ARGV[1]) { # debug flag
  print `cat $tmpFile | python ./gmdebug.py`;
}
else {
  @results =  `cat $tmpFile | python ./gmdebug.py | grep -v Making`;

  if ($#results != 1) {
    print "solution does not validate\n";
  }
  else {
    if ($results[1]  =~ /(terminal \d+)/) {
      print "$1\n";
    }
    else {
      print "! $results[$#results]";
    }
  }
}
print `rm $tmpFile`;

if (defined $mergeName) {
  print `rm $mergeName`;
}
