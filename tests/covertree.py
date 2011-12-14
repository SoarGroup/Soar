from __future__ import print_function
import os, sys
import math, random, time
import itertools as itr

def dist(a, b):
	return math.sqrt(sum((ai - bi) ** 2 for ai, bi in zip(a, b)))
	
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
		
	def bound(self):
		if len(self.q) == 0:
			return None
		return self.dir * self.q[0][0]
	
	def bounditem(self):
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

# self.children will only contain children distinct from the node itself
class Node(object):
	def __init__(self, p, level):
		self.p = p
		self.level = level
		self.branches = {}
	
	def add_branch(self, p, level):
		n = Node(p, level - 1)
		self.branches.setdefault(level, []).append(n)
	
	def children(self, level):
		if level > self.level:
			return []
		c = self.branches.get(level, [])[:]
		c.append(self)
		return c

	def check_invariant(self):
		for l, nodes in self.branches.items():
			for n in nodes:
				assert(dist(self.p, n.p) <= 2 ** l)
		
def children(nodes, level):
	return itr.chain(*(n.children(level) for n in nodes))


class CoverTree(object):
	def __init__(self, p):
		self.root = Node(p, 0)
		self.lastbranchlevel = 0
	
	def find_parent(self, p, coverset, level):
		for n in coverset:
			if dist(p, n.p) <= 2 ** level:
				n.add_branch(p, level)
				if level < self.lastbranchlevel:
					self.lastbranchlevel = level
				return True
		return False
	
	def insert(self, p):
		coverset = [self.root]
		if not self.insertrec1(p, coverset, self.root.level):
			# need to move the root up
			self.root.level = int(math.ceil(math.log(dist(p, self.root.p), 2))) # -> 2^i > d(p, self.root.p)
			self.root.add_branch(p, self.root.level)
	
	def insertrec(self, p, coverset, level):
		bound = 2 ** level
		nextset = []
		stoprecurse = True
		for c in children(coverset, level):
			d = dist(p, c.p)
			if d < bound:
				nextset.append(c)
				stoprecurse = False
		
		if stoprecurse:
			return False
				
		inserted = self.insertrec(p, nextset, level-1)
		if not inserted:
			for n in coverset:
				if dist(p, n.p) <= bound:
					n.add_branch(p, level)
					if level < self.lastbranchlevel:
						self.lastbranchlevel = level
					return True
		return inserted
	
	# A version that collapses multiple recursive calls when possible. Apparently much slower than
	# the vanilla version, most likely due to the next branch level calculation.
	def insertrec1(self, p, coverset, level):
		#import pdb; pdb.set_trace()
		bound = 2 ** level
		mindist = min((dist(p, n.p) for n in coverset))
		if mindist > 2 ** level:
			return False
		
		# we only have to check levels with branches and
		# the level where separation is guaranteeed
		separationlevel = int(math.floor(math.log(mindist, 2))) # 2^i < mindist
		# separationlevel is an upper-bound on the insertion level of the point
		# separationlevel + 1 is an upper-bound on the level of the point's parent
		branchpoints = list(itr.chain(*(filter(lambda i: i <= level, n.branches.keys()) for n in coverset)))
		if len(branchpoints) == 0:
			branchlevel = float('-inf')
		else:
			branchlevel = max(branchpoints) - 1
		# branchlevel is the first level where the coverset will have a new node
		
		if separationlevel > branchlevel:
			# no new nodes in coverset between current level and separation level
			assert(self.find_parent(p, coverset, separationlevel + 1))
		else:
			nextcoverset = [ c for c in children(coverset, branchlevel+1) if dist(p, c.p) < 2 ** (branchlevel+1) ]
			if not self.insertrec1(p, nextcoverset, branchlevel):
				assert(separationlevel == branchlevel)
				assert(self.find_parent(p, coverset, separationlevel + 1))
		return True
		
	def nn(self, p):
		#import pdb; pdb.set_trace()
		ndistcalcs = 1
		coverset = { self.root : dist(p, self.root.p) }
		level = self.root.level
		bestdist = dist(self.root.p, p)
		bestnode = self.root
		while level >= self.lastbranchlevel:
			for c in children(coverset.keys(), level):
				d = coverset.get(c, None)
				if d == None:
					d = dist(p, c.p)
					ndistcalcs += 1
					coverset[c] = d
					if d < bestdist:
						bestnode = c
						bestdist = d
			
			coverset = { n : d for n, d in coverset.items() if d <= bestdist + 2 ** level }
			level -= 1
		
		print('dist calcs', ndistcalcs)
		return bestnode.p

		
	def nodes_at_level(self, level):
		if level > self.root.level:
			return [self.root]

		n = [self.root]
		stoplevel = max(self.lastbranchlevel, level)
		for i in range(self.root.level, stoplevel+1):
			n = children(n, i)
		
		return n
	
	def check_invariants(self):
		for i in range(self.root.level, self.lastbranchlevel - 1, -1):
			nodes = self.nodes_at_level(i)
			for n in nodes:
				for c in n.children(i):
					assert(dist(n.p, c.p) <= 2 ** i)
			for n1, n2 in itr.combinations(nodes, 2):
				assert(dist(n1.p, n2.p) > 2 ** i)
				

def random_data(n, m):
	return [ tuple(random.uniform(0, 1) for j in range(m)) for i in range(n) ]

def brute(p, data):
	i = min( (dist(p, q), i) for i, q in enumerate(data) )[1]
	return data[i]

NQUERIES = 5
NDIM = 2

if __name__ == '__main__':
	seed = random.randint(0, 100000)
	print('seed:', seed)
	random.seed(1)
	data = random_data(10000, NDIM)
	t = time.time()
	tree = CoverTree(data[0])
	for p in data[1:]:
		tree.insert(p)
	print('construction:', time.time() - t)
#	tree.check_invariants()
#	
#	treetotal = 0.0
#	brutetotal = 0.0
#	for n in range(NQUERIES):	
#		q = tuple(random.uniform(0, 1) for i in range(NDIM))
#		print('query:', q)
#		t = time.time()
#		tnn = tree.nn(q)
#		treetotal += time.time() - t
#		
#		t = time.time()
#		bnn = brute(q, data)
#		brutetotal += time.time() - t
#		
#		if tnn != bnn:
#			print('MISMATCH')
#			print('tree:', tnn)
#			print('brute:', bnn)
#			sys.exit(1)
#	
#	print('tree:', treetotal / NQUERIES)
#	print('brute:', brutetotal / NQUERIES)
#	