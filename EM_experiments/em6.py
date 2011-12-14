#!/usr/bin/env python

# Try to learn on multi-dimensional data from splinter simulation

from __future__ import print_function
import math as m
import numpy as np
import random
import itertools as itl
import matplotlib.pyplot as plt
import common as cm
import sklearn.linear_model as lm
import sklearn.pls as pls
import pdb

K = 5
NITERS = 30
STD = 0.0001

COLOR_CYCLE = 'rbgcmyk'
#random.seed(0)

np.set_printoptions(precision=3)

class WRModel:
	def __init__(self, a = None, b = None, center = None):
		if a != None and b != None and center != none:
			self.a = a
			self.b = b
			self.center = center
		
	def fit(self, x, y, w):
		x1 = np.hstack((x, np.ones((x.shape[0], 1))))
		W = np.diag(w) 
		Z = np.dot(W, x1)
		v = np.dot(W, y)
		C = np.linalg.lstsq(Z, v)[0]
		self.a = C[:-1]
		self.b = C[-1]
		self.xmin = np.min(x)
		self.xmax = np.max(x)
		self.center = np.sum(x, axis=0) / x.shape[0]
	
	def predict(self, x):
		return np.dot(self.a, x) + self.b
	
	def pdf(self, x, y):
		return cm.gaussian_pdf(y, self.predict(x), STD)
	
	def plot(self, color):
		X = [self.xmin, self.xmax]
		Y = [self.predict(self.xmin), self.predict(self.xmax)]
		plt.plot(X, Y, color + '--')
		plt.scatter(self.center, [self.predict(self.center)], c = color, marker = 'd', s = 60)
		
	def __repr__(self):
		return 'coefs: {}\ninter: {}'.format(self.a, self.b)

class PLSModel:
	def __init__(self):
		pass
	
	def fit(self, x, y, w):
		#m = lm.Lasso(alpha=0.5)
		#m = lm.LinearRegression()
		self.m = pls.PLSCanonical()
		self.m.fit(x, y.reshape((len(y), 1)))
	
	def predict(self, x):
		return self.m.predict(x)
	
	def pdf(self, x, y):
		return cm.gaussian_pdf(y, self.predict(x), STD)
		
def plot_coverage(models, xdata, ydata, map_cluster):
	# noise points
	members = [i for i in range(len(map_cluster)) if map_cluster[i] == -1]
	if len(members) > 0:
		plt.scatter(xdata[members, 0], ydata[members], c = 'gray', edgecolors='none')
	
	xmin = np.min(xdata)
	xmax = np.max(xdata)
	print('num models = {}'.format(len(models)))
	for i, m in enumerate(models):
		color = COLOR_CYCLE[i % len(COLOR_CYCLE)]
		#m.plot(color)
		members = [j for j in range(len(map_cluster)) if map_cluster[j] == i]
		if len(members) > 0:
			plt.scatter(xdata[members, 0], ydata[members], c = color, edgecolors='none')
	
	plt.show()

def model_error(models, X, Y, Pij):
	best = [np.argmax(Pij[:, j]) for j in range(len(X))]
	return sum((Y[i] - models[best[i]].predict(X[i])) ** 2 for i in range(X.shape[0]))

def load_data():
	xd = []
	yd = []
	for line in open('emdata'):
		if len(line.strip()) == 0 or line.strip().startswith('#'):
			continue
		
		xs, _, ys = line.partition(';')
		xd.append(np.array(map(float, xs.split())))
		yd.append(np.array(map(float, ys.split())))
	
	return np.vstack(xd), np.vstack(yd)[:,0]

xdata, ydata = load_data()
ndata = xdata.shape[0]
ymin = np.min(ydata)
ymax = np.max(ydata)
epsilon = 0.01
Pnoise = epsilon / (ymax - ymin)

#models = [(random.random(), random.random(), random.random(), 1.0 / nmodels) for i in range(nmodels)]
models = []

#models.append(WRModel(np.array([0., 0., 0., 0., 0., 0., 0., 1.0, 0., 0., 0., 0.]), 0., np.array([0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.])))

E = []
cluster_increase_times = []

for iters in range(NITERS):
	#print('Iteration', iters)
	nmodels = len(models)
	
	if nmodels == 0:
		eligibility = np.zeros((0, ndata))
	else:
		centerdist = np.vstack([np.sum(np.power(xdata - models[i].center, 2), axis = 1) for i in range(nmodels)])
		eligibility = np.zeros((nmodels, ndata))
		for j in range(ndata):
			eligibility[np.argmin(centerdist[:, j]), j] = 1.0
	
	#print(eligibility)
	#pdb.set_trace()
	# E step
	Pij = np.zeros((nmodels, ndata))
	map_cluster = [-1] * ndata
	#import pdb; pdb.set_trace()
	for i, m in enumerate(models):
		for j, (x, y) in enumerate(itl.izip(xdata, ydata)):
			if eligibility[i, j] >= 0: # change to > 0 to use locality
				weight = 1.0 / np.sum(eligibility[:, j])
				#weight = 1.0 / nmodels
				Pij[i, j] = (1.0 - epsilon) * weight * m.pdf(x, y)
				if (map_cluster[j] == -1 and (epsilon == 0. or Pij[i, j] > Pnoise)) or \
				   (map_cluster[j] != -1 and Pij[i, j] > Pij[map_cluster[j], j]):
					map_cluster[j] = i

	for m in models:
		print(m)
	
	plot_coverage(models, xdata, ydata, map_cluster)

	Pj = np.sum(Pij, axis = 0) + Pnoise
	normPij = Pij / Pj
	
	Pi = np.sum(normPij, axis = 1)
	PiTotal = np.sum(Pi)

	w = np.zeros((nmodels, ndata))
	for i in range(nmodels):
		for j in range(ndata):
			w[i, j] = normPij[i, j] / Pi[i]
	
	if len(models) > 0:
		E.append(model_error(models, xdata, ydata, Pij))
	#Ls.append(cm.linear_log_likelihood(data, models, epsilon, 1.0 / (MAX - MIN)))
	
	#pdb.set_trace()
	# M step
	#epsilon = min(1.0, (ndata * epsilon * Pnoise) / (sum(normalizer)))
	#epsilon = float(map_cluster.count(-1)) / ndata
	#print('epsilon =', epsilon)
	for i, m in enumerate(models):
		pts = np.nonzero(w[i, :])
		m.fit(xdata[pts], ydata[pts], w[i][pts])

	# remove useless models
	for i in range(nmodels - 1, -1, -1):
		if map_cluster.count(i) < 2:
			models.pop(i)
	
	if iters % 4 != 0:
		continue
	
	# possibly create a new cluster
	noise_data = [ j for j in range(ndata) if map_cluster[j] == -1 ]
	#print('Noise samples {}'.format(len(noise_data)))
	if len(noise_data) > K:
		seed = random.choice(noise_data)
		dists = [ np.sum(np.power(xdata[j] - xdata[seed], 2)) for j in range(ndata) ]
		close = np.argsort(dists)[:K]
		xclose = xdata[close]
		yclose = ydata[close]
		m = WRModel()
		m.fit(xclose, yclose, np.ones(len(close)))
		models.append(m)
		cluster_increase_times.append(iters)
	
plt.yscale('log')
plt.plot(E)
for t in cluster_increase_times:
	plt.axvline(x = t)
plt.show()
