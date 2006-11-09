#!/usr/bin/python

from SoarProduction import UniqueNameGenerator
from SoarProduction import SoarProduction
from ElementGGP import ElementGGP as ElemGGP
import ElementGGP
import re

name_gen = UniqueNameGenerator()

class GDLSoarVarMapper:
	def __init__(self, var_gen):
		self.var_gen = var_gen
		self.var_map = dict()

	def get_var(self, gdl_var):
		if gdl_var[0] == '?':
			gdl_var = gdl_var[1:]

		if self.var_map.has_key(gdl_var):
			return self.var_map[gdl_var]
		else:
			# try to name gdl variables and soar variables similarly
			soar_var = self.var_gen.get_name(gdl_var)
			self.var_map[gdl_var] = soar_var
			return soar_var

# makes a typical skeleton production for use in GGP
def MakeTemplateProduction(type, name, game_name, gs = True, facts = True, move = True):
	p = SoarProduction(name_gen.get_name(name), type)
	s_cond = p.add_condition()
	s_cond.add_ground_predicate("name", game_name)
	if gs:
		p.add_condition(s_cond.add_id_predicate("gs"))
	
	if facts:
		p.add_condition(s_cond.add_id_predicate("facts"))

	if move:
		p.add_condition(p.get_il_cond().add_id_predicate("last-moves"))

	return p


def ParseSentenceToAction(sentence, production, action, var_mapper):
	id = action.add_id_wme_action(sentence[0])
	id_action = production.add_action(id)
	for v, i in zip(list(sentence)[1:], range(1, len(sentence))):
		if v[0] == '?':
			# variable
			id_action.add_id_wme_action("p%d" % i, var_mapper.get_var(v))
		else:
			id_action.add_ground_wme_action("p%d" % i, v)

def ParseSentenceToCondition(sentence, production, condition, var_mapper, negated = False, check_new = False, match_new = False):
	if (isinstance(sentence, str) or len(sentence) == 1) and not check_new:
		# we don't have chained conditions, so we can just check for absence
		# of attribute
		if isinstance(sentence, str):
			id = condition.add_id_predicate(sentence, negated)
		else:	
			id = condition.add_id_predicate(sentence[0], negated)
		id_cond = production.add_condition(id)
	else:
		# we have to check for absence of specific id match for attribute
		id = condition.add_id_predicate(sentence[0])
		id_cond = production.add_condition(id)
		if negated:
			production.make_negative_conjunction([condition, id_cond])
	
	if check_new:
		if match_new:
			id_cond.add_id_predicate("new")
		else:
			id_cond.add_id_predicate("new", True)

	if not isinstance(sentence, str):
		for i in range(1, len(sentence)):
			if not isinstance(sentence[i], str):
				raise Exception(str(sentence))
			attrib_name = "p%d" % i
			if sentence[i][0] == '?':
				# match on a variable
				id_cond.add_id_predicate(attrib_name, name=var_mapper.get_var(sentence[i]))
	
			else:
				# match a literal
				id_cond.add_ground_predicate(attrib_name, sentence[i])
	
def ParseGDLBodyToCondition(body, prod, gs_cond, fact_cond, move_cond, var_mapper, check_new = True, match_new = False):
	"""
	Parse a set of literals and build the appropriate conditions on the
	specified soar production
	"""
	
	negate = False
	for b in body:
		if b[0].lower() == "not":
			c = b[1]
			negate = True
		else:
			c = b
		
		if c[0].lower() == "true":
			if negate and len(c[1]) > 1:
				# we have to make a new ^gs condition for the negation
				new_gs_cond = prod.add_condition(gs_cond.head_var)
			else:
				new_gs_cond = gs_cond

			ParseSentenceToCondition(c[1], prod, new_gs_cond, var_mapper, negate, check_new, match_new)

		elif c[0].lower() == "does":
			role = c[1]
			# there should always only be one last move for each role. If the condition
			# for it already exists, use it. Otherwise create a new one
			role_cond_list = prod.get_conditions("io.input-link.last-moves.%s" % role)
			if len(role_cond_list) == 0:
				role_cond = prod.add_condition(move_cond.add_id_predicate(role))
			else:
				if negate and not isinstance(c[2], str):
					# we want to use the same id, but create another condition
					role_cond = prod.add_condition(role_cond_list[0].head_var)
				else:
					role_cond = role_cond_list[0]
			ParseSentenceToCondition(c[2], prod, role_cond, var_mapper, negate)

		elif c[0].lower() != "distinct": # must be a fact
			if negate and len(c) > 1:
				new_fact_cond = prod.add_condition(fact_cond.head_var)
			else:
				new_fact_cond = fact_cond
			ParseSentenceToCondition(c, prod, new_fact_cond, var_mapper, negate)

def ParseGDLBodyToAction(body, prod, gs):
	"""
	Parse a set of literals and build the appropriate conditions on the
	specified soar production
	"""
	negate = False
	for b in body:
		if b[0].lower() == "true": 
			ParseSentenceToAction(b[1], prod, gs)
		else:
			# can't handle anything else right now
			print "Warning: Can't handle desired condition"

def ParseDistinctions(literals, prod, var_mapper):
	for l in literals:
		if l[0].lower() == "distinct":
			v1 = var_mapper.get_var(l[1])
			if l[2][0] == '?':
				v2 = var_mapper.get_var(l[2])
				prod.add_var_distinction(v1, v2)
			else:
				prod.add_value_distinction(v1, l[2])


def MakeInitRule(game_name, role, init_conds, facts, min_success_score, tokens):
	sp = SoarProduction("init-%s" % game_name, "propose");
	c = sp.add_condition()
	c.add_ground_predicate("superstate", "nil")
	c.add_ground_predicate("name", "", True)
	op = sp.add_action().add_op_proposal("+ >")
	sp.add_action(op).add_ground_wme_action("name", "init-%s" % game_name)

	asp = sp.get_apply_rules(name_gen)[0]

	state_action = asp.add_action()
	state_action.add_ground_wme_action("name", game_name)
#	state_action.add_ground_wme_action("score", "0")

	d_var = state_action.add_id_wme_action("desired")
	#asp.add_action(d_var).add_id_wme_action("achieved-score")

	game_state_var = state_action.add_id_wme_action("gs")
	game_facts_var = state_action.add_id_wme_action("facts")
	gs_actions = asp.add_action(game_state_var)
	gs_actions.add_ground_wme_action("role", role)

	var_map = GDLSoarVarMapper(UniqueNameGenerator())
	for ic in init_conds:
		ParseSentenceToAction(ic, asp, gs_actions, var_map)

	fact_actions = asp.add_action(game_facts_var)
	for f in facts:
		ParseSentenceToAction(f, asp, fact_actions, var_map)
	
	for t in tokens:
		state_action.add_id_wme_action(t)

	return [sp, asp]


def SoarifyStr(s):
	p = re.compile('(\s|\?|\(|\))')
	result = p.sub('_', str(s))
	for i in range(len(result)):
		if result[i] != '_':
			return result[i:]
	
	# all underscores?
	return "a"


def FindNecessaryStructures(head, body):
	use_move = False
	use_gs = False
	use_facts = False
	for b in body:
		if b[0].lower() == "not":
			relation = b[1][0].lower()
		else:
			relation = b[0].lower()

		if relation == "does":
			use_move = True
		elif relation == "true":
			use_gs = True
		else:
			use_facts = True
	
	if isinstance(head, str):
		head_rel = head.lower()
	else:
		head_rel = head[0].lower()
		
	if head_rel == "next":
		use_gs = True # because we have to check for lack of it later
	elif head_rel != "legal" and head_rel != "goal" and head_rel != "terminal":
		# an elaboration
		use_facts = True
	
	return (use_gs, use_facts, use_move)

# Process axioms of the form
#
# (<= (head)
#     (body1)
#     (body2)
#     ...
# )
def TranslateImplication(game_name, head, body, tokens, remove, min_success_score):
	prod_name = name_gen.get_name(SoarifyStr(str(head)))
	
	use_gs, use_facts, use_move = FindNecessaryStructures(head, body)

	sp = MakeTemplateProduction("propose", prod_name, game_name, use_gs, use_facts, use_move)
	var_mapper = GDLSoarVarMapper(sp.var_gen)
	if use_gs:
		gs_cond = sp.get_conditions("gs")[0]
	else:
		gs_cond = None
	
	if use_facts:
		fact_cond = sp.get_conditions("facts")[0]
	else:
		fact_cond = None
	
	if use_move:
		move_cond = sp.get_conditions("io.input-link.last-moves")[0]
	else:
		move_cond = None

	if isinstance(head, str):
		relation = head.lower()
		params = []
	else:
		relation = head[0].lower()
		params = list(head)[1:]

	if relation == "terminal":
		sp.type = "elaborate"
		ParseGDLBodyToCondition(body, sp, gs_cond, fact_cond, move_cond, var_mapper)
		ParseDistinctions(body, sp, var_mapper)
#			sp_sel_success = sp.copy(name_gen.get_name(sp.name))
#			sp_sel_success.first_state_cond.add_id_predicate("duplicate-of*")
#			desired_var = sp_sel_success.first_state_cond.add_id_predicate("desired")
#			sp_sel_failure = sp_sel_success.copy(name_gen.get_name(sp_sel_success.name))
		
		sp_sel = sp.copy(name_gen.get_name(sp.name))
#			sp.first_state_cond.add_id_predicate("duplicate-of*", True)
#			sp.add_halt()
#			
#			sp_sel_success.add_condition().add_ground_predicate("score", ">= %d" % min_success_score)
#			sp_sel_success.add_action().add_id_wme_action("success", desired_var)
#			
#			sp_sel_failure.add_condition().add_ground_predicate("score", "< %d" % min_success_score)
#			sp_sel_failure.add_action().add_id_wme_action("failure", desired_var)

#			return [sp, sp_sel_success, sp_sel_failure]

		sp_sel = sp.copy(name_gen.get_name(sp.name))
		sp_sel.first_state_cond.add_id_predicate("duplicate-of*")
		sp_sel.add_action().add_id_wme_action("terminal")
		sp.first_state_cond.add_id_predicate("duplicate-of*", True)
		sp.add_halt()
		
		return [sp, sp_sel]

	elif relation == "legal":
		ParseGDLBodyToCondition(body, sp, gs_cond, fact_cond, move_cond, var_mapper, check_new = False)
		return TranslateLegal(game_name, sp, head, body, var_mapper)
	elif relation == "next":
		ParseGDLBodyToCondition(body, sp, gs_cond, fact_cond, move_cond, var_mapper)
		if not use_move:
			# This is a special case, because normally, the presence of a move
			# command on the input link maintains the progression of the state.
			# If this axiom does not check for that, we have to add a token
			# to the top state every time we progress one step (which is consumed
			# when the rule associated with this axiom fires) so that we limit
			# one firing of this rule per step
			token_name = name_gen.get_name(SoarifyStr(head))
			tokens.append(token_name)
			return TranslateNext(game_name, sp, head, body, var_mapper, remove, token_name)
		else:
			return TranslateNext(game_name, sp, head, body, var_mapper, remove)

	elif relation == "goal":
		ParseGDLBodyToCondition(body, sp, gs_cond, fact_cond, move_cond, var_mapper)
		return TranslateGoal(game_name, sp, head, body, var_mapper, min_success_score)

	else:
		# this can only be some implied fact
		ParseGDLBodyToCondition(body, sp, gs_cond, fact_cond, move_cond, var_mapper)
		return TranslateElaboration(game_name, sp, head, body, var_mapper)
		

def TranslateLegal(game_name, sp, head, body, var_mapper = dict()):

	# make sure there's no command
	sp.get_ol_cond().add_id_predicate("<cmd-name>", negate=True, name="cmd")

	if isinstance(head[2], str):
		params = list(head)[3:]
		role = head[1]
		move = head[2]
	else:
		params = list(head[2])[1:]
		role = head[1]
		move = head[2][0]

	op_name = name_gen.get_name("%s-%s" % (role, move))

	op_elab = sp.add_op_proposal("+ <")
	op_elab.add_ground_wme_action("name", op_name)
	for i in range(len(params)):
		if params[i][0] == '?':
			op_elab.add_id_wme_action("p%d" % (i+1), var_mapper.get_var(params[i]))
		else:
			op_elab.add_ground_wme_action("p%d" % (i+1), params[i])

	if body != None:
		ParseDistinctions(body, sp, var_mapper)
	
	# apply rule

	ap = sp.get_apply_rules(name_gen)[0]
	ap.first_state_cond.add_ground_predicate("name", game_name)
	op_cond = ap.get_conditions("operator")[0]
	ol_var = ap.get_ol_cond().head_var

	param_vars = []
	for i in range(len(params)):
		param_vars.append(op_cond.add_id_predicate("p%d" % (i+1)))

	move_act = ap.add_action(ap.add_action(ol_var).add_id_wme_action(move))
	for i in range(len(param_vars)):
		move_act.add_id_wme_action("p%d" % (i+1), param_vars[i])
	
	return [sp, ap]

def TranslateNext(game_name, sp, head, body, var_mapper = dict(), remove = True, token = ""):
	rel_name = head[1][0]
	op_name = name_gen.get_name("next-%s" % rel_name)
	params = list(head[1])[1:]

	# have to add condition to check for lack of head relation to not have operator no change
	negated_head = ElemGGP("(not)") + head.deep_copy()
	negated_head[1][0] = "true"
	gs_cond = sp.get_conditions("gs")[0]
	ParseGDLBodyToCondition([negated_head], sp, gs_cond, None, None, var_mapper, match_new = True)

	op_elab = sp.add_op_proposal("+ > =")
	op_elab.add_ground_wme_action("name", op_name)
	op_elab.add_id_wme_action("add")
	for i in range(len(params)):
		if params[i][0] == '?':
			op_elab.add_id_wme_action("p%d" % (i+1), var_mapper.get_var(params[i]))
		else:
			op_elab.add_ground_wme_action("p%d" % (i+1), params[i])
	
	if token != "":
		sp.first_state_cond.add_id_predicate(token)

		# not only do we have to check for the presence of the token, we also have
		# to make sure this rule doesn't fire until a move was made (i.e., check
		# for existence of some move on the output link
		sp.add_condition(sp.get_il_cond().add_id_predicate("last-moves")).add_id_predicate("<role>")

	if body != None:
		ParseDistinctions(body, sp, var_mapper)

	# apply rule
	ap = sp.get_apply_rules(name_gen)[0]
	ap.first_state_cond.add_ground_predicate("name", game_name)
	op_cond = ap.get_conditions("operator")[0]
	gs_var = ap.first_state_cond.add_id_predicate("gs")
	
	param_vars = []
	for i in range(len(params)):
		param_vars.append(op_cond.add_id_predicate("p%d" % (i+1)))

	next_act = ap.add_action(ap.add_action(gs_var).add_id_wme_action(rel_name))
	# we have to label newly added relations so they don't get deleted immediately
	# The new label will be removed by the progress-state operator
	next_act.add_id_wme_action("new")
	
	for i in range(len(param_vars)):
		next_act.add_id_wme_action("p%d" % (i+1), param_vars[i])

	if token != "":
		token_var = ap.first_state_cond.add_id_predicate(token)
		ap.add_action().remove_id_wme_action(token, token_var)
	
	# there are no frame axioms, so make productions that get rid of this after one
	# step
	if remove:
		ap_copy = ap.copy(name_gen.get_name(ap.name))
		ap.add_condition(gs_var).add_id_predicate(rel_name, negate=True)
		old_rel_var = ap_copy.add_condition(gs_var).add_id_predicate(rel_name)
		ap_copy.add_action(gs_var).remove_id_wme_action(rel_name, old_rel_var)
		
		# an operator that removes the relation if it doesn't have a new label
		op_name = name_gen.get_name("remove-%s" % rel_name)
		remove_prod_name = name_gen.get_name(op_name)
		sp_remove = MakeTemplateProduction("propose", remove_prod_name, game_name, facts = False, move = False)
		gs_cond = sp_remove.get_conditions("gs")[0]
		rel_var = gs_cond.add_id_predicate(rel_name)
		sp_remove.add_condition(rel_var).add_id_predicate("new", True)
		op_elab = sp_remove.add_op_proposal("> =")
		op_elab.add_ground_wme_action("name", op_name)
		op_elab.add_id_wme_action("remove")
		op_elab.add_id_wme_action("relation", rel_var)
		
		ap_remove = sp_remove.get_apply_rules(name_gen)[0]
		gs_var = ap_remove.add_condition().add_id_predicate("gs")
		rel_var = ap_remove.get_conditions("operator")[0].add_id_predicate("relation")
		ap_remove.add_action(gs_var).remove_id_wme_action(rel_name, rel_var)
		
		return [sp, ap, ap_copy, sp_remove, ap_remove]

	return [sp, ap]


def TranslateGoal(game_name, sp, head, body, var_mapper, score):
	sp.type = "elaborate"
	sp.first_state_cond.add_id_predicate("terminal")
	desired_var = sp.first_state_cond.add_id_predicate("desired")
	
	if body != None:
		ParseDistinctions(body, sp, var_mapper)
	
	if head[2][0] == '?':
		# I don't know how to handle this right now. Does it even come up?
		raise Exception("Variable Goal Score")
	else:
		if int(head[2]) >= score:
			sp.add_action().add_id_wme_action("success", desired_var)
		else:
			sp.add_action().add_id_wme_action("failure", desired_var)

		return [sp]

def TranslateElaboration(game_name, sp, head, body, var_mapper):
	if body != None:
		ParseDistinctions(body, sp, var_mapper)
		
	sp.type = "elaborate"
	facts_cond = sp.get_conditions("facts")[0]
	facts_action = sp.add_action(facts_cond.head_var)
	ParseSentenceToAction(head, sp, facts_action, var_mapper)
	
	return [sp]

#	sp.first_state_cond.add_id_predicate("score")
#	
#	op_name = name_gen.get_name("goal")
#	op_elab = sp.add_op_proposal("+ > =")
#	op_elab.add_ground_wme_action("name", op_name)
#		
#	if head[2][0] == '?':
#		score_var = var_mapper.get_var(head[2])
#		op_elab.add_id_wme_action("score", score_var)
#	else:
#		op_elab.add_ground_wme_action("score", head[2])
#	
#	if body != None:
#		ParseDistinctions(body, sp, var_mapper)
#	
#	ap = sp.get_apply_rules(name_gen)[0]
#	ap.first_state_cond.add_ground_predicate("name", game_name)
#	op_cond = ap.get_conditions("operator")[0]
#	orig_score_var = ap.first_state_cond.add_id_predicate("score")
#	score_inc_var = op_cond.add_id_predicate("score")
#	ap.add_action().remove_id_wme_action("score", orig_score_var)
#	ap.add_action().add_ground_wme_action("score", "(+ <%s> <%s>)" % (orig_score_var, score_inc_var))
#	
#	return [sp, ap]


def MangleVars(s, mangled, name_gen):
	for i in range(len(s)):
		if isinstance(s[i], str) and s[i][0] == '?':
			if mangled.has_key(s[i]):
				s[i] = mangled[s[i]]
			else:
				mangled[s[i]] = name_gen.get_name(s[i])
				s[i] = mangled[s[i]]
		elif isinstance(s[i], ElemGGP):
			MangleVars(s[i], mangled, name_gen)

def GenCombinations(bounds, index = 0):
	if index == len(bounds) - 1:
		result = []
		for x in range(bounds[index]):
			result.append([x])
		return result

	result = []
	ends = GenCombinations(bounds, index + 1)
	for x in range(bounds[index]):
		result.extend([ [x] + e for e in ends ])
	
	return result

# head should be of the form (next (rel p1 p2 ...))
#
#
def TranslateFrameAxioms(game_name, head, bodies):
	productions = []

	axiom_name = SoarifyStr(str(head))
	frame_literal = head.deep_copy()
	frame_literal[0] = "true" # instead of next

	distinct_literals = []
	new_bodies = []
	# eliminate frame conditions from body
	# negate each condition in each body
	for b in bodies:
		newb = []
		for c in b:
			if c != frame_literal:
				if c[0].lower() == "distinct":
					# preserve distinct
					newb.append(c)
				else:
					if c[0].lower() == "not":
						# not not cancels to nothing
						newb.append(c[1])
					else:
						newb.append(ElemGGP("(not)") + c)
		new_bodies.append(newb)

	# mangle variables in each body to prevent name collisions
	name_gen = UniqueNameGenerator()
	for b in new_bodies:
		mangled = dict()
		for c in b:
			MangleVars(c, mangled, name_gen)


	# collect all distinctions
	for b in new_bodies:
		i = 0
		while i < len(b):
			if b[i][0].lower() == "distinct":
				distinct_literals.append(b[i])
				b.pop(i)
			else:
				i += 1

	# take all possible combinations of one condition from each body
	#   (this is the same as ANDing all bodies together and distributing
	#    ANDs over ORs [ the ORs resulted from distributing negations
	#    into the ANDs implicit in each body])

	x = 0
	combinations = GenCombinations([len(b) for b in new_bodies])
	props = []
	op_name = name_gen.get_name("remove-%s" % axiom_name)
	for ci in combinations:
		x += 1
		comb = []
		for i in range(len(ci)):
			comb.append(new_bodies[i][ci[i]])

		comb_body = ElementGGP.Combine(comb + distinct_literals)
		prod_name = name_gen.get_name("%s%d" % (op_name, x))
		sp = MakeTemplateProduction("propose", prod_name, game_name)
		var_mapper = GDLSoarVarMapper(sp.var_gen)
		gs_cond = sp.get_conditions("gs")[0]
		fact_cond = sp.get_conditions("facts")[0]
		move_cond = sp.get_conditions("io.input-link.last-moves")[0]

		# first, add the condition to check for presence of head relation
		ParseGDLBodyToCondition([frame_literal], sp, gs_cond, fact_cond, move_cond, var_mapper)
		
		rel_name = frame_literal[1][0]
		rel_cond = sp.get_conditions("gs.%s" % rel_name)[0]
		rel_var = rel_cond.head_var

		# now add the conditions sufficient for removing the head relation from the
		# next state
		ParseGDLBodyToCondition(comb_body, sp, gs_cond, fact_cond, move_cond, var_mapper)

		# propose the operator
		op_elab = sp.add_op_proposal("+ > =")
		op_elab.add_ground_wme_action("name", op_name)
		op_elab.add_id_wme_action("remove")
		op_elab.add_id_wme_action("relation", rel_var)

		# set up the variable distinctions
		ParseDistinctions(comb_body, sp, var_mapper)
		props.append(sp)

		productions.append(sp)

	# create the apply rule, there is only one
	ap = props[0].get_apply_rules(name_gen)[0]
	ap.first_state_cond.add_ground_predicate("name", game_name)
	gs_var = ap.first_state_cond.add_id_predicate("gs")
	op_cond = ap.get_conditions("operator")[0]
	rel_var = op_cond.add_id_predicate("relation")
	ap.add_action(gs_var).remove_id_wme_action(head[1][0], rel_var)

	productions.append(ap)

	return productions


def MakeSelectionSpaceFakeResponseRule(game_name, role):
	sp = SoarProduction(name_gen.get_name("%s*elaborate*fake-response" % game_name))
	sp.add_condition().add_id_predicate("duplicate-of*")
	act_var = sp.get_ol_cond().add_id_predicate("<action-name>")
	sp.add_condition(act_var)
	
	il_var = sp.get_il_cond().head_var
	lm_var = sp.add_action(il_var).add_id_wme_action("last-moves")
	role_var = sp.add_action(lm_var).add_id_wme_action(role)
	sp.add_action(role_var).add_id_wme_action("<action-name>", act_var)

	return sp

def MakeSelectionSpaceRules(game_name, tokens):
	problem_space_sp = SoarProduction("%s*elaborate*problem-space" % game_name)
	s_cond = problem_space_sp.add_condition()
	s_cond.add_ground_predicate("name", game_name)

	ps_var = problem_space_sp.add_action(s_cond.head_var).add_id_wme_action("problem-space")
	ps_act = problem_space_sp.add_action(ps_var)
	ps_act.add_ground_wme_action("name", game_name)
	ps_act.add_ground_wme_action("default-state-copy", "yes")
	ps_act.add_ground_wme_action("one-level-attributes", "facts")
#	ps_act.add_ground_wme_action("one-level-attributes", "score")
	ps_act.add_ground_wme_action("two-level-attributes", "gs")

	for t in tokens:
		ps_act.add_ground_wme_action("one-level-attributes", t)


	fake_io_sp = SoarProduction("%s*elaborate*selection-space-fake-io" % game_name)
	s_cond = fake_io_sp.add_condition()
	s_cond.add_id_predicate("duplicate-of*")

	fake_io_var = fake_io_sp.add_action(s_cond.head_var).add_id_wme_action("io")
	io_act = fake_io_sp.add_action(fake_io_var)
	io_act.add_id_wme_action("input-link")
	io_act.add_id_wme_action("output-link")

	return [problem_space_sp, fake_io_sp]

def StandardizeVars(literal, prefix, map = dict(), count = 0):
	for i in range(len(literal)):
		c = literal[i]
		if isinstance(c, str):
			if c[0] == '?':
				if not map.has_key(c):
					count += 1
					map[c] = count
				literal[i] = "?%s%d" % (prefix, map[c])
		else:
			count = StandardizeVars(literal[i], prefix, map, count)

	return count

def TranslateDescription(game_name, description, filename):
	productions = []
	init_axioms = []
	fact_axioms = []
	tokens = []
	implications = []
	rels_with_frame_axioms = set([])
	role = ""

	best_score = -99999

	new_description = []
	for axiom in description:
		if axiom.or_child() != None:
			new_description.extend(SplitOr(axiom))
		else:
			new_description.append(axiom)
		
	frame_axioms = [] # [(head, body)]
	for axiom in new_description:

		if axiom[0] == "<=":
			head = axiom[1]
			body = [axiom[i] for i in range(2, len(axiom))]

			# check to see if this is a frame axiom
			if head[0].lower() == "next":
				frame_literal = head.deep_copy()
				frame_literal[0] = "true"
				if frame_literal in body:
					# this is a frame axiom, save it for later
					frame_axioms.append((head, body))
					rels_with_frame_axioms.add(frame_literal[1][0])
					continue
			
			if head[0].lower() == "goal":
				if head[2][0] != '?' and int(head[2]) > best_score:
					best_score = int(head[2])
			
			# just a regular implication
			implications.append(axiom)

		elif axiom[0].lower() == "role":
			role = axiom[1]
		elif axiom[0].lower() == "init":
			init_axioms.append(axiom[1])
		elif axiom[0].lower() == "next":
			head = axiom
			name = name_gen.get_name(SoarifyStr(str(head)))
			sp = MakeTemplateProduction("propose", name, game_name, True, False, False)
			productions.extend(TranslateNext(game_name, sp, head, None))

		elif axiom[0].lower() == "legal":
			head = axiom
			name = name_gen.get_name(SoarifyStr(str(head)))
			sp = MakeTemplateProduction("propose", name, game_name, False, False, False)
			productions.extend(TranslateLegal(game_name, sp, head, None))

		else: # these should all be facts
			fact_axioms.append(axiom)

	# Since we can't really tell what the best possible score will be, rely on a
	# heuristic that says that we've succeeded if we obtain a score higher than or equal
	# to the single highest reward
	if best_score == -99999:
		raise Exception("There are no goal axioms")
	
	for axiom in implications:
		head = axiom[1]
		body = [axiom[i] for i in range(2, len(axiom))]
		if head[1][0] in rels_with_frame_axioms:
			productions.extend(TranslateImplication(game_name, head, body, tokens, False, best_score))
		else:
			productions.extend(TranslateImplication(game_name, head, body, tokens, True, best_score))

	# get the init rules
#	desired_sp = SoarProduction("%s*elaborate*goal-achieved" % game_name)
#	s_cond = desired_sp.add_condition()
#	s_cond.add_ground_predicate("name", game_name)
#	s_cond.add_ground_predicate("score", ">= %d" % best_score)
#	desired_sp.add_action().add_id_wme_action("achieved-score")
#	productions.append(desired_sp)
	productions.extend(MakeInitRule(game_name, role, init_axioms, fact_axioms, best_score, tokens))
	
	
	# process the frame axioms

	# First, we have to merge those frame axioms whose heads are identical except for variable names.
	# We do this by constructing an N x N matrix M, where N is the number of variables in the head,
	# and M[i][j] = 1 if the ith and jth variables are specified to be distinct

	var_prefix = "std_soar_var"
	std_frame_axioms = []
	for f in frame_axioms:
		# standardize all variable names. This is so that if two head literals are the same, they will
		# match string-wise
		head = f[0]
		body = f[1]
		var_map = dict()
		count = 0
		count = StandardizeVars(head, var_prefix, var_map, count)
		for b in body:
			tmp_count = StandardizeVars(b, var_prefix, var_map, count)

		# create the matrix
		count = 0
		var_map = dict()
		for token in head[1]:
			if token[0] == '?':
				var_map[token] = count
				count += 1
	
		m = [[0] * count for i in range(count)]

		for b in body:
			if b[0].lower() == "distinct":
				if var_map.has_key(b[1]) and var_map.has_key(b[2]):
					m[var_map[b[1]]][var_map[b[2]]] = 1
		
		std_frame_axioms.append((head, [body], m))

	# go through the list, finding the repetitions
	i = 0
	while i < len(std_frame_axioms):
		j = i + 1
		while j < len(std_frame_axioms):
			h1 = std_frame_axioms[i][0]
			m1 = std_frame_axioms[i][2]
			h2 = std_frame_axioms[j][0]
			m2 = std_frame_axioms[j][2]

			if h1 == h2 and m1 == m2:
				# these two are the same, so we have to merge them together
				std_frame_axioms[i][1].extend(std_frame_axioms[j][1])

				#remove the merged entry
				std_frame_axioms.pop(j)
				continue

			j += 1
		i += 1

	# merge the frame axioms into production rules
	for f in std_frame_axioms:
		productions.extend(TranslateFrameAxioms(game_name, f[0], f[1]))

	# make the progress state productions
	sp = SoarProduction("progress-state", "propose")
	sp.get_ol_cond().add_id_predicate("<cmd-name>", name="cmd")
	sp.add_op_proposal("+ <").add_ground_wme_action("name", "progress-state")
	productions.append(sp)

	ap_cmd_remove = sp.get_apply_rules(name_gen)[0]
	ap_cmd_remove.first_state_cond.add_ground_predicate("name", game_name)
	ol_cmd_var = ap_cmd_remove.get_ol_cond().add_id_predicate("<cmd-name>")
#	last_move_var = ap_cmd_remove.get_il_cond().add_id_predicate("last-moves")
#	il_cmd_var = ap_cmd_remove.add_condition(last_move_var).add_id_predicate("<last-cmd>")
	remove_cmd = ap_cmd_remove.add_action(ap_cmd_remove.get_ol_cond().head_var)
	remove_cmd.remove_id_wme_action("<cmd-name>", ol_cmd_var)
#	ap_cmd_remove.add_action(last_move_var).remove_id_wme_action("<last-cmd>", il_cmd_var)
	productions.append(ap_cmd_remove)

	for t in tokens:
		# add an apply rule for each token
		ap = sp.get_apply_rules(name_gen)[0]
		ap.first_state_cond.add_ground_predicate("name", game_name)
		token_var = ap.first_state_cond.add_id_predicate(t, negate = True)
		ap.add_action().add_id_wme_action(t, token_var)
		productions.append(ap)
	
	ap_new_remove = sp.get_apply_rules(name_gen)[0]
	gs_cond = ap_new_remove.add_condition(ap_new_remove.first_state_cond.add_id_predicate("gs"))
	rel_var = gs_cond.add_id_predicate("<rel-name>")
	new_var = ap_new_remove.add_condition(rel_var).add_id_predicate("new")
	ap_new_remove.add_action(rel_var).remove_id_wme_action("new", new_var)
	productions.append(ap_new_remove)

	# create selection space rules
	productions.append(MakeSelectionSpaceFakeResponseRule(game_name, role))
	productions.extend(MakeSelectionSpaceRules(game_name, tokens))
	
	# create rule that makes soar prefer adding relations over removing them
	# so that all relations will be added before any are removed
	sp = SoarProduction("prefer-add-over-remove")
	o1_var = sp.add_condition().add_id_predicate("operator", predicate="+")
	o2_var = sp.add_condition().add_id_predicate("operator", predicate="+")
	o1_cond = sp.add_condition(o1_var)
	o2_cond = sp.add_condition(o2_var)
	o1_cond.add_id_predicate("add")
	o2_cond.add_id_predicate("remove")
	
	sp.add_action().add_ground_wme_action("operator", "<%s> > <%s>" % (o1_var, o2_var))
	productions.append(sp)

	f = open(filename, 'w')
	f.write("pushd default\nsource selection.soar\npopd\n")

	for p in productions:
		f.write(str(p) + '\n')
	

def DeleteOrOperands(axiom, keep):
	if isinstance(axiom[0], str) and axiom[0].lower() == "or":
		return axiom[keep]

	for i in range(len(axiom)):
		c = axiom[i]
		if isinstance(c, ElemGGP):
			ret = DeleteOrOperands(c, keep)
			if ret != None:
				axiom[i] = ret
				return axiom

	return None

def SplitOr(axiom):
	result = []
	num_splits = len(axiom.or_child()) - 1
	for i in range(num_splits):
		a = axiom.deep_copy()
		result.append(DeleteOrOperands(a, i + 1))

	return result

#blocksworld
#d="((ROLE ROBOT) (INIT (CLEAR B)) (INIT (CLEAR C)) (INIT (ON C A)) (INIT (TABLE A)) (INIT (TABLE B)) (INIT (STEP 1)) (<= (NEXT (ON ?X ?Y)) (DOES ROBOT (S ?X ?Y))) (<= (NEXT (ON ?X ?Y)) (DOES ROBOT (S ?U ?V)) (TRUE (ON ?X ?Y))) (<= (NEXT (TABLE ?X)) (DOES ROBOT (S ?U ?V)) (TRUE (TABLE ?X)) (DISTINCT ?U ?X)) (<= (NEXT (CLEAR ?Y)) (DOES ROBOT (S ?U ?V)) (TRUE (CLEAR ?Y)) (DISTINCT ?V ?Y)) (<= (NEXT (ON ?X ?Y)) (DOES ROBOT (U ?U ?V)) (TRUE (ON ?X ?Y)) (DISTINCT ?U ?X)) (<= (NEXT (TABLE ?X)) (DOES ROBOT (U ?X ?Y))) (<= (NEXT (TABLE ?X)) (DOES ROBOT (U ?U ?V)) (TRUE (TABLE ?X))) (<= (NEXT (CLEAR ?Y)) (DOES ROBOT (U ?X ?Y))) (<= (NEXT (CLEAR ?X)) (DOES ROBOT (U ?U ?V)) (TRUE (CLEAR ?X))) (<= (NEXT (STEP ?Y)) (TRUE (STEP ?X)) (SUCC ?X ?Y)) (SUCC 1 2) (SUCC 2 3) (SUCC 3 4) (<= (LEGAL ROBOT (S ?X ?Y)) (TRUE (CLEAR ?X)) (TRUE (TABLE ?X)) (TRUE (CLEAR ?Y)) (DISTINCT ?X ?Y)) (<= (LEGAL ROBOT (U ?X ?Y)) (TRUE (CLEAR ?X)) (TRUE (ON ?X ?Y))) (<= (GOAL ROBOT 100) (TRUE (ON A B)) (TRUE (ON B C))) (<= (GOAL ROBOT 0) (NOT (TRUE (ON A B)))) (<= (GOAL ROBOT 0) (NOT (TRUE (ON B C)))) (<= TERMINAL (TRUE (STEP 4))) (<= TERMINAL (TRUE (ON A B)) (TRUE (ON B C))))"

#buttons
d="((ROLE ROBOT) (INIT (OFF P)) (INIT (OFF Q)) (INIT (OFF R)) (INIT (STEP 1)) (<= (NEXT (ON P)) (DOES ROBOT A) (TRUE (OFF P))) (<= (NEXT (ON Q)) (DOES ROBOT A) (TRUE (ON Q))) (<= (NEXT (ON R)) (DOES ROBOT A) (TRUE (ON R))) (<= (NEXT (OFF P)) (DOES ROBOT A) (TRUE (ON P))) (<= (NEXT (OFF Q)) (DOES ROBOT A) (TRUE (OFF Q))) (<= (NEXT (OFF R)) (DOES ROBOT A) (TRUE (OFF R))) (<= (NEXT (ON P)) (DOES ROBOT B) (TRUE (ON Q))) (<= (NEXT (ON Q)) (DOES ROBOT B) (TRUE (ON P))) (<= (NEXT (ON R)) (DOES ROBOT B) (TRUE (ON R))) (<= (NEXT (OFF P)) (DOES ROBOT B) (TRUE (OFF Q))) (<= (NEXT (OFF Q)) (DOES ROBOT B) (TRUE (OFF P))) (<= (NEXT (OFF R)) (DOES ROBOT B) (TRUE (OFF R))) (<= (NEXT (ON P)) (DOES ROBOT C) (TRUE (ON P))) (<= (NEXT (ON Q)) (DOES ROBOT C) (TRUE (ON R))) (<= (NEXT (ON R)) (DOES ROBOT C) (TRUE (ON Q))) (<= (NEXT (OFF P)) (DOES ROBOT C) (TRUE (OFF P))) (<= (NEXT (OFF Q)) (DOES ROBOT C) (TRUE (OFF R))) (<= (NEXT (OFF R)) (DOES ROBOT C) (TRUE (OFF Q))) (<= (NEXT (STEP ?Y)) (TRUE (STEP ?X)) (SUCC ?X ?Y)) (SUCC 1 2) (SUCC 2 3) (SUCC 3 4) (SUCC 4 5) (SUCC 5 6) (SUCC 6 7) (LEGAL ROBOT A) (LEGAL ROBOT B) (LEGAL ROBOT C) (<= (GOAL ROBOT 100) (TRUE (ON P)) (TRUE (ON Q)) (TRUE (ON R))) (<= (GOAL ROBOT 0) (OR (NOT (TRUE (ON P))) (NOT (TRUE (ON Q))) (NOT (TRUE (ON R))))) (<= TERMINAL (TRUE (STEP 7))) (<= TERMINAL (TRUE (ON P)) (TRUE (ON Q)) (TRUE (ON R))))"

#maze
#d="((ROLE ROBOT) (INIT (CELL A)) (INIT (GOLD C)) (INIT (STEP 1)) (<= (NEXT (CELL ?Y)) (DOES ROBOT MOVE) (TRUE (CELL ?X)) (ADJACENT ?X ?Y)) (<= (NEXT (CELL ?X)) (DOES ROBOT GRAB) (TRUE (CELL ?X))) (<= (NEXT (CELL ?X)) (DOES ROBOT DROP) (TRUE (CELL ?X))) (<= (NEXT (GOLD ?X)) (DOES ROBOT MOVE) (TRUE (GOLD ?X))) (<= (NEXT (GOLD I)) (DOES ROBOT GRAB) (TRUE (CELL ?X)) (TRUE (GOLD ?X))) (<= (NEXT (GOLD I)) (DOES ROBOT GRAB) (TRUE (GOLD I))) (<= (NEXT (GOLD ?Y)) (DOES ROBOT GRAB) (TRUE (CELL ?X)) (TRUE (GOLD ?Y)) (DISTINCT ?X ?Y)) (<= (NEXT (GOLD ?X)) (DOES ROBOT DROP) (TRUE (CELL ?X)) (TRUE (GOLD I))) (<= (NEXT (GOLD ?X)) (DOES ROBOT DROP) (TRUE (GOLD ?X)) (DISTINCT ?X I)) (<= (NEXT (STEP ?Y)) (TRUE (STEP ?X)) (SUCC ?X ?Y)) (ADJACENT A B) (ADJACENT B C) (ADJACENT C D) (ADJACENT D A) (SUCC 1 2) (SUCC 2 3) (SUCC 3 4) (SUCC 4 5) (SUCC 5 6) (SUCC 6 7) (SUCC 7 8) (SUCC 8 9) (SUCC 9 10) (<= (LEGAL ROBOT MOVE)) (<= (LEGAL ROBOT GRAB) (TRUE (CELL ?X)) (TRUE (GOLD ?X))) (<= (LEGAL ROBOT DROP) (TRUE (GOLD I))) (<= (GOAL ROBOT 100) (TRUE (GOLD A))) (<= (GOAL ROBOT 0) (TRUE (GOLD ?X)) (DISTINCT ?X A)) (<= TERMINAL (TRUE (STEP 10))) (<= TERMINAL (TRUE (GOLD A))))"

#hanoi
#d="((ROLE PLAYER) (INIT (ON DISC5 PILLAR1)) (INIT (ON DISC4 DISC5)) (INIT (ON DISC3 DISC4)) (INIT (ON DISC2 DISC3)) (INIT (ON DISC1 DISC2)) (INIT (CLEAR DISC1)) (INIT (CLEAR PILLAR2)) (INIT (CLEAR PILLAR3)) (INIT (STEP S0)) (<= (LEGAL PLAYER (PUTON ?X ?Y)) (TRUE (CLEAR ?X)) (TRUE (CLEAR ?Y)) (SMALLERDISC ?X ?Y)) (<= (NEXT (STEP ?Y)) (TRUE (STEP ?X)) (SUCCESSOR ?X ?Y)) (<= (NEXT (ON ?X ?Y)) (DOES PLAYER (PUTON ?X ?Y))) (<= (NEXT (ON ?X ?Y)) (TRUE (ON ?X ?Y)) (NOT (DOES PLAYER (PUTON ?X ?Y1))) (DISC ?Y1)) (<= (NEXT (CLEAR ?Y)) (TRUE (ON ?X ?Y)) (DOES PLAYER (PUTON ?X ?Y1))) (<= (NEXT (CLEAR ?Y)) (TRUE (CLEAR ?Y)) (NOT (DOES PLAYER (PUTON ?X ?Y))) (DISC ?X)) (<= (GOAL PLAYER 100) (TOWER PILLAR3 S5)) (<= (GOAL PLAYER 80) (TOWER PILLAR3 S4)) (<= (GOAL PLAYER 60) (TOWER PILLAR3 S3)) (<= (GOAL PLAYER 40) (TOWER PILLAR3 S2)) (<= (GOAL PLAYER 0) (TOWER PILLAR3 ?HEIGHT) (SMALLER ?HEIGHT S2)) (<= TERMINAL (TRUE (STEP S31))) (<= (TOWER ?X S0) (TRUE (CLEAR ?X))) (<= (TOWER ?X ?HEIGHT) (TRUE (ON ?Y ?X)) (DISC ?Y) (TOWER ?Y ?HEIGHT1) (SUCCESSOR ?HEIGHT1 ?HEIGHT)) (PILLAR PILLAR1) (PILLAR PILLAR2) (PILLAR PILLAR3) (NEXTSIZE DISC1 DISC2) (NEXTSIZE DISC2 DISC3) (NEXTSIZE DISC3 DISC4) (NEXTSIZE DISC4 DISC5) (DISC DISC1) (DISC DISC2) (DISC DISC3) (DISC DISC4) (DISC DISC5) (<= (NEXTSIZE DISC5 ?PILLAR) (PILLAR ?PILLAR)) (<= (SMALLERDISC ?A ?B) (NEXTSIZE ?A ?B)) (<= (SMALLERDISC ?A ?B) (NEXTSIZE ?A ?C) (SMALLERDISC ?C ?B)) (SUCCESSOR S0 S1) (SUCCESSOR S1 S2) (SUCCESSOR S2 S3) (SUCCESSOR S3 S4) (SUCCESSOR S4 S5) (SUCCESSOR S5 S6) (SUCCESSOR S6 S7) (SUCCESSOR S7 S8) (SUCCESSOR S8 S9) (SUCCESSOR S9 S10) (SUCCESSOR S10 S11) (SUCCESSOR S11 S12) (SUCCESSOR S12 S13) (SUCCESSOR S13 S14) (SUCCESSOR S14 S15) (SUCCESSOR S15 S16) (SUCCESSOR S16 S17) (SUCCESSOR S17 S18) (SUCCESSOR S18 S19) (SUCCESSOR S19 S20) (SUCCESSOR S20 S21) (SUCCESSOR S21 S22) (SUCCESSOR S22 S23) (SUCCESSOR S23 S24) (SUCCESSOR S24 S25) (SUCCESSOR S25 S26) (SUCCESSOR S26 S27) (SUCCESSOR S27 S28) (SUCCESSOR S28 S29) (SUCCESSOR S29 S30) (SUCCESSOR S30 S31) (<= (SMALLER ?X ?Y) (SUCCESSOR ?X ?Y)) (<= (SMALLER ?X ?Y) (SUCCESSOR ?X ?Z) (SMALLER ?Z ?Y)))"

TranslateDescription("game", ElemGGP(d), "tmp")
