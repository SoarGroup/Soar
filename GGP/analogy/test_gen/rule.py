from multimap import MultiMap
from ggp_utils import cross_join
import random
import pdb

class Rule:
	def __init__(self, pos_conds, neg_conds, action, rhs, comment = ""):
		self.__pcs = frozenset(pos_conds)
		self.__ncs = frozenset(neg_conds)
		self.__action = action
		self.__rhs = frozenset(rhs)
		self.comment = comment
	
	def get_action(self):
		return self.__action

	def fires_in(self, s):
		# would be
		# return s.satisfies(self.__pcs, self.__ncs)
		# but that is a little slower
		# this function gets called A LOT
		return self.__pcs <= s.preds and not self.__ncs & s.preds

	def doesnt_fire_in(self, s):
		return (not self.__pcs <= s.preds) or self.__ncs & s.preds

	def get_pconds(self):
		return self.__pcs
	
	def has_pconds(self):
		return len(self.__pcs) > 0
	
	def get_nconds(self):
		return self.__ncs

	def has_nconds(self):
		return len(self.__ncs) > 0

	def get_rhs(self):
		assert len(self.__rhs) > 0, "Don't try to get the rhs of a goal rule"
		return self.__rhs

	def is_goal_rule(self):
		return len(self.__rhs) == 0

	def get_kif(self):
		s = ""
		pos = ''.join('(true %s) ' % p for p in self.__pcs)
		neg = ''.join('(not (true %s)) ' % p for p in self.__ncs)
		body = ('(does player %s) ' % self.__action) + pos + neg
		if len(self.__rhs) == 0:
			s += '(<= (next goal_achieved) %s)\n' % body
		else:
			for h in self.__rhs:
				s += '(<= (next %s) %s)\n' % (h, body)
		
		return s

	def has_common_rhs(self, other):
		return len(self.__rhs.intersection(other.__rhs)) > 0

	def lhs_intersect(self, other):
		""" Combine the conditions in this and the other rule so that for all nodes n,
		self.fires_in(n) -> result.fires_in(n) and other.fires_in(n) -> result.fires_in(n)"""
		pos = self.__pcs.intersection(other.__pcs)
		neg = self.__ncs.intersection(other.__ncs)
		return (pos, neg)

	def rhs_intersect(self, other):
		return self.__rhs.intersection(other.__rhs)

	def set_rhs_intersect(self, s):
		""" Make the rhs the intersection of the original rhs and a set """
		self.__rhs = self.__rhs.intersection(s)

	def add_pconds(self, s):
		self.__pcs = self.__pcs.union(s)
	
	def add_nconds(self, s):
		self.__ncs = self.__ncs.union(s)

	def remove_pconds(self, s):
		self.__pcs = self.__pcs.difference(s)
	
	def remove_nconds(self, s):
		self.__ncs = self.__ncs.difference(s)

	def add_rhs(self, s):
		self.__rhs = self.__rhs.difference(s)
	
	def remove_rhs(self, s):
		self.__rhs = self.__rhs.difference(s)

	def restrict_firing(self, firing_states, other_states):
		""" restrict this rule to fire only in a particular set of states """
		# union of all predicates in the firing states
		all_fs_preds = set()
		for s in firing_states:
			all_fs_preds.update(s.preds)

		# intersection of all predicates in the firing states
		all_fs_int = set(iter(firing_states).next().preds)
		for s in firing_states:
			all_fs_int.intersection_update(s.preds)

		old_pcs = self.__pcs.copy()
		old_ncs = self.__ncs.copy()
		for s in other_states:
			if self.fires_in(s):
				# make self not fire in this state
				# find some predicate that none of the firing states have,
				# make it a negative condition in self
				in_s = s.preds.difference(all_fs_preds)
				if len(in_s) == 0:
					# can't add a negative condition, try adding a positive
					# condition that all the firings states have, but this
					# state doesn't
					not_in_s = all_fs_int.difference(s.preds)
					if len(not_in_s) == 0:
						# there's no way, give up
						self.__pcs = old_pcs
						self.__ncs = old_ncs
						return False
					else:
						self.add_pconds(set([not_in_s.pop()]))
				else:
					self.add_nconds(set([iter(in_s).next()]))
		return True
	
	def covers(self, other):
		"""A rule r covers another rule r' if r generates all the rhs
		predicates r' does and will fire in all states r' fires in"""
		
		return self.__action == other.__action and \
		       self.__pcs <= other.__pcs and \
		       self.__ncs <= other.__ncs and \
		       self.__rhs >= other.__rhs

	def __str__(self):
		pcs = list(self.__pcs)
		pcs.sort()
		pos = ''.join(pcs)
		ncs = list(self.__ncs)
		ncs.sort()
		neg = ''.join(ncs)
		body = '%s/%s' % (pos, neg)
		if len(self.__rhs) == 0:
			return '%s -%s-> goal' % (body, self.__action)
		else:
			rhs = list(self.__rhs)
			rhs.sort()
			return '%s -%s-> %s' % (body, self.__action, ''.join(rhs))
	
	def __eq__(self, other):
		return self.__pcs == other.__pcs and \
				self.__ncs == other.__ncs and \
				self.__rhs == other.__rhs and \
				self.__action == other.__action
	
	def __hash__(self):
		return hash((self.__pcs, self.__ncs, self.__action, self.__rhs))

class RuleSet:
	def __init__(self, rules, states, all_preds):
		self.rules = rules
		self.states = states
		self.__r2fs = {}
		# mapping from predicates to the states that contain them
		# pred -> [state]
		self.__p2s = MultiMap()
		for s in self.states:
			for p in s:
				self.__p2s[p] = s

		# mapping from predicates to states that do not contain them
		# pred -> [state]
		self.__p2ns = MultiMap()
		for s in self.states:
			for p in  all_preds - s.preds:
				self.__p2ns[p] = s
	
	def check_consistency(self):
		for s in self.states:
			if not s.implemented_by(self.rules):
				return False
		return True
	
	def make_consistent(self, rule):
		"Modify a rule to make it only fire in the states that it's consistent with"
		firing_states = self.all_firing_states(rule)
		consistent = []
		inconsistent = []
		for s in firing_states:
			if s.consistent_with(rule):
				consistent.append(s)
			else:
				inconsistent.append(s)
		return rule.restrict_firing(consistent, inconsistent)

	def extract_common(self, r1, r2, modify_origs):
		if r1.is_goal_rule() or r2.is_goal_rule():
			return None
		if r1.get_action() != r2.get_action():
			return None

		orig_fstates = self.all_firing_states(r1) | self.all_firing_states(r2)
		pcs_int, ncs_int = r1.lhs_intersect(r2)
		rhs_int = r1.rhs_intersect(r2)
		if len(pcs_int) > 0 and len(rhs_int) > 0:
			cr = Rule(pcs_int, ncs_int, r1.get_action(), rhs_int, "common rule extracted from %s and %s" % (str(r1), str(r2)))
			consistent = True
			for s in self.states:
				if not s.consistent_with(cr):
					# first try to change lhs
					if not cr.restrict_firing(orig_fstates, [s]):
						# since that didn't work, try removing some rhs 
						# predicates
						cr.set_rhs_intersect(s.get_child(cr.get_action()).preds)
						if len(cr.get_rhs()) == 0:
							return None

			# common rule is acceptable
			if modify_origs:
				for x in [r1, r2]:
					r1.remove_rhs(cr.get_rhs())
			return cr
		
		return None

	def extract_all_commons(self):
		commons = []
		to_remove = set()
		non_goal_rules = filter(lambda r: not r.is_goal_rule(), self.rules)
		i = 0
		while i < len(non_goal_rules):
			r1 = non_goal_rules[i]
			r1_del = False
			j = i + 1
			while j < len(non_goal_rules):
				r2 = non_goal_rules[j]
				r2_del = False
				cr = self.extract_common(r1, r2, True)
				if cr != None:
					commons.append(cr)
					if r1.is_goal_rule():
						self.rules.remove(r1)
						del non_goal_rules[i]
						r1_del = True
					if r2.is_goal_rule():
						self.rules.remove(r2)
						del non_goal_rules[j]
						r2_del = True
				if r1_del:
					# don't loop on r1 anymore
					break
				elif not r2_del:
					j += 1

			if not r1_del:
				i += 1
		
		for r in to_remove:
			self.rules.remove(r)

		self.rules.extend(commons)

	def split_rule(self, rule, partition=None):
		"""Try to split a rule into two rules while maintaining semantics.
		partition specifies the conditions to split on"""

		assert len(rule.get_rhs()) > 1, "Resulting rules must have some actions"

		if partition is None:
			# do random splitting
			pcs = list(rule.get_pconds())
			pcs_splits = [(i,) for i in range(len(pcs))] ; random.shuffle(pcs_splits)
		else:
			# split based on a pre-determined partitioning
			# the rule should only have predicates from one side of the partition
			split = rule.get_pconds() & partition
			others = rule.get_pconds() - split
			# order pcs like the partitioning
			pcs = list(split) + list(others)
			pcs_splits = [(len(split),)]

		ncs = list(rule.get_nconds())
		rhs = list(rule.get_rhs())
		ncs_splits = [(i,) for i in range(len(ncs))] ; random.shuffle(ncs_splits)
		# both splits must have at least one action
		rhs_splits = [(i,) for i in range(1,len(rhs)-1)] ; random.shuffle(rhs_splits)

		all_split_combs = cross_join(cross_join(pcs_splits, ncs_splits), rhs_splits)
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
			#firing_states = self.all_firing_states(rule)
			#other_states = self.states - firing_states
			#if r1.restrict_firing(firing_states, other_states) and \
			#		r2.restrict_firing(firing_states, other_states):
			#	return (r1, r2)
			if self.make_consistent(r1) and self.make_consistent(r2):
				return (r1, r2)

		return None
	
		
	def all_firing_states(self, rule):
		"""Return all states a rule fires in"""
		
		# might need a more efficient algorithm in the future
		if rule in self.__r2fs:
			return self.__r2fs[rule]
		else:
			result = set()
			for p in rule.get_pconds():
				for s in self.__p2s[p]:
					if rule.fires_in(s):
						result.add(s)
			self.__r2fs[rule] = result
			return result

	def remove_pconds(self, rule, preserved_preds):
		"""Remove as many positive conditions as possible while maintaining semantics"""

		orig_fs = self.all_firing_states(rule)
		# removing positive conditions will only increase the set of firing states,
		new_pconds = set(rule.get_pconds())
		safe_to_del = set()
		for p in rule.get_pconds():
			if p in preserved_preds:
				continue

			preserves = True
			new_pconds.remove(p)
			for s in self.__p2ns[p]:
				if s not in orig_fs and s.satisfies(new_pconds, rule.get_nconds()):
					# removing this condition results in the rule firing in a state
					# where it didn't before
					preserves = False
					break
			if not preserves:
				# deletion doesn't preserve semantics, put condition back
				new_pconds.add(p)
			else:
				safe_to_del.add(p)

		if len(safe_to_del) > 0:
			rule.remove_pconds(safe_to_del)
			temp = list(safe_to_del)
			temp.sort()
			rule.comment += "\nRemoved these positive conditions: %s" % ''.join(temp)
	
	def remove_nconds(self, rule):
		"""Remove as many negative conditions as possible while maintaining semantics"""

		orig_fs = self.all_firing_states(rule)
		new_nconds = set(rule.get_nconds())
		safe_to_del = set()
		for p in rule.get_nconds():
			preserves = True
			new_nconds.remove(p)
			for s in self.__p2s[p]:
				if s not in orig_fs and s.satisfies(rule.get_pconds(), new_nconds):
					# removing this condition results in the rule firing in a state
					# where it didn't before
					preserves = False
					break
			if not preserves:
				# deletion doesn't preserve semantics, put condition back
				new_nconds.add(p)
			else:
				safe_to_del.add(p)

		if len(safe_to_del) > 0:
			rule.remove_nconds(safe_to_del)
			temp = list(safe_to_del)
			temp.sort()
			rule.comment += "\nRemoved these negative conditions: %s" % ''.join(temp)
	
	def minimize_nconds(self):
		for r in self.rules:
			self.remove_nconds(r)

	def minimize_pconds(self, preserve):
		for r in self.rules:
			self.remove_pconds(r, preserve)

	def minimize_rule_conds(self, preserve):
		for r in self.rules:
			self.remove_nconds(r)
			self.remove_pconds(r, preserve)

	def minimize(self):
		"""Make the rule set as small as possible by removing all rules that
		are covered by other rules"""

		for r1 in self.rules[:]:
			for r2 in self.rules[:]:
				if r1.covers(r2):
					try:
						self.rules.remove(r2)
					except ValueError: pass

	def write_rules(self, file):
		for r in self.rules:
			file.write('%s\n' % str(r))

	def make_kif(self, initial_state, file):
		file.write("""
(<= terminal (true goal_achieved))
(<= (goal player 100) (true goal_achieved))
(role player)
(legal player m1)
(legal player m2)
""")
		
		for init in initial_state:
			file.write("(init %s)\n" % init)

		for r in self.rules:
			file.write(r.get_kif())

	def make_rule_graph(self, file):
		""" Make a GDL representation of this rule set """
		predicates = set()
		nodes = set()
		nodes2rules = {} # node -> [rules that fire in that node]
		preds2nodes = {} # predicate -> [nodes that contain it]
		for r in self.rules:
			pcs = r.get_pconds()
			supernode = None
			to_erase = []
			for n in nodes:
				if pcs <= n:
					supernode = n
					break
				elif n <= pcs:
					supernode = pcs
					to_erase.append(n)
					nodes2rules.setdefault(pcs, []).extend(nodes2rules[n])
					del nodes2rules[n]
					for p in n:
						preds2nodes[p].remove(n)

			for n in to_erase:
				nodes.remove(n)

			if supernode == None:
				nodes.add(pcs)
				supernode = pcs
				for p in pcs:
					preds2nodes.setdefault(p, []).append(pcs)
			
			nodes2rules.setdefault(supernode,[]).append(r)

		edges = set()
		for n in nodes2rules:
			assert n in nodes
			firing = nodes2rules[n]
			for fr in firing:
				if fr.is_goal_rule():
					edges.add((n, 'goal'))
				else:
					#conn_nodes = reduce(lambda x, y: x + y, (preds2nodes[p] for p in fr.get_rhs()))
					#for n2 in conn_nodes:
					#	assert n2 in nodes
					#	edges.add((n, n2))
					
					# instead of making an edge to every node that contains any rhs predicate,
					# only make edges to the nodes that are supersets of all rhs predicates
					for n2 in nodes:
						if n2 != n and fr.get_rhs() <= n2:
							edges.add((n, n2))
		
		file.write('digraph g {\n')
		for e in edges:
			n1 = list(e[0])
			n1.sort()
			n1 = ''.join(n1)
			if e[1] != 'goal':
				n2 = list(e[1])
				n2.sort()
				n2 = ''.join(n2)
			else:
				n2 = 'goal'
			file.write('%s -> %s\n' % (n1, n2))
		file.write('}')
		file.close()
