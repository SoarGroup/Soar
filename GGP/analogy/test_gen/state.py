from rule import Rule
import pdb

class State:
	def __init__(self, preds, children = {}):
		self.preds = frozenset(preds)
		self.__children = children
	
	def __contains__(self, i):
		return i in self.preds

	def __iter__(self):
		return iter(self.preds)
	
	def deep_copy(self):
		cpreds = self.preds.copy()
		cchildren = {}
		for a, c in self.__children.items():
			cchildren[a] = c.deep_copy()
		return State(cpreds, cchildren)

	def get_actions(self):
		return self.__children.keys()

	def get_child(self, action):
		return self.__children[action]
	
	def get_children(self):
		return self.__children.values()

	def set_child(self, action, child):
		self.__children[action] = child

	def make_max_rules(self, all_preds):
		rules = []
		ncs = all_preds - self.preds
		for a, c in self.__children.items():
			comment = 'Originally generated from state %s -%s-> %s' % (''.join(self.preds), a, ''.join(c.preds))
			rules.append(Rule(self.preds, ncs, a, c.preds, comment))
		return rules
		
	def make_max_rules_rec(self, rules, all_preds):
		rules.extend(self.make_max_rule(all_preds))
		for c in self.__children.values():
			c.make_max_rules_rec(rules, all_preds)
	
	def is_goal(self):
		return len(self.__children) == 0

	def next_is_goal(self, action):
		return self.__children[action].is_goal()
	
	def satisfies(self, pconds, nconds):
		return pconds <= self.preds and not self.preds & nconds

	def consistent_with(self, r):
		"""Returns True if a rule either doesn't fire in the state or fires
		consistently with the children of r"""
		return (r.doesnt_fire_in(self)) or (r.get_rhs() <= self.__children[r.get_action()].preds)
	
	def implemented_by(self, rules):
		"""Returns True if a set of rules exactly generates this state's children"""

		gen_sets = {}
		rules_fired = {}
		for r in rules:
			if r.fires_in(self):
				if self.next_is_goal(r.get_action()):
					if not r.is_goal_rule():
						pdb.set_trace()
						return False
				else:
					if r.is_goal_rule():
						pdb.set_trace()
						return False
					gen_sets.setdefault(r.get_action(), set()).update(r.get_rhs())
					rules_fired.setdefault(r.get_action(), []).append(r)

		for a in self.__children:
			if not self.next_is_goal(a):
				if a not in gen_sets: 
					pdb.set_trace()
					return False
				if gen_sets[a] != self.__children[a].preds: 
					pdb.set_trace()
					return False

		return True

	def make_graphviz(self, file, is_root=True):
		my_label = str(self)
		if is_root:
			file.write('digraph g {\n')
		for a, c in self.__children.items():
			file.write('%s -> %s [label="%s"];\n' % (my_label, str(c), a))
			if len(c.preds) > 0:
				c.make_graphviz(file, False)
		if is_root:
			file.write('}\n')

	def get_all_states(self):
		states = [self]
		for a, c in self.__children.items():
			states.extend(c.get_all_states())
		return states
	
	def get_all_predicates(self):
		preds = set(self.preds)
		for c in self.__children.values():
			preds.update(c.get_all_predicates())
		return preds
	
	def num_occurrences(self, p):
		"Returns the number of states in the subtree that p appears in"
		n = int(p in self.preds)
		for c in self.__children.values():
			n += c.num_occurrences(p)
		return n

	def map(self, func):
		"""Like the Lisp map function"""
		func(self)
		for a in self.__children:
			self.__children[a].map(func)

	def dist_to_goal(self, table):
		if self.is_goal():
			return 0
		min_dist = float('inf')
		for a, c in self.__children.items():
			min_dist = min(min_dist, c.dist_to_goal(table)+1)
		table[self] = min_dist
		return min_dist
	
	def __str__(self):
		if self.is_goal():
			return 'goal'
		l = list(self.preds)
		l.sort()
		return '_'.join(l)
	
	def __hash__(self):
		return id(self)

	def __eq__(self, other):
		return self.preds == other.preds
