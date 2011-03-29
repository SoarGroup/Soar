#!/bin/sh
PYTHON=python

if [ ! -d wiki ]
then
	svn co http://soar.googlecode.com/svn/wiki
elif ! svn update wiki | grep -q Updated
then
	exit 0
fi

rm -rf tex
mkdir tex

for f in wiki/cmd_*.wiki
do
	tf=`echo "$f" | sed 's:^wiki/cmd_::
	                     s:_:-:g
	                     s:wiki$:tex:'`
	echo -n "$tf "
	$PYTHON moin2latex.py "$f" > "tex/$tf" || exit 1
done
echo

# make sure every command listed on the wiki is included in the
# interface section of the manual

for f in tex/*
do
	f=`echo $f | awk -F. '{print $1}'`
	if ! grep -q "input{wikicmd/$f}" ../interface.tex
	then
		unused=1
		echo UNUSED: $f
	fi
done

if [ -n "$unused" ]
then
	exit 1
fi
