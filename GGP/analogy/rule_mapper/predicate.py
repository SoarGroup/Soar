import sys, os, tempfile
import gdlyacc
import placetype
from PositionIndex import PositionIndex
from GDL import *
import pdb

class Predicate:
	
	# since there are so many predicate comparisons, this should
	# speed up the process
	
	# str -> int
	__name2id = {}
	# int -> (name, type, place_types)
	__id2info = {}
	# next new predicate gets this id
	__id_count = 0

	def __init__(self, name, type, place_types):
		if name in Predicate.__name2id:
			self.__id = Predicate.__name2id[name]
			info = Predicate.__id2info[self.__id]
			assert type == info[1] and len(place_types) == len(info[2]), '%s -> %s, %s -> %s' % (str(type), str(info[1]), str(place_types), str(info[2]))
		else:
			self.__id = Predicate.__id_count
			Predicate.__id2info[Predicate.__id_count] = (name, type, place_types)
			Predicate.__name2id[name] = Predicate.__id_count
			Predicate.__id_count += 1
		self.__type = type
		self.__ptypes = tuple(place_types)
		self.__arity = len(place_types)

	def get_name(self): return Predicate.__id2info[self.__id][0]
	def get_arity(self): return self.__arity
	def get_type(self): return self.__type
	def get_place_types(self): return self.__ptypes

	def __eq__(self, other):
		# since we've made sure that predicates with the same id
		# has the same info, we can just compare ids here
		return self.__id == other.__id
	
	def __hash__(self):
		return self.__id

	def __str__(self):
		return "%s %d %s" % (self.get_name(), self.__type, " ".join(placetype.type_names[t] for t in self.__ptypes))

class TypedEquivalenceClass:

	def __init__(self):
		# maps members to the class (index) they're in
		self.__mem2class = {}
		# maps class indexes to sets of members
		self.__class2mem = {}
		# maps class indexes to types
		self.__class2type = {}

		self.__count = 0
	
	def make_equivalent(self, m1, m2):
		if m1 not in self.__mem2class and m2 not in self.__mem2class:
			self.__mem2class[m1] = self.__count
			self.__mem2class[m2] = self.__count
			# class defaults to unknown type
			self.__class2type[self.__count] = placetype.UNKNOWN
			self.__count += 1

		elif m1 in self.__mem2class and m2 in self.__mem2class:
			# both equivalence classes exist, so they have to be merged into one
			mc = self.__count
			self.__count += 1

			# if one class has a more general type, it is subsumed by the more
			# specific type
			c1 = self.__mem2class[m1]
			c2 = self.__mem2class[m2]
			t1 = self.__class2type[c1]
			t2 = self.__class2type[c2]

			if t1 == t2:
				self.__class2type[mc] = t1
			elif t1 in placetype.type_order and t2 in placetype.type_order[t1]:
				self.__class2type[mc] = t2
			elif t2 in placetype.type_order and t1 in placetype.type_order[t2]:
				self.__class2type[mc] = t1
			else:
				assert False, "Trying to merge two classes with incompatible types %d %d" % (t1, t2)

			self.__class2mem[mc] = self.__class2mem[c1] | self.__class2mem[c2]
			for m in self.__class2mem[mc]:
				self.__mem2class[m] = mc

		elif m1 in self.__mem2class:
			c = self.__mem2class[m1]
			self.__class2mem[c].add(m2)
			self.__mem2class[m2] = c
		else:
			c = self.__mem2class[m2]
			self.__class2mem[c].add(m1)
			self.__mem2class[m1] = c

	def get_class_type(self, c):
		return self.__class2type[c]

	def set_class_type(self, c, t):
		"""Sets the type of a class to be as specific as possible"""

		orig_type = self.__class2type[c]
		if orig_type == t:
			return

#		elif set([orig_type, t]) == set([NUM_MAX, NUM_MIN]):
#			# these two cancel out, it's inconclusive in the end
#			self.__class2type[c] = NUM

		elif orig_type in placetype.type_order:
			if t in placetype.type_order[orig_type]:
				# makes type more specific
				self.__class2type[c] = t
		else:
			assert t in placetype.type_order and orig_type in placetype.type_order[t], "Illegal type assignment %d -> %d" % (orig_type, t)
			# don't do anything here, since the type assignment is more general

	def get_member_type(self, m):
		return self.__class2type[self.__mem2class[m]]
	
	def set_member_type(self, m, t):
		self.set_class_type(self.__mem2class[m], t)
	
	def has_member(self, m):
		return m in self.__mem2class

	def get_or_make_class(self, m):
		if m not in self.__mem2class:
			self.__mem2class[m] = self.__count
			self.__class2mem[self.__count] = set([m])
			self.__class2type[self.__count] = placetype.UNKNOWN
			self.__count += 1
		return self.__mem2class[m]
	
	def get_classes(self):
		return self.__class2mem.keys()

	def get_classes_as_sets(self):
		return self.__class2mem.values()

	def get_all_members(self):
		return self.__mem2class.keys()

def check_min_max(r, ec, score):
	"Find indications that a number is to be minimized, maximized"

	place_types = {}
	for b in r.get_body():
		pos = PositionIndex.get_all_positions(b)
		for p in pos:
			term = p.fetch(b)
			if isinstance(term, Constant) and term.get_name() == 0:
				c = ec.get_or_make_class((b.get_predicate(), p))
				if score == 100 ^ b.is_negated():
					ec.set_class_type(c, placetype.NUM_MIN)
				else:
					ec.set_class_type(c, placetype.NUM_MAX)

	return place_types

def get_predicates(rules, roles):
	"""Extract predicate information from the rules"""

	preds = set()
	pred_names = set()
	predTypes = {}
	# maps places to the equivalence class they're in
	ec = TypedEquivalenceClass()
	for r in rules:
		goal_place_types = {}
		if r.get_head().get_relation() == 'goal':
			score = r.get_head().get_term(1).get_name()
			if score == 0 or score == 100:
				# we can't really say anything about the middle cases
				goal_place_types = check_min_max(r, ec, score)
				score = r.get_head().get_term(1)
			sentences = r.get_body()
		elif r.get_head().get_relation() == 'terminal':
			sentences = r.get_body()
		else:
			sentences = [r.get_head()] + r.get_body()

		for i,s1 in enumerate(sentences):
			s1pred = s1.get_predicate()
			assert s1pred != None

			if s1pred in placetype.preset_types:
				# the first place is supposed to get acted upon, not act upon the
				# second place. The preset places are not to be acted upon
				continue

			predTypes[s1pred] = s1.get_type()
			
			s1pos = PositionIndex.get_all_positions(s1)
			for p1 in s1pos:
				ec.get_or_make_class((s1pred, p1))

			if len(s1pos) == 0:
				preds.add(Predicate(s1pred, s1.get_type(), ()))
				pred_names.add(s1pred)
				continue
			
			added = False
			for s2 in sentences[i+1:]:
				s2pred = s2.get_predicate()
				s2pos = PositionIndex.get_all_positions(s2)
	
				assert s1pred != None and s2pred != None

				for p1 in s1pos:
					place1 = (s1pred, p1)
					t1 = p1.fetch(s1)
					c1 = ec.get_or_make_class(place1)
					if place1 in placetype.preset_types:
						ec.set_class_type(c1, placetype.preset_types[place1])
					for p2 in s2pos:
						place2 = (s2pred, p2)
						t2 = p2.fetch(s2)
						if isinstance(t1, Variable) and isinstance(t2, Variable) and \
								t1 == t2:
							if place2 in goal_place_types:
								ec.set_class_type(c1, goal_place_types[place2])
							elif place2 in placetype.preset_types:
								ec.set_class_type(c1, placetype.preset_types[place2])
							else:
								# put these two places in the same equivalence class
								place1 = (s1pred, p1)
								place2 = (s2pred, p2)
								ec.make_equivalent(place1, place2)

	
	# run through the rules again.  If a role constant appears somewhere, and
	# no non-role constants appear in that place, then that place is an agent
	possible_agent_places = set()
	impossible_agent_places = set()
	for r in rules:
		if r.get_head().get_predicate() != None:
			sentences = [r.get_head()] + r.get_body()
		else:
			sentences = r.get_body()
		for s in sentences:
			for p in PositionIndex.get_all_positions(s):
				place = (s.get_predicate(), p)
				t = p.fetch(s)
				if t in roles:
					if place not in impossible_agent_places:
						possible_agent_places.add(place)
				else:
					if isinstance(t, Constant):
						impossible_agent_places.add(place)
						possible_agent_places.discard(place)
	
	collected = {}
	for place in ec.get_all_members():
		if place in possible_agent_places:
			type = placetype.AGENT
		else:
			type = ec.get_member_type(place)
		collected.setdefault(place[0],[]).append((place, type))

	for p, types in collected.items():
		types.sort(lambda x,y: cmp(x[0],y[0]))
		types_no_place = [t[1] for t in types]
		preds.add(Predicate(p, predTypes[p], types_no_place))
		pred_names.add(p)

	# go over one more time and see if we missed anything
	for r in rules:
		sentences = [r.get_head()] + r.get_body()
		for s in sentences:
			p = s.get_predicate()
			if p == None:
				continue
			if p not in pred_names:
				pos = PositionIndex.get_all_positions(s)
				preds.add(Predicate(p, s.get_type(), tuple([placetype.UNKNOWN] * len(pos))))

	return preds

if __name__ == '__main__':
	gdlyacc.parse_file(sys.argv[1])
	preds = get_predicates(gdlyacc.int_rep.get_all_rules(), gdlyacc.int_rep.get_roles())
	for p in preds:
		print p.get_name(), p.get_type(),
		for t in p.get_place_types():
			print placetype.type_names[t],
		print
