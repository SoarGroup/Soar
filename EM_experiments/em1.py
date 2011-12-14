#!/usr/bin/env python

from __future__ import print_function
import math as m
import numpy as np
import random
import matplotlib.pyplot as plt
from common import *

random.seed(1)
		
def gen_data(n, clusters):
	data = []
	cluster_probs = [c[2] for c in clusters]
	for i in range(n):
		c = nonuniform_int(cluster_probs)
		mean, std, _ = clusters[c]
		data.append(random.gauss(mean, std))
	
	return data

clusters = [
	( 3.0, 1.0, .5),
	(-3.0, 1.0, .5)
	#( 4.0, 1.0, 0.7)
]

nclusters = len(clusters)
data = gen_data(10, clusters)
ndata = len(data)

guesses = [(random.random(), random.random(), 1.0 / nclusters) for i in range(nclusters)]
#guesses = [( 0.351451, 0.03865, 1.0)]
#guesses = [(3, 1, 0.5), (-3, 1, 0.5)]

Ls = []

for iter in range(100):
	#print('Iteration', iter)
	Ls.append(log_likelihood(data, guesses, 0))
	
	# E step

	logPij = np.zeros((nclusters, ndata))
	for i, (mean, std, weight) in enumerate(guesses):
		for j, x in enumerate(data):
			logPij[i, j] = log_gaussian_pdf(x, mean, std) + m.log(weight)
	
	logPj = np.array([log_sum(logPij[:,j]) for j in range(ndata)])
	normLogPij = np.zeros((nclusters, ndata))
	for j in range(ndata):
		normLogPij[:,j] = logPij[:,j] - logPj[j]
	
	logPi = np.array([log_sum(normLogPij[i,:]) for i in range(nclusters)])
	Pi = np.exp(logPi)
	
	w = np.zeros((nclusters, ndata))
	for i in range(nclusters):
		for j in range(ndata):
			w[i, j] = m.exp(normLogPij[i, j] - logPi[i])
	
	PiTotal = np.sum(Pi)

	# M step
	for i in range(nclusters):
		mean = 0.
		for j in range(ndata):
			mean += w[i, j] * data[j]
		
		std = 0.
		for j in range(ndata):
			std += w[i, j] * ((data[j] - mean) ** 2)
		
		std = m.sqrt(std)
		
		weight = Pi[i] / PiTotal
		
		guesses[i] = (mean, std, weight)
		
		#print('Cluster {} = ({}, {}, {})'.format(i, mean, std, weight))
	
	assert(0.999999999 < sum([g[2] for g in guesses]) < 1.000000001)

plt.plot(Ls)
plt.show()

plt.scatter(data, [0] * ndata)
for mean, std, _ in guesses:
	plot_gaussian(mean, std, 'r-')
for mean, std, _ in clusters:
	plot_gaussian(mean, std, 'b-')
	
plt.show()

