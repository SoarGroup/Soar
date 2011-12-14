#!/usr/bin/env python

# EM using the MAP cluster indexes to calculate expectations rather than
# expected cluster indexes. This doesn't seem to work well in general:
# with 2 clusters, it's almost always the case that one cluster will
# immediately dominate all points while the other cluster gets assigned
# a weight of 0.

from __future__ import print_function
import math as m
import numpy as np
import random
import matplotlib.pyplot as plt

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

def gauss_pdf(x, mean, std):
	return (1.0 / (std * m.sqrt(2 * m.pi))) * m.exp(-((x - mean) ** 2) / (2 * std * std))

def plot_gaussian(mean, std, style):
	X = np.arange(-10, 10, 0.1)
	Y = [gauss_pdf(x, mean, std) for x in X]
	plt.plot(X, Y, style)

def gen_data(n, clusters, epsilon):
	data = []
	cluster_probs = [c[2] for c in clusters]
	for i in range(n):
		c = nonuniform_int(cluster_probs)
		mean, std, _ = clusters[c]
		data.append(random.gauss(mean, std))
	
	return data

clusters = [
	( 3.0, 1.0, .6),
	(-3.0, 1.0, .4)
	#( 4.0, 1.0, 0.7)
]

nclusters = len(clusters)
data = gen_data(100, clusters, 1.0)
ndata = len(data)

guesses = [(random.random(), random.random(), 1.0 / nclusters) for i in range(nclusters)]
#guesses = [(5.0, 1.0, 1.0)]
#guesses = []

Ls = []

for iter in range(20):
	print('Iteration', iter)
	# E step
	
	Pij = {}
	
	map_cluster = [0] * ndata
	for j, x in enumerate(data):
		for i, (mean, std, weight) in enumerate(guesses):
			Pij[(i, j)] = gauss_pdf(x, mean, std) * weight
			if Pij[(i, j)] > Pij[(map_cluster[j], j)]:
				map_cluster[j] = i
	
	members = [[j for j in range(ndata) if map_cluster[j] == i] for i in range(nclusters)]
	#print(map_cluster)
	#print(members)
	
	L = 1.0
	for j in range(ndata):
		mean, std, weight = guesses[map_cluster[j]]
		L *= weight * gauss_pdf(data[j], mean, std)
	
	Ls.append(L)
	
	# M step
	for i in range(nclusters):
		mean, std, weight = guesses[i]
		print('Cluster {} = ({}, {}, {})'.format(i, mean, std, weight))
		if len(members[i]) == 0:
			mean, std, weight = guesses[i]
			guesses[i] = (mean, std, 0.0)
			continue
			
		mean = 0.0
		for j in members[i]:
			mean += data[j]
		
		mean /= len(members[i])
		
		std = 0.0
		for j in members[i]:
			std += ((data[j] - mean) ** 2)
		
		std = m.sqrt(std / len(members[i]))
		
		weight = float(len(members[i])) / ndata
		
		guesses[i] = (mean, std, weight)
		
	assert(sum([g[2] for g in guesses]) == 1.0)

plt.plot(Ls)
plt.show()

plt.scatter(data, [0] * ndata)
for mean, std, _ in guesses:
	plot_gaussian(mean, std, 'r-')
for mean, std, _ in clusters:
	plot_gaussian(mean, std, 'b-')
	
plt.show()

