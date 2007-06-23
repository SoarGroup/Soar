from PositionIndex import PositionIndex
from GDL import *

class Model:

	def __init__(self):
		self.__grounds = set()
	
	def __iter__(self):
		return iter(self.__grounds)
	
	def add_ground(self, s):
		if s.get_relation() == "legal":
			g = Sentence("does", s.get_terms())
		elif s.get_relation() == "next":
			g = Sentence("true", s.get_terms())
		else:
			g = s
		self.__grounds.add(g)
	
	def count(self): return len(self.__grounds)

	def union(self, other):
		self.__grounds.update(other.__grounds)
	
	def copy(self):
		copy = Model()
		copy.__grounds = self.__grounds.copy()
		return copy

	def print_m(self):
		for g in self.__grounds:
			print g
		
	def intersect(self, other, my_i, other_i):
		'Return all grounds g such that there exists h in other and g[my_i] = h[other_i]'
		
		res = set()
		for g in self.__grounds:
			for h in other.__grounds:
				if g.get_term(my_i) == h.get_term(other_i):
					res.add(g)
					break
		return res

	# leave only those grounds whose terms in each position are mutually equal
	def filter_term_equality(self, positions):
		for g in self.__grounds.copy():
			for i in range(len(positions)):
				for j in range(len(positions))[i+1:]:
					v1 = positions[i].fetch(g)
					v2 = positions[j].fetch(g)
					if v1 != v2:
						self.__grounds.remove(g)

	# leave only those grounds that are compatible with the one passed in
	# constraints is a list of tuples (p1, p2, comparison)
	# it's important that p1 is for the grounds in the model and p2 is for
	# the compatible ground, and that the comparison orientation is correct
	def filter_compatible(self, ground, constraints):
		for g in self.__grounds.copy():
			for p1, p2, c in contraints:
				val1 = p1.fetch(g)
				val2 = p2.fetch(ground)
				if not c.check(val1, val2):
					self.__grounds.remove(g)

#	def unary_constrain(self, vp):
#		# remove all models that have different values for all the positions one variable appears in
#		for g in self.__grounds[:]:
#			values = vp.get_values(g)
#			if not values.uniform():
#				self.__grounds.remove(g)

#	@staticmethod
#	def binary_constrain(m1, m2, vp1, vp2, constraint):
#		# for each ground in the first model, make sure there exists a matching value in the
#		# second model
#		for g1 in m1.__grounds.copy():
#			found_match = False
#			val1 = vp1.get_values(g1)
#			for g2 in m2.__grounds:
#				val2 = vp2.get_values(g2)
#				if constraint.check(val1, val2):
#					found_match = True
#					m2_matched.add(g2)
#					break
#			if not found_match:
#				m1.__grounds.remove(g1)
#
#		# do the same for the second model
#		for g2 in m2.__grounds.copy():
#			found_match = False
#			val2 = vp2.get_values(g2)
#			for g1 in m1.__grounds:
#				val1 = vp1.get_values(g1)
#				if constraint.check(val1, val2)
#					found_match = True
#					break
#			if not found_match:
#				m2.__grounds.remove(g2)

class TotalModel:
	def __init__(self):
		self.__userrels = {}
		self.__legals = {}
		self.__persistents = {}

	def __get_map(self, sentence):
		rel = sentence.get_relation()
		if rel in ["true", "init", "next"]:
			return self.__persistents
		if rel == "does":
			move = sentence.get_term(1).get_name()
			return self.__legals
		else:
			return self.__userrels
	
	def is_modeled(self, sentence):
		return sentence.get_predicate() in self.__get_map(sentence)

	def get_model(self, sentence):
		return self.__get_map(sentence)[sentence.get_predicate()]
	
	def add_model(self, sentence, model):
		self.__get_map(sentence)[sentence.get_predicate()] = model
	
	def add_userrel_model(self, relation, model):
		assert relation != "init"
		self.__userrels[relation] = model
	
	def add_persistent_model(self, func_name, model):
		self.__persistents[func_name] = model
	
	def print_m(self):
		print "Persistents:"
		for p, m in self.__persistents.items():
			print p
			m.print_m()

		print "Legals:"
		for p, m in self.__legals.items():
			print p
			m.print_m()

		print "User relations:"
		for p, m in self.__userrels.items():
			print p
			m.print_m()

class ConstrainedModel:
	def __init__(self, rule):
		# map from (body_index, body_index) to a list [(pos, pos, comp)]
		self.__constraints = rule.get_constraints_by_body_index()
		# __models[i] is the model ith body condition, in list form
		self.__models = []
		# __valid_combs[i] is a list of indexes
		# __valid_combs[i][j] is an index of __models[j]
		self.__valid_combs = []

	def get_size(self): return len(self.__valid_combs)

	# check that the ground g1 that models the bi1th body condition
	# and the ground g2 that models the bi2th body condition fulfill
	# all constraints between the two body conditions
	def __compatible(self, bi1, bi2, g1, g2):

		if (bi1, bi2) in self.__constraints:
			constraints = self.__constraints[(bi1, bi2)]
			for pos1, pos2, comp in constraints:
				val1 = pos1.fetch(g1)
				val2 = pos2.fetch(g2)
				if not comp.check(val1, val2): return False
		# have to check both orderings 
		elif (bi2, bi1) in self.__constraints:
			constraints = self.__constraints[(bi2, bi1)]
			for pos2, pos1, comp in constraints:
				val1 = pos1.fetch(g1)
				val2 = pos2.fetch(g2)
				if not comp.check(val2, val1): return False
		else:
			return True

		return True

	def add_constraint(self, bi1, bi2, pos1, pos2, comp):
		self.__constraints.setdefault((bi1, bi2), []).append((pos1, pos2, comp))

	# Add a model to the list, extending all valid combinations with new grounds
	# from this model. The new extended grounds must still satisfy all constraints.
	# Models have to be added in order!
	def add_model(self, model):
		# we're working with lists here, rather than the sets that we use for other models
		self.__models.append([g for g in model])
		if len(self.__models) == 1:
			self.__valid_combs = [[i] for i in range(len(self.__models[0]))]
		else:
			new_valid_combs = []
			new_body_index = len(self.__models) - 1
			for comb in self.__valid_combs:
				for new_ground_i, new_ground in enumerate(model):
					compatible = True
					for model_i, ground_i in enumerate(comb):
						old_ground = self.__models[model_i][ground_i]
						if not self.__compatible(model_i, new_body_index, old_ground, new_ground):
							compatible = False
							break
					if compatible:
						# this ground is compatible with all existing grounds in this
						# particular combination. Extend the combination with this ground
						new_valid_combs.append( comb + [new_ground_i] )
				
			self.__valid_combs = new_valid_combs
	
	def get_valid_combinations(self, bi_pos):
		valids = []
		for comb in self.__valid_combs:
			val_comb = []
			for bi, pos in bi_pos:
				val_comb.append(pos.fetch(self.__models[bi][comb[bi]]))
			valids.append(val_comb)
		return valids
