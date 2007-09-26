#!/usr/bin/perl

die unless ($#ARGV == 0);

$file = $ARGV[0];

$file =~ /(\w+)-(\d+)-(\d+)-(\w+)/;
$sourceNum = "";
if ($file =~ /-source-(\d+)/) {
  $sourceNum = $1;
}
$domain = $1;
$level = $2;
$scenario = $3;
$run = $4;
$domain =~ /^(.)/;
$domain = $1;
$run =~ /^(.)/;
$run = $1;

$tmpFile = "tmp_gdlchk";

print `echo \"$domain $level $scenario $run $sourceNum\" > $tmpFile`;
print `grep ACTION $file | sed -e 's/ACTION //' >> $tmpFile`;

$ENV{"GGP_PATH"}="../";
@results =  `cat $tmpFile | python ./gmdebug.py | grep -v Making`;

if ($#results != 2) {
  print "solution does not validate\n";
}
else {
  chomp $results[1];
  chomp $results[2];
  $line = $results[1] . $results[2];
  if ($line =~ /State is terminal(Score: \d+)/) {
    print "$1\n";
  }
  else {
    print "solution is non-terminal!\n";
  }
}

print `rm $tmpFile`;
