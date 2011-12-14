#!/usr/bin/env python

from __future__ import print_function
import math as m
import numpy as np
import random
import matplotlib.pyplot as plt

# Calculate log(sum(Pi for i = 0 .. n)) given logPi for i = 0 .. n.
# The naive way to calculate this is to exponentiate all logPi
# to get Pi, sum them, and take log. This may result in underflow
# errors if some of the logPij are very small (< -700). This
# problem can be avoided with a trick. See
#
# http://en.wikipedia.org/wiki/List_of_logarithmic_identities#Summation.2Fsubtraction

def log_sum(L):
	assert(len(L) > 0)
	maxdiffs = np.array([np.max(L - x) for x in L])
	min_i = np.argmin(maxdiffs)
	log_i = L[min_i]
	s = 0.
	for i, v in enumerate(L):
		if i != min_i:
			s += m.exp(L[i] - log_i)
	return log_i + m.log(1. + s)

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

def log_gaussian_pdf(x, mean, std):
	return -m.log(std * m.sqrt(2 * m.pi)) - ((x - mean) ** 2) / ( 2 * (std ** 2))

def linear_gaussian_pdf(x, y, a, b, std):
	return gaussian_pdf(y, np.dot(a, x) + b, std)
	
def log_linear_gaussian_pdf(x, y, a, b, std):
	return log_gaussian_pdf(y, a * x + b, std)

def log_mixture_pdf(x, params):
	return log_sum(np.array([m.log(w) + log_gaussian_pdf(x, mean, s) for mean, s, w in params]))

def log_linear_mixture_pdf(x, y, params):
	return log_sum(np.array([m.log(w) + log_linear_gaussian_pdf(x, y, a, b, s) for a, b, s, w in params]))

def linear_mixture_pdf(x, y, params):
	return sum(w * linear_gaussian_pdf(x, y, a, b, s) for a, b, s, w in params)

def log_likelihood(X, params, e, pnoise):
	if len(params) == 0:
		return m.log(e * pnoise) * len(X)
		
	if (e == 0.):
		return sum(log_mixture_pdf(x, params) for x in X)
	elif (e == 1.):
		return m.log(e * pnoise) * len(X)
	else:
		lognoise = m.log(e * pnoise)
		log1e = m.log(1-e)
		return sum(log_sum(np.array([lognoise, log1e + log_mixture_pdf(x, params)])) for x in X)

def linear_log_likelihood(XY, params, e, pnoise):
	if len(params) == 0:
		return m.log(e * pnoise) * len(XY)
		
	if (e == 0.):
		return sum(log_linear_mixture_pdf(x, y, params) for x, y in XY)
	elif (e == 1.):
		return m.log(e * pnoise) * len(XY)
	else:
		lognoise = m.log(e * pnoise)
		log1e = m.log(1-e)
		return sum(log_sum(np.array([lognoise, log1e + log_linear_mixture_pdf(x, y, params)])) for x, y in XY)

def plot_gaussian(mean, std, style):
	X = np.arange(-10, 10, 0.1)
	Y = [gaussian_pdf(x, mean, std) for x in X]
	plt.plot(X, Y, style)

def plot_linear_gaussian(a, b, min, max, style):
	X = [min, max]
	Y = [a * x + b for x in [min, max]]
	plt.plot(X, Y, style)

def gen_data(n, clusters, epsilon, min, max):
	data = np.zeros(n)
	membership = []
	cluster_probs = [c[2] for c in clusters]
	for i in range(n):
		if random.random() < epsilon:
			data[i] = random.uniform(min, max)
			membership.append(-1)
		else:
			c = nonuniform_int(cluster_probs)
			mean, std, _ = clusters[c]
			data[i] = random.gauss(mean, std)
			membership.append(c)
	
	return data, membership
	
def gen_linear_data(n, clusters, epsilon, noise_min, noise_max):
	data = np.zeros((n, 2))
	membership = []
	cluster_probs = [c[-1] for c in clusters]
	for i in range(n):
		if random.random() < epsilon:
			data[i, :] = random.uniform(noise_min, noise_max), random.uniform(noise_min, noise_max)
			membership.append(-1)
		else:
			c = nonuniform_int(cluster_probs)
			a, b, std, cmin, cmax, _ = clusters[c]
			data[i, 0] = random.uniform(cmin, cmax)
			mean = a * data[i, 0] + b
			data[i, 1] = random.gauss(mean, std)
			membership.append(c)

	return data, membership

def make_arff(srcdata, arff):
	attribs = 'spx spy lrps rrps svx svy sav bpx bpy ba bvx bvy pbpx'.split()
	out = open(arff, 'w')
	print('@RELATION whatever', file = out)
	for a in attribs:
		print('@ATTRIBUTE {} NUMERIC'.format(a), file = out)
	print('@DATA', file = out)
	for line in open(srcdata):
		sline = line.strip()
		if len(sline) == 0 or sline.startswith('#'):
			continue
		x = sline.split()[:12]
		x.append(sline.split()[20])
		print(','.join(x), file=out)

def make_arff_clusters(srcdata, arff, clusters):
	attribs = 'spx spy lrps rrps svx svy sav bpx bpy ba bvx bvy'.split()
	out = open(arff, 'w')
	print('@RELATION clusters', file = out)
	for a in attribs:
		print('@ATTRIBUTE {} NUMERIC'.format(a), file = out)
	print('@ATTRIBUTE cluster {{ {} }}'.format(','.join(str(c) for c in np.unique(clusters))), file = out)
	print('@DATA', file = out)
	n = 0
	for line in open(srcdata):
		sline = line.strip()
		if len(sline) == 0 or sline.startswith('#'):
			continue
		x = sline.split()[:12]
		x.append(str(clusters[n]))
		print(','.join(x), file=out)
		n += 1
		