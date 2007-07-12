from PositionIndex import PositionIndex

CompRelations = [ 'distinct', '<', '<=' ]

class Sentence:
	
	def __init__(self, relation, terms):
		self.__rel = relation
		self.__terms = terms
		self.__negated = False

	def copy(self):
		return Sentence(self.__rel, [t.copy() for t in self.__terms])

	def negate(self):
		self.__negated = True

	def get_relation(self): return self.__rel

	def get_terms(self) : return [t.copy() for t in self.__terms]

	def get_term(self, i): return self.__terms[i]

	def set_term(self, i, val): self.__terms[i] = val

	def get_var_terms(self):
		result = []
		for t in self.__terms:
			result.extend(t.get_var_terms())
		return result

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

	def get_name(self): return self.__name

	def get_term(self, i): return self.__terms[i]

	def get_terms(self): return self.__terms

	def get_var_terms(self):
		result = []
		for t in self.__terms:
			result.extend(t.get_var_terms())
		return result

	def set_term(self, i, val): self.__terms[i] = val

	def is_complex(self): return True

	def arity(self): return len(self.__terms)
	
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
		self.__name = name
	
	def copy(self): return Variable(self.__name)
	
	def get_name(self): return self.__name

	def get_var_terms(self): return [ Variable(self.__name) ]

	def is_complex(self): return False
	
	def __eq__(self, other):
		if not isinstance(other, Variable): return False
		return self.__name == other.__name

	def __ne__(self, other):
		return not self == other

	def __str__(self): return self.__name

	def __hash__(self): return hash(self.__name)

class Constant:
	def __init__(self, name):
		self.__name = name

	def copy(self): return Constant(self.__name)
	
	def get_name(self): return self.__name

	def get_var_terms(self): return []

	def is_complex(self): return False

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
		
	def get_head(self):
		return self.__head
	
	def get_body(self):
		return self.__body[:]

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

	# returns (b1, b2) -> [(p1, p2, comp)]
	def get_constraints_by_body_index(self):
		result = {}
		for bi1, p1, bi2, p2, comp in self.__var_constraints:
			result.setdefault((bi1, bi2), []).append((p1, p2, comp))
		return result

	def __str__(self):
		body_str = ""
		for b in self.__body:
			body_str += "    %s\n" % str(b)
		return "(<= %s\n%s)" % (str(self.__head), body_str)
