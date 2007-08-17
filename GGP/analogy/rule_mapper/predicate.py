import sys, os, tempfile
base_dir = os.path.join('..','..')
sys.path.append(os.path.join(base_dir, 'scripts', 'pyparser'))
import gdlyacc
from PositionIndex import PositionIndex
import pdb

comp_rels = ['<', '>', '>=', 'lessThan', 'greaterThan', 'succ']
math_op_rels = ['+', '-', '*', '/', 'minus', 'plus']
obj_loc_place = ('location', PositionIndex([0,0]))
coord_places = set([('location', PositionIndex([0,p])) for p in [1,2]])

# an equivalence class that has one of these places as a member must
# be a number
num_places = set([(c, PositionIndex([0])) for c in comp_rels] + \
                 [(c, PositionIndex([1])) for c in comp_rels] + \
                 [(o, PositionIndex([0])) for o in math_op_rels] + \
                 [(o, PositionIndex([1])) for o in math_op_rels] + \
                 [(o, PositionIndex([2])) for o in math_op_rels])

class Predicate:

	def __init__(self, name, type, place_types):
		self.__name = name
		self.__type = type
		self.__ptypes = tuple(place_types)

	def get_name(self): return self.__name
	def get_arity(self): return len(self.__ptypes)
	def get_type(self): return self.__type
	def get_place_types(self): return self.__ptypes

	def __eq__(self, other):
		return self.__name == other.__name and \
		       self.__type == other.__type and \
		       self.__ptypes == other.__ptypes
	
	def __hash__(self):
		return hash((self.__name, self.__type, self.__ptypes))

	def __str__(self):
		return "%s %s %s" % (self.__name, self.__type, " ".join(self.__ptypes))

class EquivalenceClass:
	def __init__(self):
		# maps members to the class they're in
		self.__mem2class = {}
	
	def make_equivalent(self, m1, m2):
		if m1 not in self.__mem2class and m2 not in self.__mem2class:
			ec = set([m1, m2])
			self.__mem2class[m1] = ec
			self.__mem2class[m2] = ec
		elif m1 in self.__mem2class and m2 in self.__mem2class:
			# both equivalence classes exist, so they have to be merged into one
			merged = self.__mem2class[m1] | self.__mem2class[m2]
			for m in merged:
				self.__mem2class[m] = merged
		elif m1 in self.__mem2class:
			ec = self.__mem2class[m1]
			ec.add(m2)
			self.__mem2class[m2] = ec
		else:
			ec = self.__mem2class[m2]
			ec.add(m1)
			self.__mem2class[m1] = ec

	def get_class(self, m):
		return self.__mem2class[m]

	def get_classes(self):
		classes = []
		for c in self.__mem2class.values():
			if c not in classes:
				classes.append(c)
		return classes

	def get_members(self):
		return self.__mem2class.keys()

def get_predicates(rules):
	"""Extract predicate information from the rules"""

	preds = set()
	pred_names = set()
	predTypes = {}
	# maps places to the equivalence class they're in
	ec = EquivalenceClass()
	for r in rules:
		sentences = [r.get_head()] + r.get_body()
		for i,s1 in enumerate(sentences):
			s1pred = s1.get_predicate()
			if s1pred != None:
				predTypes[s1pred] = s1.get_type()
			
			s1pos = PositionIndex.get_all_positions(s1)
			if len(s1pos) == 0:
				preds.add(Predicate(s1pred, s1.get_type(), ()))
				pred_names.add(s1pred)
				continue
			
			added = False
			for s2 in sentences[i+1:]:
				s2pred = s2.get_predicate()
				s2pos = PositionIndex.get_all_positions(s2)

				if s1pred == None or s2pred == None:
					continue

				for p1 in s1pos:
					for p2 in s2pos:
						if p1.fetch(s1) == p2.fetch(s2):
							# put these two places in the same equivalence class
							place1 = (s1pred, p1)
							place2 = (s2pred, p2)
							ec.make_equivalent(place1, place2)
	

	place2type = {}
	classes = ec.get_classes()
	for c in classes:
		if obj_loc_place in c:
			class_type = 'object'
		# by arranging in this order, coordinates are
		# labeled before they are labeled numbers
		elif coord_places & c:
			class_type = 'coordinate'
		elif num_places & c:
			class_type = 'number'
		else:
			class_type = 'unknown'

		for m in c:
			place2type[m] = class_type
	
	pred2types = {}
	for p, t in place2type.items():
		pred2types.setdefault(p[0],[]).append(t)
	
	for p, types in pred2types.items():
		preds.add(Predicate(p, predTypes[p], types))
		pred_names.add(p)

	# go over one more time and see if we missed anything
	for r in rules:
		sentences = [r.get_head()] + r.get_body()
		for s in sentences:
			p = s.get_predicate()
			if p not in pred_names:
				pos = PositionIndex.get_all_positions(s)
				preds.add(Predicate(p, s.get_type(), tuple(['unknown'] * len(pos))))

	return preds

if __name__ == '__main__':
	gdlyacc.parse_file(sys.argv[1])
	preds = get_predicates(gdlyacc.int_rep.get_all_rules())
	for p in preds:
		print p.get_name(), p.get_type(),
		for t in p.get_place_types():
			print t,
		print
