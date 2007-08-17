from PositionIndex import PositionIndex

CompRelations = [ 'distinct', '<', '>', '<=' ]

class Sentence:
	
	def __init__(self, relation, terms):
		self.__rel = relation
		self.__terms = terms
		self.__negated = False

	def copy(self):
		c = Sentence(self.__rel, [t.copy() for t in self.__terms])
		if self.__negated:
			c.__negated = True
		return c

	def negate(self):
		self.__negated = True

	def mangle_vars(self, prefix):
		for t in self.__terms:
			t.mangle_vars(prefix)

	def get_relation(self): return self.__rel

	def get_terms(self) : return self.__terms

	def get_term(self, i): return self.__terms[i]

	def set_term(self, i, val): self.__terms[i] = val

	def get_var_terms(self):
		result = []
		for t in self.__terms:
			result.extend(t.get_var_terms())
		return result

	def has_term(self, term):
		for t in self.__terms:
			if t.has_term(term):
				return True
		return False

	def arity(self):
		return len(self.__terms)
	
	def get_predicate(self):
		if self.__rel in ['true', 'next', 'init']:
			return self.__terms[0].get_name()
		if self.__rel in ['legal', 'does']:
			return self.__terms[1].get_name()
		if self.__rel not in ['goal', 'terminal', 'role']:
			return self.__rel
		return None

	def is_complex(self): return True

	def is_comparison(self):
		return self.__rel in CompRelations

	def covers(self, other):
		if not isinstance(other, Sentence):
			return False

		if self.__rel != other.__rel: return False
		if len(self.__terms) != len(other.__terms): return False

		for i in range(len(self.__terms)):
			if not self.__terms[i].covers(other.__terms[i]):
				return False

		return True

	def __eq__(self, other):
		if self.__rel != other.__rel: return False
		for i in range(len(self.__terms)):
			if self.__terms[i] != other.__terms[i]:
				return False
		return True

	def __ne__(self, other):
		return not self == other

	def __str__(self):
		if len(self.__terms) == 0:
			return self.__rel

		term_str = ""
		for t in self.__terms:
			term_str += ' %s' % str(t)
		return "(%s%s)" % (self.__rel, term_str)

	def __hash__(self): return hash((self.__rel, tuple(self.__terms)))

class Function:
	def __init__(self, name, terms):
		self.__name = name
		self.__terms = terms

	def copy(self):
		return Function(self.__name, [t.copy() for t in self.__terms])

	def mangle_vars(self, prefix):
		for t in self.__terms:
			t.mangle_vars(prefix)

	def get_name(self): return self.__name

	def get_term(self, i): return self.__terms[i]

	def get_terms(self): return self.__terms

	def get_var_terms(self):
		result = []
		for t in self.__terms:
			result.extend(t.get_var_terms())
		return result

	def has_term(self, term):
		

	def set_term(self, i, val): self.__terms[i] = val

	def is_complex(self): return True

	def arity(self): return len(self.__terms)
	
	def covers(self, other):
		if not isinstance(other, Function):
			return False

		if self.__rel != other.__rel: return False
		if len(self.__terms) != len(other.__terms): return False

		for i in len(self.__terms):
			if not self.__terms[i].covers(other.__terms[i]):
				return False

		return True

	def __eq__(self, other):
		if not isinstance(other, Function): return False

		if self.__name != other.__name: return False

		for i in range(len(self.__terms)):
			if self.__terms[i] != other.__terms[i]:
				return False
		return True

	def __ne__(self, other):
		return not self == other

	def __str__(self):
		term_str = ""
		for t in self.__terms:
			term_str += ' %s' % str(t)
		return "(%s%s)" % (self.__name, term_str)

	def __hash__(self): return hash((self.__name, tuple(self.__terms)))

class Variable:
	def __init__(self, name):
		if name[0] == '?':
			self.__name = name[1:]
		else:
			self.__name = name
	
	def copy(self): return Variable(self.__name)

	def mangle_vars(self, prefix):
		self.__name = "%s%s" % (prefix, self.__name)

	def get_name(self): return self.__name

	def get_var_terms(self): return [ Variable(self.__name) ]


	def has_term(self, term):
		return term == self

	def is_complex(self): return False
	
	def covers(self, other):
		return isinstance(other, Variable) or isinstance(other, Constant)

	def __eq__(self, other):
		if not isinstance(other, Variable): return False
		return self.__name == other.__name

	def __ne__(self, other):
		return not self == other

	def __str__(self): return '?%s' % self.__name

	def __hash__(self): return hash(self.__name)

class Constant:
	def __init__(self, name):
		self.__name = name

	def copy(self): return Constant(self.__name)

	def mangle_vars(self, prefix):
		pass
	
	def get_name(self): return self.__name

	def get_var_terms(self): return []

	def has_term(self, term):
		return self == term

	def is_complex(self): return False

	def covers(self, other):
		return self == other

	def __eq__(self, other):
		if not isinstance(other, Constant): 
			return False
		return self.__name == other.__name
	
	def __ne__(self, other):
		return not self == other

	def __str__(self): return str(self.__name)

	def __hash__(self): return hash(self.__name)

class Comparison:
	def __init__(self, relation):
		self.__rel = relation
	
	def check(self, v1, v2):
		if self.__rel == 'distinct':
			return v1 != v2
		if self.__rel == '==':
			return v1 == v2
		if self.__rel == '<':
			return v1 < v2
		if self.__rel == '<=':
			return v1 <= v2
		if self.__rel == '>':
			return v1 > v2
		if self.__rel == '>=':
			return v1 >= v2
	
	def complement(self):
		if self.__rel == 'distinct':
			return Comparison('==')
		if self.__rel == '==':
			return Comparison('distinct')
		if self.__rel == '<':
			return Comparison('>=')
		if self.__rel == '<=':
			return Comparison('>')
		if self.__rel == '>':
			return Comparison('<=')
		if self.__rel == '>=':
			return Comparison('<')
	
	def relation(self):
		return self.__rel

class Rule:
	def __init__(self, head, body):
		self.__head = head
		self.__body = body
		
		# relations between variables like < <= = !=
		# also for relations between variables and constants
		# self.__var_rels = {}
	
		# a list  (body_index1,pos1,body_index2,pos2,comparison operator)
		self.__var_constraints = []

		# a mapping head_pos -> [(body_index, body_pos)]
		self.__headvar_bindings = {}

		# explicitly add all equality constraints between variables
		for bi1, b1 in enumerate(self.__body):
			positions1 = PositionIndex.get_all_positions(b1)
			for p1 in positions1:
				v1 = p1.fetch(b1)
				if isinstance(v1, Variable):
					# can't use enumerate here, have to preserve correct indexing
					# for bi2
					for bi2, b2 in zip(range(bi1, len(self.__body)), self.__body[bi1:]):
						positions2 = PositionIndex.get_all_positions(b2)
						for p2 in positions2:
							v2 = p2.fetch(b2)
							if isinstance(v2, Variable):
								if (p1 != p2 or bi1 != bi2) and v1 == v2:
									self.__var_constraints.append((bi1, p1, bi2, p2, Comparison('==')))

		hpositions = PositionIndex.get_all_positions(head)
		for hp in hpositions:
			hv = hp.fetch(head)
			if not isinstance(hv, Variable):
				continue
			bound = False
			for bi, b in enumerate(self.__body):
				bpositions = PositionIndex.get_all_positions(b)
				for bp in bpositions:
					bv = bp.fetch(b)
					if isinstance(bv, Variable):
						if (hv == bv):
							self.__headvar_bindings.setdefault(hp, []).append((bi, bp))
							bound = True

			assert bound, "Head variable %s not bound to any body variables" % hv
	
	def copy(self):
		c = Rule(self.__head.copy(), [b.copy() for b in self.__body])
		c.__var_constraints = self.__var_constraints[:]
		return c
		
	def get_head(self):
		return self.__head
	
	def get_body(self):
		return self.__body

	def get_cond(self, i):
		return self.__body[i]

	def num_conditions(self):
		return len(self.__body)

	def add_condition(self, cond):
		self.__body.append(cond)
		nbi = len(self.__body) - 1

		# add variable equality comparisons
		for bi1, b1 in enumerate(self.__body):
			positions1 = PositionIndex.get_all_positions(b1)
			for p1 in positions1:
				v1 = p1.fetch(b1)
				if isinstance(v1, Variable):
					positions2 = PositionIndex.get_all_positions(cond)
					for p2 in positions2:
						v2 = p2.fetch(cond)
						if isinstance(v2, Variable):
							if (p1 != p2 or bi1 != nbi) and v1 == v2:
								self.__var_constraints.append((bi1, p1, nbi, p2, Comparison('==')))

		# add head bindings
		hpositions = PositionIndex.get_all_positions(self.__head)
		for hp in hpositions:
			hv = hp.fetch(self.__head)
			if not isinstance(hv, Variable):
				continue
			bpositions = PositionIndex.get_all_positions(cond)
			for bp in bpositions:
				bv = bp.fetch(cond)
				if isinstance(bv, Variable):
					if (hv == bv):
						self.__headvar_bindings.setdefault(hp, []).append((nbi, bp))

	def remove_condition(self, bi):
		new_var_constraints = []
		self.__body = self.__body[:bi] + self.__body[bi+1:]
		for bi1, p1, bi2, p2, comp in self.__var_constraints:
			if bi1 == bi or bi2 == bi:
				# throw out this constraint
				continue
			if bi1 > bi:
				nbi1 = bi1 - 1
			else:
				nbi1 = bi1
			if bi2 > bi:
				nbi2 = bi2 - 1
			else:
				nbi2 = bi2
			new_var_constraints.append((nbi1, p1, nbi2, p2, comp))

		self.__var_constraints = new_var_constraints

		for hpos in self.__headvar_bindings:
			to_erase = []
			for i, bindings in enumerate(self.__headvar_bindings[hpos]):
				if bindings[0] == bi:
					to_erase.append(i)
				elif bindings[0] > bi:
					self.__headvar_bindings[hpos][i] = (bindings[0]-1, bindings[1])

			# have to erase backwards or else the indices will become
			# incorrect
			to_erase.sort()
			to_erase.reverse()
			for e in to_erase:
				del self.__headvar_bindings[hpos][e]

	def get_body_predicates(self):
		preds = set()
		for b in self.__body:
			preds.add(b.get_predicate())
		return preds

	def is_frame_axiom(self):
		if self.__head.get_relation() != "next":
			return False

		frame_func = self.__head.get_term(0)
		for b in self.__body:
			if b.get_relation() == "true" and b.get_term(0) == frame_func:
				return True
		return False

	def add_var_constraint(self, v1, v2, comparison):
		for bi1, b1 in enumerate(self.__body):
			pos1 = PositionIndex.get_positions(b1, v1)
			if len(pos1) > 0:
				for bi2, b2 in enumerate(self.__body):
					pos2 = PositionIndex.get_positions(b2, v2)
					for p1 in pos1:
						for p2 in pos2:
							if p1 != p2 or bi1 != bi2:
								self.__var_constraints.append((bi1, p1, bi2, p2, comparison))
	
	def add_pos_constraint(self, bi1, p1, bi2, p2, comp):
		self.__var_constraints.append((bi1, p1, bi2, p2, comp))

	# returns (b1, b2) -> [(p1, p2, comp)]
	def get_constraints_by_body_index(self):
		result = {}
		for bi1, p1, bi2, p2, comp in self.__var_constraints:
			result.setdefault((bi1, bi2), []).append((p1, p2, comp))
		return result

	# returns [(bi, pos, comp, order)], where order == 0 if the order of
	# the original constraint was (body_index, position, bi, pos, comp)
	# otherwise order == 1
	def get_constraints_on(self, body_index, position):
		result = []
		for bi1, p1, bi2, p2, comp in self.__var_constraints:
			if (bi1 == body_index and p1 == position):
				result.append((bi2, p2, comp, 0))
			elif (bi2 == body_index and p2 == position):
				result.append((bi1, p1, comp, 1))
		return result

	def get_headvar_binding(self, hpos):
		return self.__headvar_bindings[hpos]

	def mangle_vars(self, prefix):
		self.__head.mangle_vars(prefix)
		for b in self.__body:
			b.mangle_vars(prefix)

	def __str__(self):
		body_str = ""
		for b in self.__body:
			body_str += "    %s\n" % str(b)
		return "(<= %s\n%s)" % (str(self.__head), body_str)
