""" Data structures for elements in GDL """
import sys
from PositionIndex import PositionIndex
from cache_hash import CacheHash
import pdb

CompRelations = ['distinct', '<', '>', '>=']

class Complex(CacheHash):
	def __init__(self, terms):
		self.__terms = terms
		CacheHash.__init__(self)

	def get_terms(self):
		return self.__terms

	def get_terms_copy(self):
		return [t.copy() for t in self.__terms]

	def get_term(self, i): return self.__terms[i]
	
	def mangle_vars(self, prefix):
		for t in self.__terms:
			t.mangle_vars(prefix)
		self.recalc_hash()
	
	def set_term(self, i, val): 
		self.__terms[i] = val
		self.recalc_hash()

	def arity(self): return len(self.__terms)

	def is_complex(self): return True

	def get_var_terms(self):
		result = []
		positions = PositionIndex.get_all_positions(self)
		for p in positions:
			term = p.fetch(self)
			if isinstance(term, Variable):
				result.append(p, term)
		return result

	def covers(self, other):
		if self.arity() != other.arity(): return False
		for t1,t2 in zip(self.__terms, other.__terms):
			if not t1.covers(t2):
				return False
		return True

	def __eq__(self, other):
		if len(self.__terms) != len(other.__terms):
			return False

		for i in range(len(self.__terms)):
			if self.__terms[i] != other.__terms[i]:
				return False
		return True

	def __str__(self):
		s = ""
		for t in self.__terms:
			s += ' %s' % str(t)
		return s

	def get_hash(self):
		return hash(tuple(self.__terms))

class Sentence(Complex):
	# sentence types
	# STATE = next, true
	# ELAB = user defined relations
	# MOVE = movements
	STATE, ELAB, MOVE = range(3)

	def __init__(self, relation, terms, negated = False):
		self.__rel = relation
		if self.__rel in ['next', 'true']:
			self.__type = Sentence.STATE
		elif self.__rel in ['legal', 'does']:
			self.__type = Sentence.MOVE
		else:
			self.__type = Sentence.ELAB

		self.__negated = negated
		Complex.__init__(self, terms)
	
	def copy(self):
		return Sentence(self.__rel, self.get_terms_copy(), self.__negated)
	
	def get_type(self):
		return self.__type

	def negate(self):
		self.__negated = not self.__negated
	
	def is_negated(self):
		return self.__negated

	def get_relation(self): return self.__rel

	def get_predicate(self):
		if self.__rel in ['true', 'next', 'init']:
			return self.get_term(0).get_name()
		if self.__rel in ['legal', 'does']:
			return self.get_term(1).get_name()
		if self.__rel not in ['goal', 'terminal', 'role']:
			return self.__rel
		return None

	def covers(self, other):
		if not isinstance(other, Sentence): return False
		if self.__rel != other.__rel: return False
		return Complex.covers(self, other)

	def __eq__(self, other):
		if not isinstance(other, Sentence): return False
		if self.__rel != other.__rel: return False
		return Complex.__eq__(self, other)

	def __ne__(self, other):
		return not self == other

	def __str__(self):
		if self.arity() == 0:
			return self.__rel

		if self.__negated:
			return "(not (%s%s))" % (self.__rel, Complex.__str__(self))
		else:
			return "(%s%s)" % (self.__rel, Complex.__str__(self))

	def __hash__(self): return hash((self.__rel, Complex.__hash__(self)))

class Function(Complex):
	def __init__(self, name, terms):
		self.__name = name
		Complex.__init__(self, terms)

	def copy(self):
		return Function(self.__name, self.get_terms_copy())

	def get_name(self): return self.__name

	def get_predicate(self): return self.__name

	def covers(self, other):
		if not isinstance(other, Function): return False
		if self.__name != other.__name: return False
		return Complex.covers(other)

	def __eq__(self, other):
		if not isinstance(other, Function): return False
		if self.__name != other.__name: return False
		return Complex.__eq__(self, other)

	def __ne__(self, other):
		return not self == other

	def __str__(self):
		if self.arity() == 0:
			return self.__name
		return "(%s%s)" % (self.__name, Complex.__str__(self))

	def __hash__(self): return hash((self.__name, Complex.__hash__(self)))

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

	def is_complex(self): return False
	
	def covers(self, other):
		# variables cover other variables, and constants
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

	def __hash__(self):
		return hash(self.__rel)

	def __eq__(self, other):
		return self.__rel == other.__rel

	def __str__(self):
		return self.__rel

class Rule(CacheHash):
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

		# a mapping head_pos -> [(body_index, body_pos)]
		self.__headvar_bindings = {}

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

			#assert bound, "Head variable %s not bound to any body variables" % hv
			if not bound:
				print >> sys.stderr, "Head variable %s not bound to any body variables" % hv

		# equality constraints on head
		self.__headvar_constraints = []
		for i, p1 in enumerate(hpositions):
			t1 = p1.fetch(head)
			if isinstance(t1, Variable):
				for p2 in hpositions[i+1:]:
					t2 = p2.fetch(head)
					if t1 == t2:
						self.__headvar_constraints.append((p1, p2))

		CacheHash.__init__(self)

	def get_type(self):
		return self.__head.get_type()

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

		self.recalc_hash()

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
			new_bindings = []
			for i, binding in enumerate(self.__headvar_bindings[hpos]):
				if binding[0] < bi:
					new_bindings.append(binding)
				elif binding[0] > bi:
					new_bindings.append((binding[0]-1, binding[1]))
			self.__headvar_bindings[hpos] = new_bindings

		self.recalc_hash()

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
			pos1 = PositionIndex.get_term_positions(b1, v1)
			if len(pos1) > 0:
				for bi2, b2 in enumerate(self.__body):
					pos2 = PositionIndex.get_term_positions(b2, v2)
					for p1 in pos1:
						for p2 in pos2:
							if p1 != p2 or bi1 != bi2:
								self.__var_constraints.append((bi1, p1, bi2, p2, comparison))
		self.recalc_hash()
	
	def add_pos_constraint(self, bi1, p1, bi2, p2, comp):
		self.__var_constraints.append((bi1, p1, bi2, p2, comp))

	def get_constraints_by_body_index(self):
		"""returns (b1, b2) -> [(p1, p2, comp)]"""

		result = {}
		for bi1, p1, bi2, p2, comp in self.__var_constraints:
			result.setdefault((bi1, bi2), []).append((p1, p2, comp))
		return result

	def get_constraints_on(self, body_index, position):
		"""returns [(bi, pos, comp, order)], where order == 0 if the order of
		   the original constraint was (body_index, position, bi, pos, comp)
		   otherwise order == 1"""

		result = []
		for bi1, p1, bi2, p2, comp in self.__var_constraints:
			if (bi1 == body_index and p1 == position):
				result.append((bi2, p2, comp, 0))
			elif (bi2 == body_index and p2 == position):
				result.append((bi1, p1, comp, 1))
		return result

	def get_headvar_binding(self, hpos):
		return self.__headvar_bindings[hpos]

	def get_all_headvar_bindings(self):
		return self.__headvar_bindings
	
	def get_headvar_constraints(self):
		return self.__headvar_constraints

	def enforce_equality(self, preserve = []):
		"""Make good on all equality constraints by changing terms to be equal,
		preserving certain variable names. If a variable and a constant should
		be equal, the variable is always changed into the constant"""
		

		preserve_var_names = set(preserve)
		# first enforce constraints on body
		changed = True
		while changed:
			# have to run through multiple times to propogate the changes completely
			changed = False
			for bi1, p1, bi2, p2, comp in self.__var_constraints:
				if comp.relation() != '==':
					continue
					
				t1 = p1.fetch(self.__body[bi1])
				t2 = p2.fetch(self.__body[bi2])
				if t1 == t2:
					# all is good, nothing to change
					continue
				if isinstance(t1, Variable) and isinstance(t2, Variable):
					if t1.get_name() in preserve_var_names:
						p2.set(self.__body[bi2], t1.copy())
					elif t2.get_name() in preserve_var_names:
						p1.set(self.__body[bi1], t2.copy())
					else:
						name = '__eq_%s_%s' % (t1.get_name(), t2.get_name())
						preserve_var_names.add(name)
						p1.set(self.__body[bi1], Variable(name))
						p2.set(self.__body[bi2], Variable(name))
					changed = True
				elif isinstance(t1, Constant) and isinstance(t2, Constant):
					assert False, "Trying to equate two unequal constants %s, %s" % (str(t1), str(t2))
				elif isinstance(t1, Constant) and isinstance(t2, Variable):
					p2.set(self.__body[bi2], t1.copy())
					changed = True
				else:
					assert isinstance(t1, Variable) and isinstance(t2, Constant)
					p1.set(self.__body[bi1], t2.copy())
					changed = True
			
			# enforce equality constraints on the head
			for p1, p2 in self.__headvar_constraints:
				t1 = p1.fetch(self.__head)
				t2 = p2.fetch(self.__head)
				if t1 == t2:
					continue
				if t1.get_name() in preserve_var_names:
					p2.set(self.__head, t1.copy())
				elif t2.get_name() in preserve_var_names:
					p1.set(self.__head, t2.copy())
				else:
					name = '__eq_%s_%s' % (t1.get_name(), t2.get_name())
					preserve_var_names.add(name)
					p1.set(self.__head, Variable(name))
					p2.set(self.__head, Variable(name))

			# next enforce head variable bindings
			for hp, bindings in self.__headvar_bindings.items():
				ht = hp.fetch(self.__head)
				if isinstance(ht, Constant):
					# I guess it's possible that a variable in the head got
					# changed to a constant somehow ...
					for bi, bp in bindings:
						bt = bp.fetch(self.__body[bi])
						if ht == bt:
							continue
						if isinstance(bt, Constant):
							assert False, "Trying to equate two unequal constants %s, %s" % (str(ht), str(bt))
						else:
							assert isinstance(bt, Variable)
							bp.set(self.__body[bi], ht.copy())
							changed = True
				else:
					assert isinstance(ht, Variable)
					for bi, bp in bindings:
						bt = bp.fetch(self.__body[bi])
						if ht == bt:
							continue
						if ht.get_name() in preserve_var_names:
							bp.set(self.__body[bi], ht.copy())
						elif bt.get_name() in preserve_var_names:
							hp.set(self.__head, bt.copy())
						else:
							name = '__eq_%s_%s' % (ht.get_name(), bt.get_name())
							preserve_var_names.add(name)
							bp.set(self.__body[bi], Variable(name))
							hp.set(self.__head, Variable(name))
						changed = True

		self.recalc_hash()

	def mangle_vars(self, prefix):
		self.__head.mangle_vars(prefix)
		for b in self.__body:
			b.mangle_vars(prefix)

		self.recalc_hash()

	def __str__(self):
		body_str = ""
		for b in self.__body:
			body_str += "    %s\n" % str(b)

		constraint_str = ""
		for bi1, p1, bi2, p2, comp in self.__var_constraints:
			t1 = p1.fetch(self.__body[bi1])
			t2 = p2.fetch(self.__body[bi2])
			constraint_str += "    (%s %s %s)\n" % (str(comp), str(t1), str(t2))

		return "(<= %s\n%s%s)" % (str(self.__head), body_str, constraint_str)
	
	def __eq__(self, other):
		if self.__head != other.__head: return False
		if self.__body != other.__body: return False
		if self.__var_constraints != other.__var_constraints: return False
		return True

	def get_hash(self):
		return hash((self.__head, tuple(self.__body), tuple(self.__var_constraints)))
