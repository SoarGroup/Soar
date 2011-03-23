#!/bin/sh
PYTHON=python2

svn export -q http://soar.googlecode.com/svn/wiki/CommandLineInterface.wiki CommandLineInterface.wiki.new

if [ -f CommandLineInterface.wiki ] && cmp -s CommandLineInterface.wiki.new CommandLineInterface.wiki
then
	# no changes
	rm CommandLineInterface.wiki.new
else
	mv CommandLineInterface.wiki.new CommandLineInterface.wiki
	
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
	echo
fi

# make sure every command listed on the wiki is included in the
# interface section of the manual

for cmd in `ls wiki/ | awk -F. '{print $1}'`
do
	if ! grep -q "input{wikicmd/tex/$cmd}" ../interface.tex
	then
		unused=1
		echo UNUSED: $cmd
	fi
done

if [ -n "$unused" ]
then
	exit 1
fi
