class ConstrainedModel:
	@staticmethod
	def __bin2dec(binary):
		result = 0
		for bit in binary:
			result <<= 1
			if bit: result += 1
		return result

	# returns true if the pos bit of the binary representation of num is set
	@staticmethod
	def __testbinpos(num, pos): return num & (1 << pos) > 0

	# set the pos bit of the binary representation of num
	@staticmethod
	def __setbinpos(num, pos): return num | (1 << pos)

	@staticmethod
	def __numbitssetbefore(num, pos):
		count = 0
		for p in range(pos):
			if ConstrainedModel.__testbinpos(num, p): count += 1
		return count

	@staticmethod
	def __numbitsset(num):
		count = 0
		i = 0
		while 2 ** i <= num:
			if ConstrainedModel.__testbinpos(num, i): count += 1
			i += 1
		return count

	@staticmethod
	def __allsetbits(num):
		pos = []
		i = 0
		while 2 ** i <= num:
			if ConstrainedModel.__testbinpos(num, i): pos.append(i)
			i += 1
		return pos

	@staticmethod
	def __cross_product(list1, list2):
		return reduce(lambda x,y: x+y, ([e1 + e2 for e2 in list2] for e1 in list1))

	def __init__(self, rule):
		# map from (body_index, body_index) to a list [(pos, pos, comp)]
		self.__constraints = rule.get_constraints_by_body_index()
		# __models[i] is the model ith body condition, in list form
		self.__arity = len(rule.get_body())

		self.__models = []
		for i in range(self.__arity): self.__models.append([])

		self.__partials = []
		for i in range(2 ** self.__arity): self.__partials.append([])

		self.__pair_cache = {}

	def get_cache_size(self): return sum([len(c) for c in self.__partials])

	def add_constraint(self, bi1, bi2, pos1, pos2, comp):
		self.__constraints.setdefault((bi1, bi2), []).append((pos1, pos2, comp))

	# check that the ground g1 that models the bi1th body condition
	# and the ground g2 that models the bi2th body condition fulfill
	# all constraints between the two body conditions
	def __compatible(self, bi1, gi1, bi2, gi2):
		cached = self.__pair_cache.get((bi1, gi1, bi2, gi2), None)
		if cached != None:
			return cached

		cached = self.__pair_cache.get((bi2, gi2, bi1, gi1), None)
		if cached != None:
			return cached

		# cache miss, calculate it the hard way
		g1 = self.__models[bi1][gi1]
		g2 = self.__models[bi2][gi2]
		if (bi1, bi2) in self.__constraints:
			constraints = self.__constraints[(bi1, bi2)]
			for pos1, pos2, comp in constraints:
				val1 = pos1.fetch(g1)
				val2 = pos2.fetch(g2)
				if not comp.check(val1, val2): 
					self.__pair_cache[(bi1, gi1, bi2, gi2)] = False
					return False
		# have to check both orderings 
		elif (bi2, bi1) in self.__constraints:
			constraints = self.__constraints[(bi2, bi1)]
			for pos2, pos1, comp in constraints:
				val1 = pos1.fetch(g1)
				val2 = pos2.fetch(g2)
				if not comp.check(val2, val1):
					self.__pair_cache[(bi1, gi1, bi2, gi2)] = False
					return False

		self.__pair_cache[(bi1, gi1, bi2, gi2)] = True
		return True

	def __check_compat(self, gi, bi, rest, comb_i):
		rest_bis = ConstrainedModel.__allsetbits(comb_i)
		for gi1, bi1 in zip(rest, rest_bis):
			if not self.__compatible(bi, gi, bi1, gi1): return False
		return True

	def add_ground(self, bi, g):
		self.__models[bi].append(g)
		gi = len(self.__models[bi]) - 1

		# just insert this ground into the comb_i_set position
		self.__partials[ConstrainedModel.__setbinpos(0, bi)].append((gi,))
		for comb_i in range(1,2 ** self.__arity):
			if not ConstrainedModel.__testbinpos(comb_i, bi):
				comb_i_set = ConstrainedModel.__setbinpos(comb_i, bi)
				# combine with rest
				for rest in self.__partials[comb_i]:
					if self.__check_compat(gi, bi, rest, comb_i):
						# move this combination into the slot where bi is set
						ins_pos = ConstrainedModel.__numbitssetbefore(comb_i, bi)
						new_partial = rest[0:ins_pos] + (gi,) + rest[ins_pos:]
						assert len(new_partial) == ConstrainedModel.__numbitsset(comb_i) + 1
						self.__partials[comb_i_set].append(new_partial)

	def get_valid_combinations(self, bi_pos):
		valids = []
		for comb in self.__partials[(2 ** self.__arity) - 1]:
			val_comb = []
			for bi, pos in bi_pos:
				val_comb.append(pos.fetch(self.__models[bi][comb[bi]]))
			valids.append(val_comb)
		return valids
