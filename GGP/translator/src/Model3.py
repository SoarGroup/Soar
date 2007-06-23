from PositionIndex import PositionIndex
from GDL import *
import pdb

class Model:

	def __init__(self):
		self.__grounds = set()
	
	def __iter__(self):
		return iter(self.__grounds)

	def __eq__(self, other):
		if not isinstance(other, Model): return False
		return self.__grounds == other.__grounds
	
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
	
	def subtract(self, other):
		self.__grounds.difference_update(other.__grounds)
	
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

