import random
from state import State

def cross_product(list1, list2):
	return reduce(lambda x,y: x+y, ([e1 + e2 for e2 in list2] for e1 in list1))

class TreeGen:
	def __init__(self):
		self.min_branch_len = 4
		self.max_branch_len = 5
		self.preserve_prob = 0.5
		self.min_pred_change = 1
		self.max_preds = 5
		alpha = [chr(i) for i in range(ord('A'), ord('Z')+1)]
		self.predicates = alpha
		#self.predicates = cross_product(alpha, alpha)
		#self.predicates.remove('OR')
		self.actions = ['m1', 'm2']

		self.__states = set()

	def generate(self):
		initial_state = frozenset([random.choice(self.predicates) for i in range(self.max_preds)])
		self.__states = set()
		return self.__generate_rec(initial_state, 0)

	def __generate_rec(self, curr, branch_len):
		self.__states.add(curr)
		if self.max_branch_len == self.min_branch_len:
			if branch_len == self.max_branch_len:
				return State(frozenset(), {})
		elif random.random() < max(0, branch_len - self.min_branch_len) / float(self.max_branch_len - self.min_branch_len):
			return State(frozenset(), {})

		children = {}
		for a in self.actions:
			#pdb.set_trace()
			preserved = []
			# randomly preserve some of the state variables
			for p in curr:
				if random.random() < self.preserve_prob:
					preserved.append(p)

			while self.max_preds - len(preserved) < self.min_pred_change:
				preserved.pop(random.randint(0, len(preserved)-1))

			# keep looping until we get a unique state
			while True:
				new_preds = [random.choice(self.predicates) for i in range(self.max_preds - len(preserved))]
				next_state = frozenset(preserved + new_preds)
				if next_state not in self.__states and len(next_state - curr) >= self.min_pred_change:
					break

			children[a] = self.__generate_rec(next_state, branch_len + 1)
		
		return State(curr, children)

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
