from __future__ import print_function
import sys, os, random

if len(sys.argv) != 4:
	print('usage: divide.py <num train> <num test> <data>', file=sys.stderr)
	sys.exit(1)

ntrain = int(sys.argv[1])
ntest = int(sys.argv[2])
lines = [l.strip() for l in open(sys.argv[3])]
random.shuffle(lines)
print('\n'.join(lines[:ntrain]))
print('$')
print('\n'.join(lines[ntrain:ntrain+ntest]))
