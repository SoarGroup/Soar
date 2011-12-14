#/usr/bin/env python2

from __future__ import print_function
import os, sys
import numpy as np
import heapq
import cPickle
from scikits.learn import ball_tree
from itertools import izip

KP = -3.0

Ref = {}

Ref[6] = np.zeros(16)
Ref[6][6]  = 0.933090909
Ref[6][14] = 0.209454545

Ref[7] = np.zeros(16)
Ref[7][7]  = 0.933090909
Ref[7][15] = 0.209454545

Ref[10] = np.zeros(16)
Ref[10][6] =  -0.010663896
Ref[10][7] =  0.010663896
Ref[10][14] = -0.002393766
Ref[10][15] = 0.002393766

Ref[5] = np.zeros(16)
Ref[5][5] =  1.0
Ref[5][6] =  -0.010663896
Ref[5][7] =  0.010663896
Ref[5][14] = -0.002393766
Ref[5][15] = 0.002393766

class LWR:
	def __init__(self, xdim, ydim, nnbrs):
		self.xs = []
		self.ys = []
		self.xmin = None
		self.xmax = None
		self.xrange = np.ones(xdim)
		self.ball_tree = None
		self.xarray = np.zeros((0,0))
		self.yarray = np.zeros((0,0))
		self.xdim = xdim
		self.ydim = ydim
		self.nnbrs = nnbrs
		self.transform = np.eye(xdim)
		self.dirty = True
	
	def train(self, x, y):
		if len(x) != self.xdim or len(y) != self.ydim:
			print('LWR.train: dim mismatch', file=sys.stderr)
			sys.exit(1)
		
		if self.xmin == None and self.xmax == None:
			self.xmin = x.copy()
			self.xmax = x.copy()
		else:
			self.xmin = np.array([min(a, b) for a, b in izip(self.xmin, x)])
			self.xmax = np.array([max(a, b) for a, b in izip(self.xmax, x)])
		
		self.xs.append(x)
		self.ys.append(y)
		self.dirty = True
		
	def trainbatch(self, X, Y):
		if len(X[0]) != self.xdim or len(Y[0]) != self.ydim:
			print('LWR.trainbatch: dim mismatch', file=sys.stderr)
			sys.exit(1)

		xmin = np.min(X, axis=0)
		xmax = np.max(X, axis=0)
		if self.xmin == None and self.xmax == None:
			self.xmin = xmin
			self.xmax = xmax
		else:
			self.xmin = np.array([min(a, b) for a, b in izip(self.xmin, xmin)])
			self.xmax = np.array([max(a, b) for a, b in izip(self.xmax, xmax)])
		
		self.xs.extend(X)
		self.ys.extend(Y)
		self.dirty = True
		
	def choose_nearest_balltree(self, x):
		xnorm = np.divide(x - self.xmin, self.xrange)
		k = min(self.nnbrs, len(self.xs))
		d, inds = self.ball_tree.query(np.dot(xnorm, self.transform), k=k)
		return d[0], inds[0]
	
	def choose_nearest(self, x):
		xnorm = np.divide(x - self.xmin, self.xrange)
		k = min(self.nnbrs, self.Axarray.shape[0])
		Ax = np.dot(xnorm, self.transform)
		distsq = np.power(self.Axarray - Ax, 2).sum(axis=1)
		inds = heapq.nsmallest(k, range(len(distsq)), key=lambda i: distsq[i])
		return np.sqrt(distsq[inds]), inds
	
	def update(self):
		if not self.dirty:
			return
		
		self.xrange = self.xmax - self.xmin
		for i in range(len(self.xrange)):
			if self.xrange[i] == 0.0:
				self.xrange[i] = 1.0
				
		self.xarray = np.vstack(self.xs)
		#self.Axarray = np.dot(self.xarray, self.transform)
		self.Axarray = np.dot(np.divide(self.xarray - self.xmin, self.xrange), self.transform)
		assert(np.all(np.less_equal(self.Axarray, 1.0)))
		self.yarray = np.vstack(self.ys)
		self.ball_tree = ball_tree.BallTree(self.Axarray)
		self.dirty = False
		
	def predict(self, x):
		if len(self.xs) == 0:
			return None
		
		self.update()
		dists, inds = self.choose_nearest_balltree(x)
		w = np.sqrt(np.power(dists, KP))
		
		# these points are going to cause trouble for regression, hack around it
		isclose = np.logical_or(np.equal(dists, 0), np.isinf(w))
		if np.any(isclose):
			closeinds = [ i for i, v in enumerate(isclose) if v ]
			return np.sum(self.yarray[closeinds], axis=0) / float(len(closeinds))
		
		X = np.hstack((self.xarray[inds], np.ones((len(inds), 1))))
		
		yt = self.yarray[inds]
		changed = [ i for i in range(yt.shape[1]) if np.any(yt[:,i]-yt[0,i]) ]
		y = yt[:,changed]
		W = np.diag(w) 
		Z = np.dot(W, X)
		v = np.dot(W, y)
		C = np.linalg.lstsq(Z, v)[0].transpose()
		x1 = np.append(x, 1)
		p = np.dot(C, x1)
		
		p1 = yt[0].copy()
		for i1, i2 in enumerate(changed):
			if i2 in Ref:
				assert(np.allclose(Ref[i2], C[i1,:16], atol = 1e-5, rtol = 0.01))
			p1[i2] = p[i1]
		return p1

	def set_transform(self, t):
		self.transform = np.transpose(t)
		self.dirty = True