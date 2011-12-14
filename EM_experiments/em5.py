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

MIN = -20
MAX = 20
K = 5
NITERS = 20
STD = 1.0

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
	
	for i, (a, b, _) in enumerate(guesses):
		color = COLOR_CYCLE[i % len(COLOR_CYCLE)]
		cm.plot_linear_gaussian(a, b, MIN, MAX, color + '--')
		members = [j for j in range(len(map_cluster)) if map_cluster[j] == i]
		if len(members) > 0:
			x = data[members, 0]
			y = data[members, 1]
			plt.scatter(x, y, c = color)
	for a, b, std, min, max, _ in clusters:
		cm.plot_linear_gaussian(a, b, min, max, 'k-')
	
	plt.show()

def model_error(guesses, X, Y, Pij):
	if len(guesses) == 0:
		return 0.0
	best = [np.argmax(Pij[:, j]) for j in range(len(X))]
	A = np.array([guesses[i][0] for i in best])
	B = np.array([guesses[i][1] for i in best])
	predictions = A * X + B
	return np.sum(np.power(Y - predictions, 2))
	
clusters = [
	(-1,  0, STD, -20,  0, 0.3),
	( 1,  0, STD,   0, 10, 0.3),
	(-1, 20, STD,  10, 20, 0.4)
]

nclusters = len(clusters)

epsilon = 0.01
Pnoise = epsilon / (MAX - MIN)
logPnoise = m.log(Pnoise)

data, cluster_memberships = cm.gen_linear_data(50, clusters, epsilon, MIN, MAX)
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

E = []
cluster_increase_times = []

for iters in range(NITERS):
	#print('Iteration', iters)
	nclusters = len(guesses)
	
	if nclusters == 0:
		eligibility = np.zeros((0, ndata))
	else:
		centerdist = np.array([[(x - guesses[i][2]) ** 2 for x in xdata] for i in range(nclusters)])
		eligibility = np.zeros((nclusters, ndata))
		for j in range(ndata):
			eligibility[np.argmin(centerdist[:, j]), j] = 1.0
	
	print(eligibility)
	#pdb.set_trace()
	# E step
	Pij = np.zeros((nclusters, ndata))
	map_cluster = [-1] * ndata
	#import pdb; pdb.set_trace()
	for i, (a, b, _) in enumerate(guesses):
		for j, (x, y) in enumerate(data):
			if eligibility[i, j] > 0:
				weight = 1.0 / np.sum(eligibility[:, j])
				Pij[i, j] = (1.0 - epsilon) * weight * cm.linear_gaussian_pdf(x, y, a, b, STD)
				if (map_cluster[j] == -1 and (epsilon == 0. or Pij[i, j] > Pnoise)) or \
				   (map_cluster[j] != -1 and Pij[i, j] > Pij[map_cluster[j], j]):
					map_cluster[j] = i

	#print(map_cluster)

	print('---')
	for g in guesses:
		print(g)
	plot_guesses(guesses, clusters, data, map_cluster)

	Pj = np.sum(Pij, axis = 0) + Pnoise
	normPij = Pij / Pj
	
	Pi = np.array([np.sum(normPij[i,:]) for i in range(nclusters)])
	PiTotal = np.sum(Pi)

	w = np.zeros((nclusters, ndata))
	for i in range(nclusters):
		for j in range(ndata):
			w[i, j] = normPij[i, j] / Pi[i]
	
	E.append(model_error(guesses, xdata, ydata, Pij))
	#Ls.append(cm.linear_log_likelihood(data, guesses, epsilon, 1.0 / (MAX - MIN)))
	
	#pdb.set_trace()
	# M step
	#epsilon = min(1.0, (ndata * epsilon * Pnoise) / (sum(normalizer)))
	#epsilon = float(map_cluster.count(-1)) / ndata
	#print('epsilon =', epsilon)
	for i in range(nclusters):
		a, b = weighted_linear_regression(xdata, ydata, w[i,:])
		#std = max(1.0e-10, np.sum(w[i,:] * np.power(ydata - (a * xdata + b), 2)))
		center = np.sum(eligibility[i, :] * xdata) / np.sum(eligibility[i, :])
		guesses[i] = (a, b, center)
		#print('Cluster {} = ({}, {}, {})'.format(i, mean, std, center))

	# remove useless clusters
	for i in range(nclusters - 1, -1, -1):
		if map_cluster.count(i) < 2:
			guesses.pop(i)
	
	if iters % 2 != 0:
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
		#std = np.sum(np.power(yclose - (a * xclose + b), 2)) / K
		center = np.mean(xclose)
		guesses.append((a, b, center))
		cluster_increase_times.append(iters)
	
		
plt.plot(E)
for t in cluster_increase_times:
	plt.axvline(x = t)
plt.show()
