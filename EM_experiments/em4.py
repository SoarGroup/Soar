#!/usr/bin/env python

# Data is generated either from noise with epsilon probability and
# gaussians otherwise. Algorithm initially starts with no clusters and adds
# additional clusters as needed. There needs to be a delay between each
# cluster addition to allow a newly added cluster to adjust its parameters
# to cover the largest number of data points. Currently I'm just enforcing
# an arbitrary number of iterations between introducing new clusters.

from __future__ import print_function
import math as m
import numpy as np
import random
import itertools as itl
import matplotlib.pyplot as plt
import common as cm
import pdb

MIN = -100
MAX = 100
K = 5
NITERS = 10

COLOR_CYCLE = 'bgrcmyk'
#random.seed(0)

def weighted_linear_regression(x, y, w):
	x1 = np.transpose(np.vstack((x, np.ones(x.shape[0]))))
	W = np.diag(w) 
	Z = np.dot(W, x1)
	v = np.dot(W, y)
	return np.linalg.lstsq(Z, v)[0]

def plot_guesses(guesses, clusters, data, map_cluster):
	# noise points
	members = [i for i in range(len(map_cluster)) if map_cluster[i] == -1]
	if len(members) > 0:
		x = data[members, 0]
		y = data[members, 1]
		plt.scatter(x, y, c = 'w')
	
	for i, (a, b, std, _) in enumerate(guesses):
		color = COLOR_CYCLE[i % len(COLOR_CYCLE)]
		cm.plot_linear_gaussian(a, b, std, MIN, MAX, color + '--')
		members = [j for j in range(len(map_cluster)) if map_cluster[j] == i]
		if len(members) > 0:
			x = data[members, 0]
			y = data[members, 1]
			plt.scatter(x, y, c = color)
	for a, b, std, min, max, _ in clusters:
		cm.plot_linear_gaussian(a, b, std, min, max, 'k-')
	
	plt.show()

clusters = [
	(-1, 0, 1.0, -20,  0, 1.0),
	#( 1, 0, 1.0,   0, 20, 0.5)
]

nclusters = len(clusters)

epsilon = 0.01
Pnoise = epsilon / (MAX - MIN)
logPnoise = m.log(Pnoise)

data, cluster_memberships = cm.gen_linear_data(100, clusters, epsilon, MIN, MAX)
xdata = data[:,0]
ydata = data[:,1]
ndata = len(data)

#plt.scatter(xdata, ydata)
#for a, b, std, _ in clusters:
#	cm.plot_linear_gaussian(a, b, std, 'b-')
#plt.show()

#guesses = [(random.random(), random.random(), random.random(), 1.0 / nclusters) for i in range(nclusters)]
guesses = []
#guesses = clusters[:]

Ls = []
cluster_increase_times = []

for iters in range(NITERS):
	#print('Iteration', iters)
	#pdb.set_trace()
	nclusters = len(guesses)
	
	# E step
	logPij = np.zeros((nclusters, ndata))
	map_cluster = [-1] * ndata
	#import pdb; pdb.set_trace()
	for i, (a, b, std, weight) in enumerate(guesses):
		for j, (x, y) in enumerate(data):
			logPij[i, j] = m.log(1.0 - epsilon) + m.log(weight) + cm.log_linear_gaussian_pdf(x, y, a, b, std)
			if (map_cluster[j] == -1 and (epsilon == .0 or logPij[i, j] > logPnoise)) or \
			   (map_cluster[j] != -1 and logPij[i, j] > logPij[map_cluster[j], j]):
				map_cluster[j] = i

	#print(map_cluster)

	print('---')
	for g in guesses:
		print(g)
	plot_guesses(guesses, clusters, data, map_cluster)

	logPj = np.array([cm.log_sum(np.append(logPij[:,j], logPnoise)) for j in range(ndata)])
	normLogPij = np.zeros((nclusters, ndata))
	for j in range(ndata):
		normLogPij[:,j] = logPij[:,j] - logPj[j]	
	
	logPi = np.array([cm.log_sum(normLogPij[i,:]) for i in range(nclusters)])
	Pi = np.exp(logPi)
	PiTotal = np.sum(Pi)

	w = np.zeros((nclusters, ndata))
	for i in range(nclusters):
		for j in range(ndata):
			w[i, j] = m.exp(normLogPij[i, j] - logPi[i])
		
	Ls.append(cm.linear_log_likelihood(data, guesses, epsilon, 1.0 / (MAX - MIN)))
	
	# M step
	#epsilon = min(1.0, (ndata * epsilon * PNOISE) / (sum(normalizer)))
	#epsilon = float(map_cluster.count(-1)) / ndata
	#print('epsilon =', epsilon)
	for i in range(nclusters):
		a, b = weighted_linear_regression(xdata, ydata, w[i,:])
		std = max(1.0e-10, np.sum(w[i,:] * np.power(ydata - (a * xdata + b), 2)))
		weight = Pi[i] / PiTotal
		guesses[i] = (a, b, std, weight)
		#print('Cluster {} = ({}, {}, {})'.format(i, mean, std, weight))
	
	assert(nclusters == 0 or 0.9999999 <= sum([g[3] for g in guesses]) <= 1.000000001)

	# remove useless clusters
	for i in range(nclusters - 1, -1, -1):
		if map_cluster.count(i) < 2:
			guesses.pop(i)
	
	if iters % 5 != 0:
		continue
	
	# possibly create a new cluster
	noise_data = [ j for j in range(ndata) if map_cluster[j] == -1 ]
	#print('Noise samples {}'.format(len(noise_data)))
	if len(noise_data) > K:
		seed = random.choice(noise_data)
		dists = [ np.sum(np.power(data[j] - data[seed], 2)) for j in range(ndata) ]
		close = np.argsort(dists)[:K]
		xclose = xdata[close]
		yclose = ydata[close]
		a, b = weighted_linear_regression(xclose, yclose, np.ones(len(close)))
		std = np.sum(np.power(yclose - (a * xclose + b), 2)) / K
		weight = K / float(ndata)
		guesses.append((a, b, std, weight))
		cluster_increase_times.append(iters)
	
		
plt.plot(Ls)
for t in cluster_increase_times:
	plt.axvline(x = t)
plt.show()
