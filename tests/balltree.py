from __future__ import print_function
import sys, os, math, random
import numpy as np
import heapq
import Tkinter as tk
import itertools as itl
import time
import scipy

LEAFN = 10

def distsq(a, b):
	return np.sum(np.power(a - b, 2))

def dist(a, b):
	return math.sqrt(distsq(a, b))

# a is a matrix of M, N, b is a vector of N
# return a vector of N for the distance of each row in a from b
def mdistsq(a, b):
	return np.sum(np.power(a - b, 2), axis=1)

def contain_trees(a, b):
	dir = a.center - b.center
	dir = dir / np.linalg.norm(dir)
	p1 = a.center + dir * a.radius
	p2 = b.center - dir * b.radius
	center = (p1 + p2) / 2.0
	radius = dist(center, p1)
	return center, radius

# return centroid, radius of a set of points
def bounding_sphere(pts):
	c = np.sum(pts, axis=0) / len(pts)
	r = math.sqrt(np.max(mdistsq(pts, c)))
	return c, r
	
def sphere_volume(r, n):
	if n % 2 == 0:
		C = (np.pi ** (n / 2)) / scipy.factorial(n / 2)
	else:
		df = np.product(np.array(range(1, n+1, 2)))
		C = (2 ** ((n+1) / 2)) * (np.pi ** ((n - 1) / 2)) / df
	
	return C * (r ** n)
	
class KQueue(object):
	def __init__(self, k, min):
		self.q = []
		self.k = k
		self.dir = 1 if min else -1
	
	def push(self, eval, item):
		#import pdb; pdb.set_trace()
		if self.will_push(eval):
			heapq.heappush(self.q, (self.dir * eval, item))
			if len(self.q) > self.k:
				heapq.heappop(self.q)
	
	def push_all(self, eval_items):
		for e, i in eval_items:
			self.push(e, i)
	
	def will_push(self, eval):
		return len(self.q) < self.k or (len(self.q) > 0 and self.dir * eval > self.q[0][0])
		
	def max(self):
		if len(self.q) == 0:
			return None
		return self.dir * self.q[0][0]
	
	def maxitem(self):
		if len(self.q) == 0:
			return None
		return self.q[0][1]
		
	def pop(self):
		eval, item = self.q[0]
		heapq.heappop(self.q)
		return self.dir * eval, item
	
	def items(self):
		return zip(*self.q)[1]
	
	def __len__(self):
		return len(self.q)
	
	def full(self):
		return len(self.q) >= self.k

class BallTree(object):
	def __init__(self, pts, inds=None, left=None, right=None):
		self.left = left
		self.right = right
		self.parent = None
		self.pts = pts
		self.ndim = len(pts[0])

		if inds == None and self.left == None:
			self.inds = range(len(self.pts))
		else:
			self.inds = inds
		
		if self.inds != None:
			self.npts = len(self.inds)   # total points in subtree
			self.local = self.pts[self.inds]
		else:
			self.npts = self.left.npts + self.right.npts
		
		self.update_ball()
		if self.inds != None and len(self.inds) > LEAFN:
			self.split()
			
	def update_ball(self):
		if self.left != None:
			self.center, self.radius = contain_trees(self.left, self.right)
		else:
			self.center = np.sum(self.local, axis=0) / self.npts
			farthest = np.argmax(mdistsq(self.local, self.center))
			self.radius = dist(self.center, self.local[farthest])
		
		if self.parent != None:
			self.parent.update_ball()
	
	def split(self):
		l = np.argmax(mdistsq(self.local, self.center))
		dl = mdistsq(self.local, self.local[l])
		r = np.argmax(dl)                 # farthest from l
		dr = mdistsq(self.local, self.local[r])
		linds = [self.inds[i] for i in np.where(np.less(dl, dr))[0]]
		rinds = [self.inds[i] for i in np.where(np.greater_equal(dl, dr))[0]]
		self.inds = None
		self.left = BallTree(self.pts, linds)
		self.left.parent = self
		self.right = BallTree(self.pts, rinds)
		self.right.parent = self
		self.update_ball()

	def split1(self):
		bestvol = None
		for l in range(len(self.local)):
			dl = mdistsq(self.local, self.local[l])
			r = np.argmax(dl)
			dr = mdistsq(self.local, self.local[r])
			linds = [self.inds[i] for i in np.where(np.less(dl, dr))[0]]
			rinds = [self.inds[i] for i in np.where(np.greater_equal(dl, dr))[0]]
			assert(len(linds) + len(rinds) == len(self.inds))
			lc, lr = bounding_sphere(self.pts[linds])
			rc, rr = bounding_sphere(self.pts[rinds])
			v = lr ** self.ndim + rr ** self.ndim
			if bestvol == None or v < bestvol:
				bestvol = v
				bestl = linds
				bestr = rinds
		
		self.left = BallTree(self.pts, bestl)
		self.left.parent = self
		self.right = BallTree(self.pts, bestr)
		self.right.parent = self
		self.update_ball()
	
	def split2(self):
		bestvol = None
		for l, r in itl.combinations(range(len(self.local)), 2):
			dl = mdistsq(self.local, self.local[l])
			dr = mdistsq(self.local, self.local[r])
			linds = [self.inds[i] for i in np.where(np.less(dl, dr))[0]]
			rinds = [self.inds[i] for i in np.where(np.greater_equal(dl, dr))[0]]
			lc, lr = bounding_sphere(self.pts[linds])
			rc, rr = bounding_sphere(self.pts[rinds])
			v = lr ** self.ndim + rr ** self.ndim
			if bestvol == None or v < bestvol:
				bestvol = v
				bestl = linds
				bestr = rinds
		
		self.left = BallTree(self.pts, bestl)
		self.left.parent = self
		self.right = BallTree(self.pts, bestr)
		self.right.parent = self
		self.update_ball()
		
	
	def linear_search(self, q, nn):
		if self.inds == None:
			return
		
		for i, d in enumerate(mdistsq(self.local, q)):
			nn.push(d, self.inds[i])
		
	def knn(self, q, nn):
		dmin = max(dist(self.center, q) - self.radius, 0.0) ** 2
		if nn.full() and dmin > nn.max():
			return self.npts, 0
		
		if self.left == None:
			self.linear_search(q, nn)
			return 0, 1
		
		p1, s1 = self.left.knn(q, nn)
		p2, s2 = self.right.knn(q, nn)
		
		return p1 + p2, s1 + s2 + 1

	def volume(self):
		return sphere_volume(self.radius, self.ndim)
		
	def total_volume(self):
		if self.left == None:
			return self.volume()
		else:
			return self.volume() + self.left.total_volume() + self.right.total_volume()
	
	def size(self):
		if self.left == None:
			return 1
		return 1 + self.left.size() + self.right.size()

class Visualizer(object):
	def __init__(self, data, tree):
		self.data = data
		self.tree = tree
		self.win = tk.Tk()
		self.canvas = tk.Canvas(self.win)
		#self.canvas.bind('<Button-1>', self.add_point)
		self.canvas.bind('<Button-3>', self.query)
		self.canvas.pack(expand=True, fill=tk.BOTH)
		self.draw()
	
	def add_point(self, evt):
		p = np.array([[evt.x, evt.y]])
		self.tree.insert(p)
		self.canvas.delete(tk.ALL)
		self.draw()
	
	def query(self, evt):
		q = np.array([[evt.x, evt.y]])
		nn = KQueue(3, False)
		self.tree.knn(q, nn)
		self.canvas.delete(tk.ALL)
		self.draw()
		for x, y in self.data[nn.items()]:
			self.canvas.create_oval((x-2,y-2,x+2,y+2), outline='blue')
	
	def draw(self, node=None):
		if node == None:
			node = self.tree
		
		cx, cy = node.center
		r = node.radius
		self.canvas.create_oval((cx-r,cy-r,cx+r,cy+r), outline='red')
		if node.left != None:
			self.draw(node.left)
			self.draw(node.right)
		else:
			for x, y in node.local:
				self.canvas.create_oval(x-1,y-1,x+1,y+1, fill='black')

def brute_force(q, data, k):
	di = heapq.nsmallest(k, zip(mdistsq(data, q), itl.count()))
	return zip(*di)[1]

def top_down(data):
	return BallTree(data)
	
def bottom_up(data):
	nodes = [BallTree(data, [i]) for i in range(len(data))]

	while len(nodes) > 1:
		best = None
		for i, j in itl.combinations(range(len(nodes)), 2):
			t = BallTree(data, None, nodes[i], nodes[j])
			v = t.volume()
			if best == None or bestvol > v:
				best = t
				bestvol = v
				besti = i
				bestj = j
		
		if besti < bestj:
			nodes.pop(bestj)
			nodes.pop(besti)
		else:
			nodes.pop(besti)
			nodes.pop(bestj)
		
		nodes.append(best)
	
	return nodes[0]
	
NDIM = 5
K = 3

if __name__ == '__main__':
	#np.random.seed(2)
	data = np.random.rand(100000, NDIM)
	q = np.random.rand(NDIM)
	#q = np.zeros(NDIM)

	t = time.time()
	nn1 = brute_force(q, data, K)
	print('brute force:', time.time() - t)

	for method in [top_down]:
		t = time.time()
		tree = method(data)
		print('ball tree construction:', time.time() - t)
		print('ball tree size', tree.size())
		print('ball tree volume:', tree.total_volume())
		t = time.time()
	
		t = time.time()
		nn2 = KQueue(K, False)
		pruned, searched = tree.knn(q, nn2)
		print('pruned', pruned, 'searched', searched)
		print('ball tree query:', time.time() - t)

		if set(nn1) != set(nn2.items()):
			print('MISMATCH 1')
			print(set(nn1))
			print(set(nn2.items()))

if __name__ == 'a__main__':
	#np.random.seed(10)
	data = np.random.rand(100, 2) * 500
	tree = top_down(data)
	vis = Visualizer(data, tree)
	tk.mainloop()
	