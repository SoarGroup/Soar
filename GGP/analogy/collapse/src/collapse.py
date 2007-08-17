import GDL
from PositionIndex import PositionIndex

grounded_rules = [] # head pred -> [rule]
ungrounded_rules = [] # [rule]
grounded_preds = {} # pred name -> [head]

def sentence_grounded(sentence):
	return sentence.get_relation() in ['true', 'does']

def rule_grounded(rule):
	for b in rule.get_body():
		if not sentence_grounded(b):
			return False
	return True


def expand(r, bi):
	global grounded_rules
	global grounded_preds

	expanded = []

	b = r.get_body()[bi]

	positions = PositionIndex.get_all_positions(b)
	terms = []
	for p in positions:
		terms.append(p.fetch(b))

	indices = grounded_preds[b.get_relation()]

	for i in indices:
		grule = grounded_rules[i]
		if grule.get_head().covers(b):
			r_copy = r.copy()
			gr_copy = grule.copy()
			expand_with_rule(r_copy, bi, gr_copy)

	
	return expanded

def negate_body(r):

	binding_sets = {}
	for b in r.get_body():
		if b.is_comparison():
			binding_set = []
			vars = b.get_var_terms()
			for v in vars:
				# find all conditions that contain the variable
				for b2 in r.get_body():
					if not b2.is_comparison() and b2.has_term(v):
						if b2 not in binding_set:
							binding_set.append(b2)



def expand_with_rule(r, bi, gr):
	expanded = []

	offset = r.num_conditions()
	gr_head = gr.get_head()

	# this is so there won't be name collisions
	gr.mangle_vars("__cr_")

	# set the terms in the ground rule
	# to resemble the ones in the replaced condition
	for p, t in zip(positions, terms):
		t_equiv = p.fetch(gr_head)
		for grb in gr.get_body():
			bpos = PositionIndex.get_positions(grb, t_equiv)
			for bp in bpos:
				bp.set(grb, t)

	# for each constraint that applied to the condition 
	# being expanded, we have to insert duplicates of 
	# those that apply to the inserted conditions

	# we're going to append all the new rules to the end of
	# the body first, and then add and modify the approriate
	# constraints. After everything is done, remove the body
	# condition that was substituted
	
	# no variable equalities should be drawn here
	for grb in gr.get_body():
		r.add_condition(grb)

	# add the variable constraints
	for p in positions:
		b2b_cons = r.get_constraints_on(bi, p) # body-to-body constraints
		hv_bindings = gr.get_headvar_binding(p)
		for old_i, old_p, comp, order in b2b_cons:
			for bound_i, bound_p in hv_bindings:
				# head var first
				if order == 0:
					r.add_pos_constraint(offset + bound_i, bound_p, old_i, old_p, comp)
				else:
					r.add_pos_constraint(old_i, old_p, offset + bound_i, bound_p, comp)

				if comp.relation() == '==':
					# have to rename the variables in original condition to equal new
					# condition
					new_cond = r.get_cond(bound_i + offset)
					new_name = bound_p.fetch(new_cond)
					old_p.set(r.get_cond(old_i), new_name)

	# finally remove the replaced condition
	r.remove_condition(bi)
	expanded.append(r)

	return expanded

def collapse(rules):
	global grounded_rules
	global ungrounded_rules
	global grounded_preds

	# first split the rules into grounded and ungrounded
	for r in rules:
		if len(r.get_body()) == 0:
			continue

		h = r.get_head()
		if not rule_grounded(r):
			ungrounded_rules.append(r)
		else:
			grounded_rules.append(r)
			if h.get_relation() not in ['next', 'legal', 'terminal', 'goal']:
				index = len(grounded_rules) - 1
				grounded_preds.setdefault(h.get_relation(), []).append(index)

	ri = 0
	while ri < len(ungrounded_rules):
		r = ungrounded_rules[ri]
		can_expand = False
		for bi, b in enumerate(r.get_body()):
			if not sentence_grounded(b) and b.get_relation() in grounded_preds:
				cond_index = bi
				can_expand = True
				break

		if can_expand:
			expanded = expand(r, cond_index)
			for r1 in expanded:
				if rule_grounded(r1):
					grounded_rules.append(r1)
				else:
					ungrounded_rules.append(r1)

			del ungrounded_rules[ri]
			ri = 0
		else:
			ri += 1

	for r in grounded_rules:
		print str(r)
	
