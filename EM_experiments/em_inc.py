#!/usr/bin/env python

# Messing around with incremental EM.
#
# After updating Py_z for some data point zi:
#
# 1. Recalculate MAP models.
# 2. Retrain each model whose support data set changed.
# 3. Mark all data points in each retrained model as eligible for update.
#
# For every data point added:
#
# 1. Calculate Py_z for all y
# 2. Calculate MAP model for point
# 3. Refit models

from __future__ import print_function
import sys
import math as m
import numpy as np
import numpy.ma as mat
import random as rnd
import itertools as itl
import Tkinter as tk
import matplotlib.pyplot as plt
import matplotlib.backends.backend_tkagg as tkagg
import common as cm
import pdb
import cProfile

MIN = -20
MAX = 20
K = 5
NITERS = 20
STD = .1
NDATA = 500
NMDLS = 100
XDIM = 1
YDIM = 1

COLOR_CYCLE = 'bgrcmyk'
rnd.seed(0)


	
clusters = [
	(-1,  0, STD, -20,  0, 0.3),
	( 1,  0, STD,   0, 10, 0.3),
	(-1, 20, STD,  10, 20, 0.4)
]

epsilon = 0.01
Pnoise = epsilon / (MAX - MIN)
logPnoise = m.log(Pnoise)

cluster_increase_times = []

numfits = 0

data, cluster_memberships = cm.gen_linear_data(NDATA, clusters, epsilon, MIN, MAX)
xdata = np.array([[x] for x in data[:,0]])
ydata = data[:,1]

class Model:
	def __init__(self):
		self.xdata_chunk = np.zeros((NDATA, XDIM + 1))    # extra column is all 1's
		self.ydata_chunk = np.zeros((NDATA, YDIM))
		self.ndata = 0
		self.data_inds = {}  # index of data point in global data table -> index of data point in local data table
	
	def resize(self):
		self.xdata = self.xdata_chunk[:self.ndata,:]
		self.ydata = self.ydata_chunk[:self.ndata,:]
		
	def add_data(self, i, x, y):
		self.data_inds[i] = self.ndata
		self.ndata += 1
		self.resize()
		self.xdata[-1, :XDIM] = x
		self.xdata[-1, -1] = 1.
		self.ydata[-1, :] = y
		self.center = np.mean(self.xdata[:,:-1])
	
	def remove_data(self, i):
#		print('supports: ', self.data_inds.keys())
		j = self.data_inds[i]
		del self.data_inds[i]
		for k, l in self.data_inds.items():
			if l > j:
				self.data_inds[k] -= 1
		
		self.xdata[j:self.ndata-1,:] = self.xdata[j+1:,:]
		self.ydata[j:self.ndata-1,:] = self.ydata[j+1:,:]
		self.ndata -= 1
		self.resize()
		
	def fit(self):
		if len(self.xdata) < 2:
			return
#		print('Fitting {} to {}'.format(id(self), ' '.join(str(i) for i in self.data_inds.keys())))
		global numfits
		numfits += 1
		coefs = np.linalg.lstsq(self.xdata, self.ydata)[0]
		self.a = coefs[:-1]
		self.b = coefs[-1]
	
	def predict(self, x):
		return np.dot(self.a, x) + self.b
	
	def likelihood(self, x, y):
		return cm.gaussian_pdf(self.predict(x), y, STD)
	
	def model_error(self, X, Y, Py_z):
		predictions = X * self.a + self.b
		return np.sum(np.power(Y - predictions, 2))

	def __str__(self):
		return '{} {}'.format(self.a, self.b)

	def supports(self):
		return self.data_inds.keys()
		
class EM:
	def __init__(self, fig):
		self.fig = fig
		
		self.ndata = 0
		self.nmodels = 0
		self.models = []
		self.Py_z_chunk = np.zeros((NMDLS, NDATA))
		self.centerdists_chunk = np.zeros(self.Py_z_chunk.shape)
		self.eligibility_chunk = np.zeros(self.Py_z_chunk.shape)
		self.map_model_chunk = np.ones(NDATA, np.int) * -1
		self.xdata_chunk = np.zeros((NDATA, XDIM))
		self.ydata_chunk = np.zeros((NDATA, YDIM))
		self.resize()
		
		self.error = []
		self.stale_models = set()
		self.nstep = 0
		
		if self.fig != None:
			self.axes = self.fig.add_subplot(111)

	def resize(self):
		self.Py_z = self.Py_z_chunk[:self.nmodels, :self.ndata]
		self.centerdists = self.centerdists_chunk[:self.nmodels, :self.ndata]
		self.eligibility = self.eligibility_chunk[:self.nmodels, :self.ndata]
		self.map_model = self.map_model_chunk[:self.ndata]
		self.xdata = self.xdata_chunk[:self.ndata,:]
		self.ydata = self.ydata_chunk[:self.ndata,:]
		
	def add_data(self, x, y):
		self.ndata += 1
		self.resize()
		self.xdata[-1, :] = x
		self.ydata[-1, :] = y
		self.map_model[-1] = -1
		
		for i, m in enumerate(self.models):
			self.centerdists[i, -1] = np.sum(np.power(x - m.center, 2))
		
		self.update_eligibility()
		
		for i, m in enumerate(self.models):
			self.update_Py_z(i, -1)
		
		self.update_map()
		if self.fig != None:
			self.plot_models()
	
	def update_eligibility(self):
		self.eligibility.fill(1)
		for i, x in enumerate(self.xdata):
			for j, m1 in enumerate(self.models):
				for k, m2 in enumerate(self.models):
					if j != k and np.dot(x - m2.center, m1.center - m2.center) < 0:
						self.eligibility[j, i] = 0
						break
	
	def update_Py_z(self, i, j):
		if self.eligibility[i, j] == 0:
			self.Py_z[i, j] = 0.
		else:
			weight = 1.0 / np.sum(self.eligibility[:, j])
			self.Py_z[i, j] = (1.0 - epsilon) * weight * self.models[i].likelihood(self.xdata[j], self.ydata[j])
	
	def update_map(self):
		if self.nmodels == 0:
			self.map_model.fill(-1)
		else:
			prev = np.copy(self.map_model)
			self.map_model[:] = np.argmax(self.Py_z, 0)
			for i in range(self.ndata):
				if self.Py_z[self.map_model[i], i] < Pnoise:
					self.map_model[i] = -1
			for i in np.flatnonzero(prev != self.map_model):
				#print('reassigning point {} from model {} to {}'.format(i, prev[i], self.map_model[i]))
				if prev[i] != -1:
					self.stale_models.add(prev[i])
					self.models[prev[i]].remove_data(i)
				if self.map_model[i] != -1:
					self.stale_models.add(self.map_model[i])
					self.models[self.map_model[i]].add_data(i, self.xdata[i], self.ydata[i])
	
	def Estep(self):
		if len(self.stale_models) == 0:
			return
		
		self.update_eligibility()
		
		i = rnd.sample(self.stale_models, 1)[0]
		for j in range(self.ndata):
			self.update_Py_z(i, j)
		self.stale_models.discard(i)
		
	def Mstep(self):
		self.update_map()
		for i in self.stale_models:
			self.models[i].fit()
			self.centerdists[i,:self.ndata] = np.sum(np.power(self.xdata - self.models[i].center, 2), axis = 1)
	
	def add_model(self):
		noise_data = [ j for j in range(self.ndata) if self.map_model[j] == -1 ]
		#print('Noise samples {}'.format(len(noise_data)))
		if len(noise_data) < K:
			return
			
		seed = rnd.choice(noise_data)
		dists = [ np.sum(np.power(data[j] - data[seed], 2)) for j in range(self.ndata) ]
		close = np.argsort(dists)[:K]
		
		m = Model()
#		print('adding model {} for points {}'.format(id(m), ' '.join(str(i) for i in close)))
		
		for i in close:
			m.add_data(i, self.xdata[i], self.ydata[i])
		
		m.fit()
		self.models.append(m)

		# we can't just assume that the new model fits the points well
		#self.map_model[close] = self.nmodels
		
		self.stale_models.add(self.nmodels)
		#self.cluster_increase_times.append(iters)
		
		self.nmodels += 1
		self.resize()
		self.centerdists[-1, :] = np.sum(np.power(self.xdata - m.center, 2), axis = 1)
	
	def remove_model(self, i):
#		print('removing model {}'.format(i))
		
		for j in range(len(self.map_model)):
			if self.map_model[j] == i:
				self.map_model[j] = -1
			elif self.map_model[j] > i:
				self.map_model[j] -= 1
		
		self.models.pop(i)
		self.Py_z[i:self.nmodels-1,:] = self.Py_z[i+1:,:]
		self.centerdists[i:self.nmodels-1,:] = self.centerdists[i+1:,:]
		self.eligibility[i:self.nmodels-1,:] = self.eligibility[i+1:,:]
		self.nmodels -= 1
		self.resize()
			
	def step(self):
#		print("step {}".format(self.nstep))
		self.nstep += 1
		self.Estep()
		self.Mstep()
		
#		self.error.append(model_error(self.models, self.xdata, self.ydata, self.Py_z))
		#Ls.append(cm.linear_log_likelihood(data, models, epsilon, 1.0 / (MAX - MIN)))
		
		
	def plot_error(self):
		plt.plot(self.error)
		for t in cluster_increase_times:
			plt.axvline(x = t)
		plt.show()

	def run(self, maxiters):
		for i in range(maxiters):
			self.step()
			if len(self.stale_models) == 0:
#				print(' '.join('{}:{}'.format(i, m) for i, m in enumerate(self.map_model)))
				
				for i in range(self.nmodels - 1, -1, -1):
					if np.count_nonzero(self.map_model == i) < 2:
						self.remove_model(i)
				
				if np.count_nonzero(self.map_model == -1) > K:
					self.add_model()
				else:
#					print("stablized")
					return
			else:
				pass
				#print("stale models: {}".format(self.stale_models))
				
	
	def plot_models(self):
		self.axes.clear()
		# noise points
		members = np.flatnonzero(self.map_model[:self.ndata] == -1)
		if len(members) > 0:
			x = self.xdata[members]
			y = self.ydata[members]
			self.axes.scatter(x, y, c = 'w')
		
		for i, m in enumerate(self.models):
			color = COLOR_CYCLE[i % len(COLOR_CYCLE)]
			cm.plot_linear_gaussian(m.a[0], m.b, MIN, MAX, color + '--')
			members = np.flatnonzero(self.map_model[:self.ndata] == i)
			if len(members) > 0:
				x = self.xdata[members]
				y = self.ydata[members]
				self.axes.scatter(x, y, c = color)
		
		for a, b, std, min, max, _ in clusters:
			X = [min, max]
			Y = [np.dot(a, x) + b for x in [min, max]]
			self.axes.plot(X, Y, 'k-')
		
		self.axes.relim()
		self.fig.canvas.draw()
	
	def model_error(self, y):
		if len(self.models) == 0:
			return 0.0
		
		predictions = np.array([models[m].predict(X[i,:]) for i, m in enumerate(self.map_model) if m != -1])
		return np.sum(np.power(y - predictions, 2))

class GUI:
	def __init__(self):
		self.i = 0
		
		self.win = tk.Tk()
		self.fig = plt.figure()
		self.canvas = tkagg.FigureCanvasTkAgg(self.fig, master = self.win)
		self.canvas.get_tk_widget().pack(side='top', fill='x', expand=1)
		self.canvas.show()
		self.em = EM(self.fig)
		
		self.buttons = tk.Frame(self.win)
		self.stepbtn = tk.Button(self.buttons, text = 'step', command = self.step)
		self.addbtn = tk.Button(self.buttons, text = 'add', command = self.add)
		self.quitbtn = tk.Button(self.buttons, text = 'quit', command = self.quit)
		self.replaybtn = tk.Button(self.buttons, text = 'replay', command = self.replay)
		self.stepbtn.pack(side='left')
		self.addbtn.pack(side='left')
		self.quitbtn.pack(side='left')
		self.replaybtn.pack(side='left')
		self.buttons.pack(side='bottom')
		
		self.log = open('gui.log', 'w')
	
	def step(self):
		self.em.run(1)
		self.em.plot_models()
		self.log.write('s\n')
	
	def quit(self):
		self.log.close()
		self.win.quit()
		self.win.destroy()
	
	def add(self):
		if self.i < len(xdata):
			self.em.add_data(xdata[self.i], ydata[self.i])
			self.i += 1
			self.log.write('a\n')
	
	def replay(self):
		self.log.close()
		for line in open('gui.log'):
			if 'a' in line:
				self.add()
			elif 's' in line:
				self.step()

def run_gui():
	gui = GUI()
	tk.mainloop()

def replay():
	f = plt.figure()
	em = EM(f)
	i = 0
	for line in open('gui.log'):
		if 'a' in line:
			em.add_data(xdata[i], ydata[i])
			i += 1
		elif 's' in line:
			em.run(1)
	
	f.show()
	pdb.set_trace()
	em.run(10)

def run_auto():
	em = EM(None)
	for i in range(len(xdata)):
		em.add_data(xdata[i], ydata[i])
		em.run(5)
	
if __name__ == '__main__':
	if len(sys.argv) > 1:
		if sys.argv[1] == 'replay':
			replay()
		elif sys.argv[1] == 'gui':
			run_gui()
	else:
		cProfile.run('run_auto()', 'profile')
		
	