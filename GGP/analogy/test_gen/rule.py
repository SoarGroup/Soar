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
		all_fs_int = set(firing_states[0].preds)
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
