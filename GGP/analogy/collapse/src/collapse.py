import GDL

grounded_rules = {} # head pred -> [rule]
ungrounded_rules = [] # [rule]
grounded_preds = {} # pred name -> [head]

def grounded(rule):
	for b in rule.get_body():
		if b.get_relation() not in ['true', 'does']:
			return False
	return True

def expand(r, b, bi):
	global grounded_rules
	global grounded_preds

	expanded = []

	positions = PositionIndex.get_all_positions(b)
	heads = grounded_preds[b.get_relation()]
	for h in heads:
		grules = grounded_rules[h]
		# for each rule that can be substituted into
		# the condition, we have to make a separate copy
		new_rules = []
		for gr in grules:
			# fold the body conditions into the rule
			r_copy = r.copy()
			gr_copy = gr.copy()
			gr_copy.mangle_vars("inserted_rule_")
			# for each constraint that applied to the condition 
			# being expanded, we have to insert duplicates of 
			# those that apply to the inserted conditions

			# we're going to append all the new rules to the end of
			# the body first, and then add and modify the approriate
			# constraints. After everything is done, remove the body
			# condition that was substituted
			
			# no variable equalities should be drawn here
			for grb in grules.get_body():
				r_copy.add_condition(grb)

			# add the variable constraints
			offset = r_copy.num_conditions()
			for p in positions:
				b2b_cons = r_copy.get_constraints_on(bi, p) # body-to-body constraints
				hv_bindings = gr_copy.get_headvar_binding(p)
				for old_i, old_p, comp, order in b2b_cons:
						for bound_i, bound_p in hv_bindings:
							# head var first
							if order == 0:
								r_copy.add_pos_constraint(offset + bound_i, bound_p, old_i, old_p, comp)
							else:
								r_copy.add_pos_constraint(old_i, old_p, offset + bound_i, bound_p, comp)

							if comp.relation() == '==':
								# have to rename the variables in original condition to equal new
								# condition
								new_cond = r_copy.get_cond(bound_i + offset)
								new_name = bound_p.fetch(new_cond)
								old_p.set(r_copy.get_cond(old_i), new_name)

			# finally remove the replaced condition
			r_copy.remove_condition(bi)
			expanded.append(r_copy)
	
	return expanded

def collapse(rules):
	global grounded_rules
	global ungrounded_rules
	global grounded_preds

	# first split the rules into grounded and ungrounded
	for r in rules:
		h = r.get_head()
		if h.get_relation() in ['next', 'legal']:
			continue

		if not grounded(r):
			ungrounded_rules.append(r)
		else:
			grounded_rules.setdefault(h, []).append(r)
			grounded_preds.setdefault(h.get_relation(), []).append(h)

	ri = 0
	while ri < len(ungrounded_rules):
		r = ungrounded_rules[ri]
		can_expand = False
		for bi, b in enumerate(r.get_body()):
			if b.get_relation in grounded_preds:
				cond_to_expand = b
				cond_index = bi
				can_expand = True

		if can_expand:
			expanded = expand(r, cond_to_expand, cond_index)
			for r1 in expanded:
				if grounded(r1):
					grounded_rules.setdefault(r1.get_head(), []).append(r1)
				else:
					ungrounded_rules.append(r1)

			del ungrounded_rules[ri]
			ri = 0
		else:
			ri += 1

	for h, r in grounded_rules.items():
		print str(r[0])
	
