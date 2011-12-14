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
import matplotlib.pyplot as plt

MIN = -20
MAX = 20
K = 10
PNOISE = 1.0 / (MAX - MIN)

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

def mixture_pdf(x, params):
	return sum(w * gaussian_pdf(x, m, s) for m, s, w in params)

def log_likelihood(X, params, e):
	return sum(m.log(e * PNOISE + (1-e) * mixture_pdf(x, params)) for x in X)
	
def plot_gaussian(mean, std, style):
	X = np.arange(-30, 30, 0.1)
	Y = [gaussian_pdf(x, mean, std) for x in X]
	plt.plot(X, Y, style)

def gen_data(n, clusters, epsilon):
	data = []
	cluster_probs = [c[2] for c in clusters]
	for i in range(n):
		if random.random() < epsilon:
			data.append(random.uniform(MIN, MAX))
		else:
			c = nonuniform_int(cluster_probs)
			mean, std, _ = clusters[c]
			while True:
				r = random.gauss(mean, std)
				if MIN <= r <= MAX:
					data.append(r)
					break
	
	return data

clusters = [
	( -3.0, 1.0, 0.3),
	(  0.0, 1.0, 0.3),
	( 10.0, 2.0, 0.4)
]

data = gen_data(100, clusters, 0.1)
ndata = len(data)

#guesses = [(random.random(), random.random(), 1.0 / nclusters) for i in range(nclusters)]
guesses = []
#guesses = clusters[:]
epsilon = 0.1

Ls = []
cluster_increase_times = []

for iters in range(100):
	#print('Iteration', iter)
	
	nclusters = len(guesses)
	
	# E step
	
	Pi = [0.0] * nclusters
	Pij = {}
	normalizer = [epsilon * PNOISE] * ndata
	map_cluster = [-1] * ndata
	
	for i, (mean, std, weight) in enumerate(guesses):
		for j, x in enumerate(data):
			Pij[(i, j)] = (1.0 - epsilon) * gaussian_pdf(x, mean, std) * weight
			assert(Pij[(i, j)] >= 0)
			normalizer[j] += Pij[(i, j)]
			if (map_cluster[j] == -1 and Pij[(i, j)] > epsilon * PNOISE) or \
			   (map_cluster[j] != -1 and Pij[(i, j)] > Pij[(map_cluster[j], j)]):
				map_cluster[j] = i
	
	# normalize everything
	for i in range(nclusters):
		for j in range(ndata):
			Pij[(i, j)] /= normalizer[j]
			Pi[i] += Pij[(i,j)]
			#print('P({},{}) = {}'.format(i, j, Pij[(i,j)]))
	
	Ls.append(log_likelihood(data, guesses, epsilon))
	
	# M step
	#epsilon = min(1.0, (ndata * epsilon * PNOISE) / (sum(normalizer)))
	#epsilon = float(map_cluster.count(-1)) / ndata
	#print('epsilon =', epsilon)
	for i in range(nclusters):
		mean = 0.0
		for j in range(ndata):
			mean += Pij[(i, j)] * data[j]
		mean /= Pi[i]
		
		std = 0.0
		for j in range(ndata):
			std += Pij[(i, j)] * ((data[j] - mean) ** 2)
		std = m.sqrt(std / Pi[i])
		
		weight = Pi[i] / sum(Pi)
		
		guesses[i] = (mean, std, weight)
		
		#print('Cluster {} = ({}, {}, {})'.format(i, mean, std, weight))
	
	assert(nclusters == 0 or 0.9999999 <= sum([g[2] for g in guesses]) <= 1.000000001)

	# remove useless clusters
	for i in range(nclusters - 1, -1, -1):
		if map_cluster.count(i) <= 5:
			guesses.pop(i)
	
	if iters % 20 != 0:
		continue
	
	# possibly create a new cluster
	noise_data = [ j for j in range(ndata) if map_cluster[j] == -1 ]
	#print('Noise samples {}'.format(len(noise_data)))
	if len(noise_data) > K:
		seed = random.choice(noise_data)
		dists = [ (data[j] - data[seed]) ** 2 for j in range(ndata) ]
		close = np.argsort(dists)[:K]
		mean = sum(data[j] for j in close) / K
		std = sum((data[j] - mean) ** 2 for j in close) / K
		weight = K / float(ndata)
		guesses.append((mean, std, weight))
		cluster_increase_times.append(iters)
		
plt.plot(Ls)
for t in cluster_increase_times:
	plt.axvline(x = t)
plt.show()

plt.ylim((0.0, .4))
plt.scatter(data, [0] * ndata)
for mean, std, _ in guesses:
	plot_gaussian(mean, std, 'r-')
for mean, std, _ in clusters:
	plot_gaussian(mean, std, 'b-')
	
plt.show()

