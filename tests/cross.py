from __future__ import print_function
import sys, os
import random
from subprocess import *
sys.path.append('.')
from lwr import LWR
import numpy as np

def distsq(x1, x2):
	return np.power(x1 - x2, 2).sum(axis=0)

def toarray(line):
	return np.array([float(v) for v in line.split()])
	
def regress(nnbrs, text):
	p = Popen(['./lwrtest', str(nnbrs)], stdin=PIPE, stdout=PIPE)
	out = p.communicate(text)[0]
	return toarray(out)

nnbrs = int(sys.argv[1])
m = None
for line in open(sys.argv[2]):
	x, y = map(toarray, line.split(';'))
	if m == None:
		

data = lines[1:]

rdata = random.sample(data, int(len(data) * 0.5))

p1 = regress(nnbrs, lines[0] + '\n' + '\n'.join(rdata))

fdata = [tuple(float(v) for v in l.split()) for l in rdata]
adata = [(np.array(d[:xdim]), np.array(d[xdim:])) for d in fdata]
px, py = adata[0]
m = LWR(xdim, ydim, nnbrs)
for x, y in adata[1:]:
	m.update(x, y)
p2 = m.predict(px)

print(distsq(p1, py))
#print('ERR2: ', distsq(p2, py))
