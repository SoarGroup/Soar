#!/usr/bin/env python

from __future__ import print_function
import math as m
import numpy as np
import random
import matplotlib.pyplot as plt

random.seed(1)

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

def mixture_pdf(X, params):
	return sum(m.log(sum(w * gaussian_pdf(x, m, s) for m, s, w in params)) for x in X)

def plot_gauss(mean, std, style):
	X = np.arange(-10, 10, 0.1)
	Y = [gaussian_pdf(x, mean, std) for x in X]
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
	( 3.0, 1.0, 0.7),
	(-3.0, 1.0, 0.3)
	#( 4.0, 1.0, 0.7)
]

#guesses = [(random.random(), random.random(), 1.0 / nclusters) for i in range(nclusters)]
guesses = [
	( 5.0, 2.0, .5), 
	( 1.0, 1.0, .5)
]

nclusters = len(clusters)
data = gen_data(100, clusters, 0.5)
ndata = len(data)


Ls = []

for iter in range(100):
	#print('Iteration', iter)
	# E step
	
	Pij = np.zeros((nclusters, ndata))
	Pi = np.zeros(nclusters)
	Pj = np.zeros(ndata)
	
	for i, (mean, std, weight) in enumerate(guesses):
		for j, x in enumerate(data):
			Pij[i, j] = gaussian_pdf(x, mean, std) * weight
			Pj[j] += Pij[i, j]
	
	# normalize everything
	for i in range(nclusters):
		for j in range(ndata):
			Pij[i, j] /= Pj[j]
			Pi[i] += Pij[i,j]
			#print('P({},{}) = {}'.format(i, j, Pij[(i,j)]))
	
	
	Ls.append(mixture_pdf(data, guesses))
	
	# M step
	for i in range(nclusters):
		mean = 0.0
		for j in range(ndata):
			mean += Pij[i, j] * data[j]
		
		mean /= Pi[i]
		
		std = 0.0
		for j in range(ndata):
			std += Pij[i, j] * ((data[j] - mean) ** 2)
		
		std = m.sqrt(std / Pi[i])
		
		weight = Pi[i] / sum(Pi)
		
		guesses[i] = (mean, std, weight)
		
		#print('Cluster {} = ({}, {}, {})'.format(i, mean, std, weight))
	
	assert(0.999999999 < sum([g[2] for g in guesses]) < 1.000000001)

plt.plot(Ls)
plt.show()

plt.scatter(data, [0] * ndata)
for mean, std, _ in guesses:
	plot_gauss(mean, std, 'r-')
for mean, std, _ in clusters:
	plot_gauss(mean, std, 'b-')
	
plt.show()

