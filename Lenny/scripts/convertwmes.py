# Changes text from the Soar print output format to the epmem episode
# definition format.

import os, sys
import re
from itertools import imap

def formattype(s):
	if re.match(r'[A-Z]\d+', s) or re.match(r'-?\d*\.\d*', s) or re.match(r'-?\d+', s):
		return s   # symbols and numbers all print as is
	else:
		return '"%s"' % s

# Splits up a string respecting pipe quoting
def tokenize(s):
	ss = s.split()
	ss2 = []
	i = 0
	while i < len(ss):
		if ss[i].startswith('|'):
			t = ''
			while (not ss[i].endswith('|')) or ss[i].endswith(r'\|'):
				t += ss[i]
				i += 1;
			t += ss[i]
			ss2.append(t[1:-1])
		else:
			ss2.append(ss[i])
		i += 1
	return ss2

if __name__ == '__main__':
	addlist = []
	dellist = []
	eps = []
	
	if len(sys.argv) < 2:
		fin = sys.stdin
	else:
		fin = open(sys.argv[1])
		
	for line in imap(lambda x: x.strip(), fin):
		if len(line) == 0:
			continue
		if line.startswith('=>WM:') or line.startswith('<=WM:'):
			toks = tokenize(line[line.find('(')+1:line.rfind(')')])
			timetag = toks[0][:-1] # strip :
			id = toks[1]
			if toks[2] == '^operator' and len(toks) == 5:
				# line has form (205: S1 ^operator O1 +)
				attr = '"operator*"'
				val = formattype(toks[3])
			else:
				# line has form (212: O2 ^name move-disk)
				attr = formattype(toks[2][1:])    # strip caret
				val = formattype(toks[3])
			
			if line.startswith('=>WM:'):
				addlist.append((id, attr, val, timetag))
			else:
				dellist.append((id, attr, val, timetag))
		elif re.match(r'\d+:', line):
			eps.append((addlist, dellist))
			addlist = []
			dellist = []
	
	for addlist, dellist in eps:
		print '+'
		for id, attr, val, timetag in addlist:
			print id, attr, val, timetag
		print '-'
		for id, attr, val, timetag in dellist:
			print timetag
		print '#'