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


$tmpFile = "tmp_gdlchk";

print `echo \"$domain $level $scenario $run $sourceNum\" > $tmpFile`;
print `grep ACTION $file | sed -e 's/ACTION //' >> $tmpFile`;

$ENV{"GGP_PATH"}="../";
if ($ARGV[1]) { # debug flag
  print `cat $tmpFile | python ./gm.py`;
}
else {
 print  `cat $tmpFile | python ./gm.py | grep Created`;
}
print `rm $tmpFile`;

if (defined $mergeName) {
  print `rm $mergeName`;
}
