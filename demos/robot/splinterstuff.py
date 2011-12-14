#/usr/bin/env python

# A lot of random functions to do with testing and data generation for splinter

from __future__ import print_function
import sys, os, random, math
sys.path.append('../../tests')
from itertools import *
import subprocess as sub
import numpy as np
import lwr
import multiprocessing as multi
from itertools import izip, repeat, imap
import cPickle
from scipy.optimize import *

RCOEF = 1.0;
ECOEF = 2.0;
CCOEF = 0.5;
SCOEF = 0.5;

#              pos               rot          lrps rrps   rot rate       vel    lvolt rvolt
#          1     2     3     4     5     6      7   8    9 10  11    12  13 14   15    16
#         17    18    19    20    21    22     23  24   25 26  27    28  29 30
MINS = [  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,   -3, -3,   0, 0, -1,   -1, -1, 0,  -1,   -1   ]
MAXS = [ 10.0, 10.0,  0.0,  0.0,  0.0,  3.14,   3,  3,   0, 0,  1,    1,  1, 0,   1,    1   ]

XDIM = 16
YDIM = 14
NTRAIN = 1000
NTEST = 1000
NNBRS = 50
NITER = 20
NPROCS = 8

Tests = []

def sim(xs):
	input = '\n'.join(' '.join(map(str, x)) for x in xs)
	p = sub.Popen(['splintersim2'], stdin=sub.PIPE, stdout=sub.PIPE)
	out, _ = p.communicate(input)
	if p.returncode != 0:
		print('splintersim2 failed', file=sys.stderr)
		sys.exit(1)
	return [np.array(map(float, line.split())) for line in filter(None, out.split('\n'))]

def randx():
	return np.array([ random.uniform(l, h) for l, h in zip(MINS, MAXS) ])

def nelder_mead_simplex(objective, simplex):
	eval = np.apply_along_axis(objective, 0, simplex)
	for i in range(NITER):
		bi = np.argmin(eval)
		wi = np.argmax(eval)
		t = eval.copy()
		t[wi] = t[bi]
		ni = np.argmax(t)

		print('iteration', i, 'best =', eval[bi], 'worst =', eval[wi])
		
		best = simplex[:, bi]
		worst = simplex[:, wi]
		next = simplex[:, ni]
		centroid = np.sum(simplex, axis=1) / simplex.shape[1]
		dir = centroid - worst
		reflect = centroid + RCOEF * dir;
		
		reval = objective(reflect)
		if eval[bi] <= reval < eval[ni]:
			# reflection
			simplex[:, wi] = reflect
			eval[wi] = reval
			continue
		
		if reval < eval[bi]:
			# expansion
			expand = centroid + ECOEF * dir;
			eeval = objective(expand)
			if eeval < reval:
				simplex[:, wi] = expand
				eval[wi] = eeval
			else:
				simplex[:, wi] = reflect
				eval[wi] = reval
			continue
		
		assert(reval >= eval[ni])
		
		contract = worst + CCOEF * dir
		ceval = objective(contract)
		if ceval < eval[wi]:
			simplex[:, wi] = contract
			eval[wi] = ceval
			continue
		
		# shrink
		for i in range(simplex.shape[1]):
			simplex[:, i] = best + SCOEF * (simplex[:, i] - best)
			eval[i] = objective(simplex[:, i])
	
	bi = np.argmin(eval)
	return bi, eval[bi]

def stringify(x, y):
	xs = ' '.join(map(str, x))
	ys = ' '.join(map(str, y))
	return '{} ; {}'.format(xs, ys)
	
def sample_to_file(n, file):
	X = [ randx() for i in range(n) ]
	Y = sim(X)
	f = open(file, 'w')
	for x, y in izip(X, Y):
		print(stringify(x, y), file=f)
	f.close()
	
def test_accuracy(dummy):
	inds = np.array([5,6,7,10,14,15])
	m = lwr.LWR(XDIM, YDIM, NNBRS)
	#m = lwr.LWR(len(inds), YDIM, NNBRS)
	
	trainx = [randx() for i in range(NTRAIN)]
	trainy = sim(trainx)
	assert(len(trainx) == len(trainy))

	testx = [randx() for i in range(NTEST)]
	testy = sim(testx)

	m.trainbatch(trainx, trainy)
	
	results = []
	#f = open('test_samples', 'a')
	for x, y in izip(testx, testy):
		p = m.predict(x)
		#print(stringify(x, p), file=f)
		#delta1 = p - y
		#delta2 = y - x[:14]
		#ratio = np.array([ abs(d1 / d2) if d2 != 0.0 else 0.0 for d1, d2 in izip(delta1, delta2)])
		#results.append(ratio)
		results.append(np.absolute(y - p))
		
	return np.sum(np.vstack(results), axis=0) / NTEST

def test_transform(a):
	global Tests
	x, y = Tests[a[0]]
	A = np.reshape(a[1], (XDIM, XDIM), order='F')
	m.set_transform(A)
	return np.sum(np.power(y - m.predict(x), 2))
	
def find_transform():
	global Tests

	m = LWR(XDIM, YDIM, NNBRS)
	
	use_pools = False
	
	if use_pools:
		pool = multi.Pool(NPROCS)

		def objective(x):
			seq = izip(range(len(Tests)), repeat(x))
			return sum(pool.imap_unordered(runtest, seq, 5))
	else:
		def objective(x):
			seq = izip(range(len(Tests)), repeat(x))
			return sum(imap(runtest, seq))
	
	baseline = objective(np.eye(XDIM).flatten('F'))
	print('Baseline = ', baseline, file=sys.stderr)

	print('Searching ...', file=sys.stderr)
	simplex = np.random.rand(XDIM * XDIM, XDIM * XDIM + 1) * 10 - 5
	
	bfgs_res = fmin_bfgs(objective, np.eye(XDIM).flatten('F'), maxiter=NITER, callback=lambda x: print('.', file=sys.stderr))
	
	A = np.reshape(bfgs_res, (XDIM, XDIM), order='F')
	B = np.eye(XDIM + 12)
	B[:6,:6]   = A[:6,:6]
	B[:6,18:]  = A[:6,6:]
	B[18:,:6]  = A[6:,:6]
	B[18:,18:] = A[6:,6:]
	print(' '.join(str(b) for b in np.transpose(B).flatten('F')))
	
	#bi, eval = nelder_mead_simplex(objective, simplex), (XDIM, XDIM)
	#A = np.reshape(simplex[bi], (XDIM, XDIM))
	
	if use_pools:
		pool.close()
		pool.join()

if __name__ == '__main__':
	avgerr = 0
	
	if False:
		pool = multi.Pool(NPROCS)
		res = list(pool.imap_unordered(test_accuracy, range(NITER)))
		pool.close()
		pool.join()
	else:
		res = []
		for i in range(NITER):
			res.append(test_accuracy(1))

	avg = np.sum(np.vstack(res), axis=0) / NITER
	print(avg)
	
	