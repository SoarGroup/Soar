#!/usr/bin/env python

# Data is generated from linear gaussians. Algorithm initially starts
# with no clusters and adds additional clusters as needed. There needs
# to be a delay between each cluster addition to allow a newly added
# cluster to adjust its parameters to cover the largest number of data
# points. Currently I'm just enforcing an arbitrary number of iterations
# between introducing new clusters.

from __future__ import print_function
import math as m
import numpy as np
import random
import itertools as itl
import matplotlib.pyplot as plt

MIN = -20
MAX = 20
K = 5
EPSILON = 0.01
PNOISE = EPSILON / (MAX - MIN)
NITERS = 50
NDATA = 50

random.seed(0)

def nonuniform_int(probs):
	assert(len(probs) > 0)
	intervals = [probs[0]]
	for p in probs[1:]:
		intervals.append(intervals[-1] + p)
	
	assert(intervals[-1] == 1.0)
	
	r = random.random()
	for i, j in enumerate(intervals):
		if r < j:
			return i

def gaussian_pdf(x, mean, std):
	return (1.0 / (std * m.sqrt(2 * m.pi))) * m.exp(-((x - mean) ** 2) / (2 * std * std))

def linear_gaussian_pdf(x, y, a, b, std):
	return gaussian_pdf(y, a * x + b, std)

def log_gaussian_pdf(x, mean, std):
	return m.log(1.0 / (std * m.sqrt(2 * m.pi))) - ((x - mean) ** 2) / ( 2 * std * std)
	
def log_linear_gaussian_pdf(x, y, a, b, std):
	return log_gaussian_pdf(y, a * x + b, std)

def mixture_pdf(x, y, params):
	return sum(w * linear_gaussian_pdf(x, y, a, b, s) for a, b, s, w in params)

def log_likelihood(XY, params):
	s = 0.0
	for x, y in XY:
		p = max(10e-10, mixture_pdf(x, y, params))
		s += m.log(PNOISE + (1-EPSILON) * p)
	return s
	
def plot_gaussian(mean, std, style):
	X = np.arange(-30, 30, 0.1)
	Y = [gaussian_pdf(x, mean, std) for x in X]
	plt.plot(X, Y, style)

def plot_linear_gaussian(a, b, std, style):
	#X = np.arange(-30, 30, 0.1)
	#Y = np.arange(-30, 30, 0.1)
	#Z = np.array([[linear_gaussian_pdf(x, y, a, b, std) for y in Y] for x in X])
	#plt.contour(X, Y, Z)
	plt.plot([MIN, MAX], [a * x + b for x in [MIN, MAX]], style)
	
def gen_data(n, clusters):
	data = []
	cluster_probs = [c[2] for c in clusters]
	for i in range(n):
		c = nonuniform_int(cluster_probs)
		mean, std, _ = clusters[c]
		while True:
			x = random.gauss(mean, std)
			if MIN <= x <= MAX:
				data.append(x)
				break
	
	return data

def gen_linear_data(n, clusters, cluster_memberships):
	data = []
	cluster_probs = [c[-1] for c in clusters]
	for i in range(n):
		c = nonuniform_int(cluster_probs)
		cluster_memberships.append(c)
		a, b, std, _ = clusters[c]
		x = random.uniform(MIN, MAX)
		mean = a * x + b
		while True:
			y = random.gauss(mean, std)
			if MIN <= y <= MAX:
				data.append((x, y))
				break
	return data

def weighted_linear_regression(x, y, w):
	x1 = np.transpose(np.vstack((x, np.ones(x.shape[0]))))
	W = np.diag(w) 
	Z = np.dot(W, x1)
	v = np.dot(W, y)
	return np.linalg.lstsq(Z, v)[0]

clusters = [
	(-1, 0, 1.0, 0.5),
	( 1, 0, 1.0, 0.5)
]
nclusters = len(clusters)
cluster_memberships = []

data = np.array(gen_linear_data(NDATA, clusters, cluster_memberships))
xdata = data[:,0]
ydata = data[:,1]

plt.scatter(xdata, ydata)
for a, b, std, _ in clusters:
	plot_linear_gaussian(a, b, std, 'b-')
plt.show()

#guesses = [(random.random(), random.random(), random.random(), 1.0 / nclusters) for i in range(nclusters)]
guesses = []
#guesses = clusters[:]

Ls = []
cluster_increase_times = []

for iters in range(NITERS):
	#print('Iteration', iter)
	
	nclusters = len(guesses)
	
	# E step
	
	Pij = np.zeros((nclusters, NDATA))
	Pi = PNOISE * np.ones(nclusters)
	Pj = PNOISE * np.ones(NDATA)
	map_cluster = [-1] * NDATA

	#import pdb; pdb.set_trace()
	for i, (a, b, std, weight) in enumerate(guesses):
		for j, (x, y) in enumerate(data):
			if i == 1 and j == 4:
				import pdb; pdb.set_trace()
			Pij[i, j] = (1.0 - EPSILON) * weight * linear_gaussian_pdf(x, y, a, b, std)
			Pj[j] += Pij[i, j]
			if (map_cluster[j] == -1 and Pij[i, j] > PNOISE) or \
			   (map_cluster[j] != -1 and Pij[i, j] > Pij[map_cluster[j], j]):
				map_cluster[j] = i
	

	for i in range(nclusters):
		for j in range(NDATA):
			Pij[i, j] /= Pj[j]
			Pi[i] += Pij[i,j]
			#print('P({},{}) = {}'.format(i, j, Pij[(i,j)]))
	
	Ls.append(log_likelihood(data, guesses))
	
	# M step
	for i in range(nclusters):
		a, b = weighted_linear_regression(xdata, ydata, Pij[i,:] / Pi[i])
		assert(a != 0.)
		std = np.sum(Pij[i,:] * np.power(ydata - (a * xdata + b), 2)) / Pi[i]
		assert(std > 10e-10)
		weight = Pi[i] / np.sum(Pi)
		guesses[i] = (a, b, std, weight)
		#print('Cluster {} = ({}, {}, {})'.format(i, mean, std, weight))
	
	assert(nclusters == 0 or 0.9999999 <= sum([g[3] for g in guesses]) <= 1.000000001)

	# remove useless clusters
	#for i in range(nclusters - 1, -1, -1):
	#	if map_cluster.count(i) <= 5:
	#		guesses.pop(i)
	
	if iters % 20 != 0:
		continue
	
	# possibly create a new cluster
	noise_data = [ j for j in range(NDATA) if map_cluster[j] == -1 ]
	#print('Noise samples {}'.format(len(noise_data)))
	if len(noise_data) > K:
		seed = random.choice(noise_data)
		dists = [ np.sum(np.power(data[j] - data[seed], 2)) for j in range(NDATA) ]
		close = np.argsort(dists)[:K]
		w = np.zeros(NDATA)
		for j in close:
			w[j] = 1.0
		a, b = weighted_linear_regression(xdata, ydata, w)
		std = np.sum(w * np.power(ydata - (a * xdata + b), 2)) / K
		print(std)
		weight = K / float(NDATA)
		guesses.append((a, b, std, weight))
		cluster_increase_times.append(iters)
		
plt.plot(Ls)
for t in cluster_increase_times:
	plt.axvline(x = t)
plt.show()

plt.scatter(xdata, ydata)
for a, b, std, _ in guesses:
	plot_linear_gaussian(a, b, std, 'r-')
for a, b, std, _ in clusters:
	plot_linear_gaussian(a, b, std, 'b-')
	
plt.show()

