import sys, os
from GDL import *
import gdlyacc
from PositionIndex import PositionIndex

def same_body_preds(r1, r2):
	if len(r1.get_body()) != len(r2.get_body()):
		return False
	for b1, b2 in zip(r1.get_body(), r2.get_body()):
		if b1.get_predicate() != b2.get_predicate():
			return False
	return True

class Expander:
	def __init__(self, ir):
		self.__static_preds = set(s.get_predicate() for s in ir.get_statics())

		# grounded non-elab rules
		self.__g_rules = [] # head pred -> [rule]
		# ungrounded state update rules
		self.__ug_rules = [] # [rule]

		# grounded elab rules
		self.__g_elabs = {} # pred name -> [rule]
		# ungrounded elab rules
		self.__ug_elabs = {} # pred name -> [rule]
		
		self.__recursive_preds = set()
		# split the rules into grounded and ungrounded
		for r in ir.get_update_rules() + ir.get_goal_rules() + ir.get_terminal_rules():
			if len(r.get_body()) == 0:
				continue

			if not self.rule_grounded(r):
				self.__ug_rules.append(r)
			else:
				self.__g_rules.append(r)
		
		for r in ir.get_elab_rules():
			if len(r.get_body()) == 0:
				continue
		
			pred = r.get_head().get_predicate()
			for b in r.get_body():
				if b.get_predicate() == pred:
					# recursive rule
					self.__recursive_preds.add(pred)
					self.__g_elabs.setdefault(pred,[]).append(r)
					continue
			if not self.rule_grounded(r):
				self.__ug_elabs.setdefault(pred,[]).append(r)
			else:
				self.__g_elabs.setdefault(pred,[]).append(r)


	def sentence_grounded(self, sentence):
		if sentence.get_predicate() in self.__static_preds:
			return True
		if sentence.get_predicate() in ['+', '-', '*', '/', '<', '>', '>=', 'plus', 'minus']:
			return True
		if sentence.get_predicate() in self.__recursive_preds:
			return True
		if sentence.is_negated():
			# can't handle negations
			return True
		else:
			return sentence.get_relation() in ['true', 'does']

	def rule_grounded(self, rule):
		for b in rule.get_body():
			if not self.sentence_grounded(b):
				return False
		return True

	# let's hold off on this for now
	#
	#def negate_body(r):
	#
	#	binding_sets = {}
	#	for b in r.get_body():
	#		if b.is_comparison():
	#			binding_set = []
	#			vars = b.get_var_terms()
	#			for v in vars:
	#				# find all conditions that contain the variable
	#				for b2 in r.get_body():
	#					if not b2.is_comparison() and b2.has_term(v):
	#						if b2 not in binding_set:
	#							binding_set.append(b2)

	def can_expand(self, r):
		"""Return the index of a condition in r that can be expanded given the
		current set of grounded elaborations. If no such condition exists,
		return -1"""

		for bi, b in enumerate(r.get_body()):
			if b.is_negated():
				# ignore negations for now
				continue
			if not self.sentence_grounded(b):
				elab_rel = b.get_relation()
				if elab_rel in self.__g_elabs and elab_rel not in self.__ug_elabs:
					# only those elaborations for which all generating rules are
					# grounded can be expanded. Otherwise, we might miss some
					# instantiations
					return bi
		
		# can't expand any sentences
		return -1

	def expand(self, r, bi):
		expanded = []
		b = r.get_body()[bi]
		pred = b.get_predicate()
		assert pred in self.__g_elabs

		for elab in self.__g_elabs[pred]:
			r_copy = r.copy()
			elab_copy = elab.copy()
			expanded.extend(self.expand_with_rule(r_copy, bi, elab_copy))
		
		return expanded

	def expand_with_rule(self, r, bi, elab):
		expanded = []
		b = r.get_body()[bi]
		elab_head = elab.get_head()

		for grb in elab.get_body():
			r.add_condition(grb)

		r.remove_condition(bi)
		expanded.append(r)

		return expanded
	
	def collapse(self):

		# the main loops

		# first try to ground all the elab rules, since grounding update, goal, or
		# terminal rules has no effect on the grounding of other rules
		while len(self.__ug_elabs) > 0:
			print len(self.__ug_elabs)
			print len(self.__g_elabs)
			pdb.set_trace()
			for p, rules in self.__ug_elabs.items():
				for r in rules[:]:
					expand_bi = self.can_expand(r)
					if expand_bi >= 0:
						expanded = self.expand(r, expand_bi)
						rules.remove(r)
						for er in expanded:
							if self.rule_grounded(er):
								existing = self.__g_elabs.setdefault(p,[])
								duplicate = False
								for oldr in existing:
									if same_body_preds(oldr, er):
										duplicate = True
										break
								if not duplicate:
									existing.append(er)
							else:
								rules.append(er)
						if len(rules) == 0:
							del self.__ug_elabs[p]
		
		# at this point, all elaboration rules are grounded. We should be able to
		# go through all the other kinds of ungrounded rules
		while len(self.__ug_rules) > 0:
			for r in self.__ug_rules[:]:
				expand_bi = self.can_expand(r)
				assert expand_bi >= 0, "Rule is impossible to expand"
				expanded = self.expand(r, expand_bi)
				self.__ug_rules.remove(r)
				self.__ug_rules.extend(expanded)

if __name__ == '__main__':
	gdlyacc.parse_file(sys.argv[1])
	ex = Expander(gdlyacc.int_rep)
	grounded = ex.collapse()
	for r in grounded:
		print r
