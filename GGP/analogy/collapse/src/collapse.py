import sys, os
from GDL import *
import gdlyacc
from PositionIndex import PositionIndex

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
			if not self.rule_grounded(r):
				self.__ug_elabs.setdefault(pred,[]).append(r)
			else:
				self.__g_elabs.setdefault(pred,[]).append(r)


	def sentence_grounded(self, sentence):
		if sentence.get_predicate() in self.__static_preds:
			return True
		if sentence.get_predicate() in ['+', '-', '*', '/', '<', '>', '>=', 'plus', 'minus']:
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
			ehead = elab.get_head()
			conflict = False
			for p in PositionIndex.get_all_positions(b):
				t1 = p.fetch(b)
				t2 = p.fetch(ehead)
				if isinstance(t1, Constant) and isinstance(t2, Constant) and t1 != t2:
					# this elab rule is incompatible
					conflict = True
					break
			if conflict:
				continue
			r_copy = r.copy()
			elab_copy = elab.copy()
			expanded.extend(self.expand_with_rule(r_copy, bi, elab_copy))
		
		return expanded

	def expand_with_rule(self, r, bi, elab):
		expanded = []
		b = r.get_body()[bi]
		elab_head = elab.get_head()
		positions = PositionIndex.get_all_positions(b)
		assert positions == PositionIndex.get_all_positions(elab_head)

		# this is so there won't be name collisions
		elab.mangle_vars("__cr_")
		
		# first we have to add the equality constraints constraints from the head
		# of the elab rule to the condition which is being substituted.
		for p1,p2 in elab.get_headvar_constraints():
			r.add_pos_constraint(bi, p1, bi, p2, Comparison('=='))
		
		# propagate the constraints throughout the rule
		r.enforce_equality()
		
		# make the head of the elab rule and the condition being expanded look
		# exactly the same
		preserve_vars = []
		terms = [p.fetch(b) for p in positions]
		for p, t in zip(positions, terms):
			elab_term = p.fetch(elab_head)
			if t == elab_term:
				continue
			if isinstance(elab_term, Constant) and isinstance(t, Variable):
				p.set(b, elab_term.copy())
			elif isinstance(elab_term, Variable) and isinstance(t, Constant):
				p.set(elab_head, t.copy())
			elif isinstance(elab_term, Variable) and isinstance(t, Variable):
				# should change the variables in the elab rule to match the
				# expanded rule. Since the equality constraints on the head
				# variables are already applied to the substituted condition,
				# we should have that if two variables in the elab head are
				# equal, those variables in the condition are equal too
				p.set(elab_head, t.copy())
				preserve_vars.append(t.get_name())
			else:
				assert t == elab_term

		r.enforce_equality(preserve_vars)
		elab.enforce_equality(preserve_vars)
		
		offset = r.num_conditions()
		gr_head = elab.get_head()

		# for each constraint that applied to the condition 
		# being expanded, we have to insert duplicates of 
		# those that apply to the inserted conditions

		# we're going to append all the new rules to the end of
		# the body first, and then add and modify the approriate
		# constraints. After everything is done, remove the body
		# condition that was substituted
		
		# no variable equalities should be drawn here
		for grb in elab.get_body():
			r.add_condition(grb)

		# add the variable constraints
		for p in positions:
			if isinstance(p.fetch(gr_head), Constant):
				continue
			b2b_cons = r.get_constraints_on(bi, p) # body-to-body constraints
			hv_bindings = elab.get_headvar_binding(p)
			for old_i, old_p, comp, order in b2b_cons:
				for bound_i, bound_p in hv_bindings:
					if order == 0:
						# head var first
						r.add_pos_constraint(offset + bound_i, bound_p, old_i, old_p, comp)
					else:
						r.add_pos_constraint(old_i, old_p, offset + bound_i, bound_p, comp)

					if comp.relation() == '==':
						# have to rename the variables in original condition to
						# equal new condition
						new_cond = r.get_cond(bound_i + offset)
						new_name = bound_p.fetch(new_cond)
						old_p.set(r.get_cond(old_i), new_name)

		# finally remove the replaced condition
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
								self.__g_elabs.setdefault(p,[]).append(er)
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
