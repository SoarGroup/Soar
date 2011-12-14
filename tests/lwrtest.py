from __future__ import print_function
import sys, os, math, random
from subprocess import *
sys.path.append('.')
from lwr import LWR
import numpy as np

def dist(x1, x2):
	d = x1 - x2
	return math.sqrt(np.dot(d, d))

def toarray(line):
	return np.array([float(v) for v in line.split()])
	
def regress(nnbrs, text):
	p = Popen(['./lwrtest', str(nnbrs)], stdin=PIPE, stdout=PIPE)
	out = p.communicate(text)[0]
	return toarray(out)

if len(sys.argv) != 3:
	print('usage: lwrtest.py <num neighbors> <data file>', file=sys.stderr)
	sys.exit(1)

m = None
nnbrs = int(sys.argv[1])
diffs = []
nulldiffs = []
training = True
for line in open(sys.argv[2]):
	if line.startswith('$'):
		training = False
		continue
		
	x, y = map(toarray, line.split(';'))
	if training:
		if not m:
			m = LWR(len(x), len(y), nnbrs)
		m.update(x, y)
	else:
		p = m.predict(x)
		diffs.append(dist(p, y))
		nulldiffs.append(dist(x[:len(y)], y))
	
diffa = np.array(diffs)
print('Mean: ', np.mean(diffa))
print('STD:  ', np.std(diffa))

ndiffa = np.array(nulldiffs)
print('NULL Mean: ', np.mean(ndiffa))
print('NULL STD:  ', np.std(ndiffa))
