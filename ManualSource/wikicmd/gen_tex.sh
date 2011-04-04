#!/bin/sh
PYTHON=python

if ! $PYTHON -V 2>&1 | awk '{split($2, v, "."); if (v[1] != 2 || v[2] < 4) exit 1}'
then
	echo "moin2latex.py needs 2.4 <= Python version < 3.0"
	exit 1
fi

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
	printf "$tf "
	sed '/#summary/d' "$f" | $PYTHON moin2latex.py > "tex/$tf" || exit 1
done
printf "\n"

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
