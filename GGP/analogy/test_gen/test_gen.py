import random
import pdb

from rule import Rule

class State:
	def __init__(self, preds, children):
		self.preds = frozenset(preds)
		self.__children = children
	
	def get_child(self, action):
		return self.__children[action]

	def make_max_rules(self, rules):
		for a, c in self.__children.items():
			rules.append(Rule(self.preds, [], a, c.preds))
			c.make_max_rules(rules)

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
				gen_sets.setdefault(r.get_action(), set()).update(r.get_rhs())
				rules_fired.setdefault(r.get_action(), []).append(r)

		for a in self.__children:
			if a not in gen_sets: 
				pdb.set_trace()
				return False
			if gen_sets[a] != self.__children[a].preds: 
				pdb.set_trace()
				return False

		return True

	def get_graphviz(self):
		s = ''
		for a, c in self.__children.items():
			r_label = ''.join(self.preds)
			if len(c.preds) == 0:
				c_label = 'goal'
			else:
				c_label = ''.join(c.preds)
			s += '%s -> %s [label="%s"];\n' % (r_label, c_label, a)

			if len(c.preds) > 0:
				s += c.get_graphviz()
		return s

	def get_all_states(self, states):
		states.append(self)
		for a, c in self.__children.items():
			c.get_all_states(states)
	
	def map(self, func):
		"""Like the Lisp map function"""

		func(self)
		for a in self.__children:
			map(self.__children[a], func)
	
	def __str__(self):
		l = list(self.preds)
		l.sort()
		return ''.join(l)

class Rules2FS:
	""" maintains a cache of which states rules fire in """
	def __init__(self, states):
		self.__states = states
		self.__r2fs = {}
	
	def __getitem__(self, rule):
		if rule in self.__r2fs:
			return self.__r2fs[rule]
		else:
			fs = []
			for s in self.__states:
				if rule.fires_in(s):
					fs.append(s)
			self.__r2fs[rule] = fs
			return fs

class TreeGen:
	def __init__(self):
		self.min_branch_len = 6
		self.max_branch_len = 10
		self.preserve_prob = 0.3
		self.min_pred_change = 1
		self.max_preds = 8
		self.predicates = [chr(i) for i in range(ord('A'), ord('M'))]
		self.actions = ['l', 'r']

		self.__states = set()

	def generate(self):
		initial_state = [random.choice(self.predicates) for i in range(self.max_preds)]
		self.__states = set()
		return self.__generate_rec(initial_state, 0)

	def __generate_rec(self, curr, branch_len):
		if random.random() < max(0, branch_len - self.min_branch_len) / float(self.max_branch_len - self.min_branch_len):
			return State(frozenset(), {})

		children = {}
		for a in self.actions:
			preserved = []
			# randomly preserve some of the state variables
			for p in curr:
				if random.random() < self.preserve_prob:
					preserved.append(p)

			while len(preserved) > len(curr) - self.min_pred_change:
				preserved.pop(random.randint(0, len(preserved)-1))

			# keep looping until we get a unique state
			while True:
				new_preds = [random.choice(self.predicates) for i in range(self.max_preds - len(preserved))]
				next_state = frozenset(preserved + new_preds)
				if next_state not in self.__states:
					self.__states.add(next_state)
					break

			children[a] = self.__generate_rec(next_state, branch_len + 1)
		
		return State(curr, children)

def match_rules_to_states(rules, states):
	for s in states:
		if not s.implemented_by(rules):
			return False
	return True

def make_kif(rules, initial_state):
	s = """
(<= terminal (true goal_achieved))
(<= (goal player 100) (true goal_achieved))
(role player)
(legal player l)
(legal player r)
"""
	
	for init in initial_state:
		s += "(init %s)\n" % init

	for r in rules:
		s += r.get_kif()
	
	return s

def extract_commons(rules, states, rules2fstates):
	commons = []
	non_goal_rules = [r for r in rules if len(r.get_rhs()) > 0]
	for i, r1 in enumerate(non_goal_rules):
		for r2 in non_goal_rules[i+1:]:
			if r1.get_action() != r2.get_action():
				continue

			orig_fstates = rules2fstates[r1] + rules2fstates[r2]
			pcs_int, ncs_int = r1.lhs_intersect(r2)
			rhs_int = r1.rhs_intersect(r2)
			if len(pcs_int) > 0 and len(rhs_int) > 0:
				cr = Rule(pcs_int, ncs_int, r1.get_action(), rhs_int, "common rule extracted from %s and %s" % (str(r1), str(r2)))
				consistent = True
				for s in states:
					if not s.consistent_with(cr):
						# first try to change lhs
						if not cr.restrict_firing(orig_fstates, [s]):
							# since that didn't work, try removing some rhs 
							# predicates
							cr.remove_rhs(s.preds)
							if len(cr.get_rhs()) == 0:
								consistent = False
								break

				if consistent:
					r1.remove_rhs(cr.get_rhs())
					r2.remove_rhs(cr.get_rhs())
					commons.append(cr)

	rules.extend(commons)

def cross_product(list1, list2):
	return reduce(lambda x,y: x+y, ([e1 + e2 for e2 in list2] for e1 in list1))

def split_rule(rule, states, rules2fstates):
	""" Try to split a rule into two rules while maintaining semantics """
	assert len(rule.get_rhs()) > 1, "Resulting rules must have some actions"

	pcs = list(rule.get_pconds())
	ncs = list(rule.get_nconds())
	rhs = list(rule.get_rhs())
	pcs_splits = [(i,) for i in range(len(pcs))] ; random.shuffle(pcs_splits)
	ncs_splits = [(i,) for i in range(len(ncs))] ; random.shuffle(ncs_splits)
	rhs_splits = [(i,) for i in range(len(rhs))] ; random.shuffle(rhs_splits)

	all_split_combs = cross_product(cross_product(pcs_splits, ncs_splits), rhs_splits)
	for pcs_split, ncs_split, rhs_split in all_split_combs:
		if rule.has_pconds():
			pcs1 = pcs[0:pcs_split]
			pcs2 = pcs[pcs_split:]
		else:
			pcs1 = []
			pcs2 = []

		if rule.has_nconds():
			ncs1 = ncs[0:ncs_split]
			ncs2 = ncs[ncs_split:]
		else:
			ncs1 = []
			ncs2 = []

		rhs1 = rhs[0:rhs_split]
		rhs2 = rhs[rhs_split:]

		a = rule.get_action()
		
		r1 = Rule(pcs1, ncs1, a, rhs1, 'split from %s' % rule)
		r2 = Rule(pcs2, ncs2, a, rhs2, 'split from %s' % rule)

		# make sure split rules aren't over-general
		firing_states = rules2fstates[rule]
		other_states = states.difference(firing_states)
		if r1.restrict_firing(firing_states, other_states) and \
				r2.restrict_firing(firing_states, other_states):
			return (r1, r2)

	return None

if __name__ == '__main__':
	random.seed(0)

	tree_gen = TreeGen()
	root = tree_gen.generate()
	all_states = []
	root.get_all_states(all_states)

	graph = open('graph.gdl', 'w')
	graph.write("digraph g {\n");
	graph.write(root.get_graphviz())
	graph.write("}\n")
	graph.close()

	rules = []
	root.make_max_rules(rules)
	rules2fstates = Rules2FS(all_states)

	extract_commons(rules, all_states, rules2fstates)

	kif = make_kif(rules, root.preds)

	rule_file = open('rules.kif', 'w')
	rule_file.write(kif)
	rule_file.close()

	for r in rules:
		print r
	
	print match_rules_to_states(rules, all_states)
