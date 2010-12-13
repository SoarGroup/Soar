#!/bin/sh
PYTHON=python2

svn export http://soar.googlecode.com/svn/wiki/CommandLineInterface.wiki

# split into single file for each command
awk '
	BEGIN { f="/dev/null" }
	
	/^= *[-_a-zA-Z]* *=$/ {
		f="wiki/" $2 ".wiki"
	}
	
	/----/ { next }   # delete horizontal separators
	
	{ print > f }' CommandLineInterface.wiki || exit 1

# translate to tex
echo generating command sources
for f in `ls wiki`
do
	tf=`echo "$f" | sed 's/wiki$/tex/'`
	echo -n "$tf "
	$PYTHON moin2latex.py "wiki/$f" > "tex/$tf" || exit 1
done
echo finished
