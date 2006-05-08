#!/usr/bin/perl

# patcher script to modify ORTS files to allow us to use the pathfinder.
# changes:
#
#libs/serverclient/src/TerrainModule.H:


#libs/serverclient/src/TerrainModule.H:
#  void findPath(GameObj* gob, TerrainBase::Loc goal, TerrainBase::Path path);
#  as public member function
#libs/serverclient/src/TerrainModule.C:
#implement:
#void TerrainModule::findPath(GameObj* gob, TerrainBase::Loc goal, TerrainBase::Path path) {
#  timp.findPath(gob, goal, path);
#}

#libs/serverclient/src/TerrainBase.H:
#virtual void findPath(const Object* gob, const Loc &l2, Path &path) = 0;

#libs/newpath/src/SimpleTerrain.H:
#virtual void findPath(const Object* gob, const Loc &l2, Path &path);
#(as public member function of ST_Terrain, inherits from TerrainBase)

#libs/newpath/src/SimpleTerrain.C:
#void ST_Terrain::findPath(const Object* gob, const Loc &l2, Path &path) {
#  pfEngine->find_path(gob, l2, path);
#}
 
# expect to run from the orts3/apps/sorts/src directory
$serverClientDir = "../../../libs/serverclient/src";
$newPathDir = "../../../libs/newpath/src";

$terrModH = "$serverClientDir/TerrainModule.H";
$terrModC = "$serverClientDir/TerrainModule.C";
$terrBaseH = "$serverClientDir/TerrainBase.H";
$simpleTerrH = "$newPathDir/SimpleTerrain.H";
$simpleTerrC = "$newPathDir/SimpleTerrain.C";

die "$terrModH does not exist" unless (-e $terrModH);
die "$terrModC does not exist" unless (-e $terrModC);
die "$terrBaseH does not exist" unless (-e $terrBaseH);
die "$simpleTerrH does not exist" unless (-e $simpleTerrH);
die "$simpleTerrC does not exist" unless (-e $simpleTerrC);

print `mv $terrModH $terrModH.original`;
print `mv $terrModC $terrModC.original`;
print `mv $terrBaseH $terrBaseH.original`;
print `mv $simpleTerrH $simpleTerrH.original`;
print `mv $simpleTerrC $simpleTerrC.original`;

open (NEW, ">$terrModH") or die "can't open.";
open (ORIG, "<$terrModH.original") or die "can't open";

$step = 1;
while ($line = <ORIG>) {
  print NEW $line;
  if ($step == 1 and $line =~ "class TerrainModule") {
    $step++;
  }
  elsif ($step == 2 and $line =~ "public") {
    print NEW "\tvoid findPath(GameObj* gob, TerrainBase::Loc goal, TerrainBase::Path& path);\n";
    $step++;
  }
}

die "bad patch in $terrModH" unless ($step == 3);
print "successfully patched $terrModH, original file is $terrModH.original.\n";

open (NEW, ">$terrModC") or die "can't open.";
open (ORIG, "<$terrModC.original") or die "can't open";

while ($line = <ORIG>) {
  print NEW $line;
}
print NEW "\n";
print NEW "void TerrainModule::findPath(GameObj* gob, TerrainBase::Loc goal, TerrainBase::Path& path) {\n";
print NEW "\ttimp.findPath(gob, goal, path);\n";
print NEW "}\n";
  
#die "bad patch in $terrModC" unless ($step == 3);
print "successfully patched $terrModC, original file is $terrModC.original.\n";

open (NEW, ">$terrBaseH") or die "can't open.";
open (ORIG, "<$terrBaseH.original") or die "can't open";

$step = 1;
while ($line = <ORIG>) {
  print NEW $line;
  if ($step == 1 and $line =~ "class TerrainBase") {
    $step++;
  }
  elsif ($step == 2 and $line =~ "find_path") {
    print NEW "\tvirtual void findPath(const Object* gob, const Loc &l2, Path &path) = 0;\n";
    $step++;
  }
}

die "bad patch in $terrBaseH" unless ($step == 3);
print "successfully patched $terrBaseH, original file is $terrBaseH.original.\n";

open (NEW, ">$simpleTerrH") or die "can't open.";
open (ORIG, "<$simpleTerrH.original") or die "can't open";

$step = 1;
while ($line = <ORIG>) {
  print NEW $line;
  if ($step == 1 and $line =~ "class ST_Terrain") {
    $step++;
  }
  elsif ($step == 2 and $line =~ "find_path") {
    print NEW "\t\tvirtual void findPath(const Object* gob, const Loc &l2, Path &path);\n";
    $step++;
  }
}

die "bad patch in $simpleTerrH" unless ($step == 3);
print "successfully patched $simpleTerrH, original file is $simpleTerrH.original.\n";

open (NEW, ">$simpleTerrC") or die "can't open.";
open (ORIG, "<$simpleTerrC.original") or die "can't open";

$step = 1;
while ($line = <ORIG>) {
  print NEW $line;
  if ($step == 1 and $line =~ "namespace SimpleTerrain") {
    print NEW "\tvoid ST_Terrain::findPath(const Object* gob, const Loc &l2, Path &path) {\n";
    print NEW "\t\tpfEngine->find_path(gob, l2, path);\n";
    print NEW "\t}";
    $step++;
  }
}

die "bad patch in $simpleTerrC" unless ($step == 2);
print "successfully patched $simpleTerrC, original file is $simpleTerrC.original.\n";
