import random
from state import State
from ggp_utils import cross_join
import pdb


class TreeGen:
	def __init__(self):
		self.min_branch_len = 4
		self.max_branch_len = 8
		self.preserve_prob = 0.5
		self.min_pred_change = 1
		self.min_preds = 2
		self.max_preds = 8
		alpha = [chr(i) for i in range(ord('A'), ord('Z')+1)]
		self.predicates = alpha
		self.predicates = cross_join(cross_join(alpha, alpha), alpha)
		#self.predicates.remove('OR')
		self.actions = ['m1', 'm2']

		self.__states = set()

	def __generate_empty(self, branch_len=0):
		if self.max_branch_len == self.min_branch_len:
			if branch_len == self.max_branch_len:
				return State(frozenset())
		elif random.random() < max(0, branch_len - self.min_branch_len) / float(self.max_branch_len - self.min_branch_len):
			return State(frozenset())

		children = {}
		for a in self.actions:
			children[a] = self.__generate_empty(branch_len + 1)
		
		return State(frozenset(), children)

	def __add_random_preds(self, existing):
		if len(self.__used_preds) == 0:
			used_preds_pool = []
		else:
			max_num = max(len(x) for x in self.__used_preds.values())
			# this creates an array of used predicates where each used predicate
			# appears n times, n being the most number of states any predicate
			# has been in minus the number of states each predicate is already
			# part of. So the probability of choosing a predicate that has
			# appeared a lot is lower
			used_preds_pool = reduce(lambda x,y: x+y, \
			  ([p] * (max_num - len(d) + 1) for p, d in self.__used_preds.items()))

		new_pred_prob = 0
		while True:
			# with more iterations, the probability of using a new predicate
			# becomes greater, since it probably makes combinations of used
			# predicates is already expended
			new_pred_prob += 0.05
			min_random = max(self.min_preds - len(existing), 0)
			max_random = max(min_random, self.max_preds - len(existing))
			num_random = random.randint(min_random, max_random)
			random_preds = set()
			while len(random_preds) < num_random:
				# choose between either using a used predicate or a new predicate
				if random.random() < new_pred_prob or len(used_preds_pool) == 0:
					random_preds.add(random.choice(self.predicates))
				else:
					random_preds.add(random.choice(used_preds_pool))

			s = frozenset(set(existing) | random_preds)
			assert len(s) >= self.min_preds and len(s) <= self.max_preds
			if s not in self.__states:
				# this is the final set, update self.__used_preds
				for p in random_preds:
					self.__used_preds.setdefault(p,[]).append(s)
				return s

	def generate_random(self):
		root = self.__generate_empty()
		self.__populate_random(root, None)
		self.__generate_random_rec(root, 0)
		return root

	def __generate_random_rec(self, state, depth):
		for c in state.get_children():
			if not c.is_goal():
				self.__populate_random(c, state)
				self.__generate_random_rec(c, depth+1)
		
	def __populate_random(self, curr, parent):
		preserved = []
		# randomly preserve some of the state variables
		if parent is not None:
			for p in parent.preds:
				if random.random() < self.preserve_prob:
					preserved.append(p)

		while self.max_preds - len(preserved) < self.min_pred_change:
			preserved.pop(random.randint(0, len(preserved)-1))

		# keep looping until we get a unique state
		unused_preds = set(self.predicates) - set(preserved)
		while True:
			min_rand = max(0, self.min_preds - len(preserved))
			max_rand = max(min_rand, self.max_preds - len(preserved))
			num_rand = random.randint(min_rand, max_rand)
			new_preds = random.sample(unused_preds, num_rand)
			next_state = frozenset(preserved + new_preds)
			assert len(next_state) >= self.min_preds and len(next_state) <= self.max_preds
			if next_state not in self.__states and \
			  (parent is None or len(next_state - parent.preds) >= self.min_pred_change):
				self.__states.add(next_state)
				curr.preds = next_state
				return
	
	def generate_indicators(self, indicators):
		# we only want the specified predicates to be good indicators, and have to
		# make sure that the other predicates used are bad indicators (as much as
		# possible). __used_preds is a map from preds to depths at which they've
		# been used, and predicates used at fewer depths are chosen more frequently
		self.__used_preds = {}
		root = self.__generate_empty()
		dists = {}
		root.dist_to_goal(dists)
		self.__gen_indicators_rec(root, indicators, dists)
		return root

	def __gen_indicators_rec(self, state, ind, dists):
		to_goal = dists[state]
		self.__populate_indicators(state, ind.get(to_goal, []))
		for c in state.get_children():
			if not c.is_goal():
				self.__gen_indicators_rec(c, ind, dists)

	def __populate_indicators(self, curr, ind):
		# just add some random predicates to the indicators
		s = self.__add_random_preds(ind)
		self.__states.add(s)
		curr.preds = s
	
	def generate_contradictory_indicators(self, indicators):
		self.__used_preds = {}
		root = self.__generate_empty()
		dists = {}
		root.dist_to_goal(dists)
		# these are the 4 predicates we're going to keep shuffling
		# around to make the contradictions
		split_preds = random.sample(self.predicates, 4)
		# randomly impose an ordering the first time
		#partition = (split_preds[:2], split_preds[2:])
		# the alter egos of the predicate in the target will be constantly changed
		ind_preds = set()
		for i in indicators.values():
			ind_preds |= set(i)

		self.__used_split2 = ind_preds

		#self.__gen_contradict_rec(root, partition, indicators, dists)
		self.__gen_contradict_rec(root, split_preds, indicators, dists)
		return (root, split_preds, self.__used_split2)

	# split2 contains the predicates the target intends to split on. split2alt is the
	# same thing, but for the next level down. Keep alternating these two as the tree
	# deepens
	def __gen_contradict_rec(self, curr, split, indicators, dists):
		inds = indicators.get(dists[curr],[])
		split2 = random.sample(set(self.predicates) - self.__used_split2, 4)
		self.__used_split2 |= set(split2)
		self.__populate_contradict(curr, split, split2, inds)
		next_level_split = ((split[0][0], split[1][0]),(split[0][1],split[1][1]))
		for c in curr.get_children():
			if not c.is_goal():
				self.__gen_contradict_rec(c, next_level_split, indicators, dists)

	def __gen_contradict_rec2(self, curr, split, indicators, dists):
		inds = indicators.get(dists[curr],[])
		split2 = random.sample(set(self.predicates) - self.__used_split2, 4)
		self.__used_split2 |= set(split2)
		curr.preds = self.__add_random_preds(split + split2)
		for c in curr.get_children():
			if not c.is_goal():
				self.__gen_contradict_rec(c, split, indicators, dists)

	# split = [(A, C), (B, D)]
	# second_split = (W, X, Y, Z)
	#
	# Assume that the source rules of the previous level will be split on the
	# (A,B):(C,D) set, and the target will either be split on (w,x):(y,z). Make
	# this level have the split sets (A,C):(B,D) and (M,N):(O,P), and have the
	# source split on (A,C):(B,D) whereas the target split on (m,n):(o,p). We
	# will then have some variation of the following mappings:
	#
	# A -> m, C -> n / B -> o, D -> p
	#
	# which is mutually exclusive from
	# 
	# A -> w, B -> x / C -> y, D -> z
	#
	# under structural consistency constraint.
	def __populate_contradict(self, curr, split, second_split, indicators):
		split_preds = []
		if random.random() < 0.5:
			split_preds.extend(split[0])
			split_preds.extend(second_split[:2])
		else:
			split_preds.extend(split[1])
			split_preds.extend(second_split[2:])
		
		split_preds.extend(indicators)
		
		# now add some random predicates to make state unique
		s = self.__add_random_preds(split_preds)
		self.__states.add(s)
		curr.preds = s

	def generate_buttons(self):
		goal = State([])
		npnqnr = State(['np','nq','nr'])
		pnqnr = State(['p','nq','nr'])
		npqnr = State(['np','q','nr'])
		npnqr = State(['np','nq','r'])
		pqnr = State(['p','q','nr'])
		npqr = State(['np','q','r'])
		pnqr = State(['p','nq','r'])

		npnqnr.set_child('a', pnqnr)
		npnqnr.set_child('b', npnqnr)
		npnqnr.set_child('c', npnqnr)

		pnqnr.set_child('a', npnqnr)
		pnqnr.set_child('b', npqnr)
		pnqnr.set_child('c', pnqnr)
		
		npqnr.set_child('a', pqnr)
		npqnr.set_child('b', pnqnr)
		npqnr.set_child('c', npnqr)

		pqnr.set_child('a', npqnr)
		pqnr.set_child('b', npnqnr)
		pqnr.set_child('c', pnqr)

		npnqr.set_child('a', pnqr)
		npnqr.set_child('b', goal)
		npnqr.set_child('c', npqnr)

		pnqr.set_child('a', npnqr)
		pnqr.set_child('b', npqr)
		pnqr.set_child('c', pqnr)

		npqr.set_child('a', goal)
		npqr.set_child('b', pnqr)
		npqr.set_child('c', npnqnr)
		
		return npnqnr

if __name__ == '__main__':
	indicators = dict((d, ["I%d" % d]) for d in range(10))
	gen = TreeGen()
	root = gen.generate_contradictory_indicators(indicators)
	root.make_graphviz(open('contradict.dot','w'))
