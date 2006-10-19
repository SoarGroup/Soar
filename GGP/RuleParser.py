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
def MakeTemplateProduction(name, game_name, gs = True, facts = True, move = True):
	p = SoarProduction(name)
	s_cond = p.add_condition()
	s_cond.add_ground_predicate("name", game_name)
	if gs:
		p.add_condition(s_cond.add_id_predicate("gs"))
	
	if facts:
		p.add_condition(s_cond.add_id_predicate("facts"))

	if move:
		p.add_condition(p.get_il_cond().add_id_predicate("last-moves"))

	return p


def ParseSentenceToAction(sentence, production, action):
	id = action.add_id_wme_action(sentence[0])
	id_action = production.add_action(id)
	for i in range(1, len(sentence)):
		attrib_name = "p%d" % i
		id_action.add_ground_wme_action(attrib_name, sentence[i])

def ParseSentenceToCondition(sentence, production, condition, var_mapper, negated = False):

	if len(sentence) == 1:
		# we don't have chained conditions, so we can just check for absence
		# of attribute
		id = condition.add_id_predicate(sentence[0], negated)
		id_cond = production.add_condition(id)
	else:
		# we have to check for absence of specific id match for attribute
		id = condition.add_id_predicate(sentence[0])
		id_cond = production.add_condition(id)
		if negated:
			production.make_negative_conjunction([condition, id_cond])

	for i in range(1, len(sentence)):
		attrib_name = "p%d" % i
		if sentence[i][0] == '?':
			# match on a variable
			id_cond.add_id_predicate(attrib_name, name=var_mapper.get_var(sentence[i]))

		else:
			# match a literal
			id_cond.add_ground_predicate(attrib_name, sentence[i])
	
def ParseGDLBodyToCondition(body, prod, gs_cond, fact_cond, move_cond, var_mapper):
	"""
	Parse a set of literals and build the appropriate conditions on the
	specified soar production
	"""
	negate = False
	for b in body:
		if b[0] == "not":
			c = b[1]
			negate = True
		else:
			c = b
			
		if c[0] == "true":
			if negate and len(c[1]) > 1:
				# we have to make a new ^gs condition for the negation
				new_gs_cond = prod.add_condition(gs_cond.head_var)
			else:
				new_gs_cond = gs_cond

			ParseSentenceToCondition(c[1], prod, new_gs_cond, var_mapper, negate)

		elif c[0] == "does":
			role = c[1]
			# there should always only be one last move for each role. If the condition
			# for it already exists, use it. Otherwise create a new one
			role_cond_list = prod.get_conditions("io.input-link.last-moves.%s" % role)
			if len(role_cond_list) == 0:
				role_cond = prod.add_condition(move_cond.add_id_predicate(role))
			else:
				if negate and len(c[2]) > 0:
					# we want to use the same id, but create another condition
					role_cond = prod.add_condition(role_cond_list[0].head_var)
				else:
					role_cond = role_cond_list[0]
			ParseSentenceToCondition(c[2], prod, role_cond, var_mapper, negate)

		elif c[0] != "distinct": # must be a fact
			if negate and len(c) > 1:
				new_fact_cond = prod.add_condition(fact_cond.head_var)
			else:
				new_fact_cond = fact_cond
			ParseSentenceToCondition(c, prod, new_fact_cond, var_mapper, negate)

def ParseDistinctions(literals, prod, var_mapper):
	for l in literals:
		if l[0] == "distinct":
			v1 = var_mapper.get_var(l[1])
			v2 = var_mapper.get_var(l[2])
			prod.add_var_distinction(v1, v2)


def MakeInitRule(game_name, role, init_conds, facts):
	sp = SoarProduction("propose*init-%s" % game_name);
	c = sp.add_condition()
	c.add_ground_predicate("superstate", "nil")
	c.add_ground_predicate("name", "", True)
	op = sp.add_action().add_op_proposal("+ >")
	sp.add_action(op).add_ground_wme_action("name", "init-blocksworld")

	asp = sp.get_apply_rules(name_gen)[0]

	state_action = asp.add_action()
	state_action.add_ground_wme_action("name", game_name)
	game_state_var = state_action.add_id_wme_action("gs")
	game_facts_var = state_action.add_id_wme_action("facts")
	gs_actions = asp.add_action(game_state_var)
	gs_actions.add_ground_wme_action("role", role)

	for ic in init_conds:
		ParseSentenceToAction(ic, asp, gs_actions)

	fact_actions = asp.add_action(game_facts_var)
	for f in facts:
		ParseSentenceToAction(f, asp, fact_actions)

	return [sp, asp]


def SoarifyStr(s):
	p = re.compile('(\s|\?|\(|\))')
	result = p.sub('_', str(s))
	for i in range(len(result)):
		if result[i] != '_':
			return result[i:]
	
	# all underscores?
	return "a"


# Process axioms of the form
#
# (<= (head)
#     (body1)
#     (body2)
#     ...
# )
def TranslateImplication(game_name, head, body, tokens, remove):
	prod_name = name_gen.get_name(SoarifyStr(head))

	use_move = False
	use_gs = False
	use_facts = False
	for b in body:
		if b[0] == "not":
			relation = b[1][0]
		else:
			relation = b[0]

		if relation == "does":
			use_move = True
		elif relation == "true":
			use_gs = True
		else:
			use_facts = True
	
	if head[0] == "next":
		use_gs = True # because we have to check for lack of it later

	sp = MakeTemplateProduction(prod_name, game_name, use_gs, use_facts, use_move)
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

	ParseGDLBodyToCondition(body, sp, gs_cond, fact_cond, move_cond, var_mapper)

	if isinstance(head, str):
		if head == "terminal":
			ParseDistinctions(body, sp, var_mapper)
			sp.add_halt()
			return [sp]
	else:
		if head[0] == "legal":
			return TranslateLegal(game_name, sp, head, body, var_mapper)
		elif head[0] == "next":
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

	# doesn't match anything
	return []

def TranslateLegal(game_name, sp, head, body, var_mapper):

	# make sure there's no command
	sp.get_ol_cond().add_id_predicate("<cmd-name>", negate=True, name="cmd")

	params = list(head[2])[1:]
	role = head[1]
	move = head[2][0]
	op_name = "%s-%s" % (role, move)

	op_elab = sp.add_op_proposal("+ < =")
	op_elab.add_ground_wme_action("name", op_name)
	for i in range(len(params)):
		if params[i][0] == '?':
			op_elab.add_id_wme_action("p%d" % (i+1), var_mapper.get_var(params[i]))
		else:
			op_elab.add_ground_wme_action("p%d" % (i+1), params[i])
	
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

def TranslateNext(game_name, sp, head, body, var_mapper, remove, token = ""):
	rel_name = head[1][0]
	op_name = "next-%s" % rel_name
	params = list(head[1])[1:]

	# have to add condition to check for lack of head relation to not have operator no change
	negated_head = ElemGGP("(not)") + head.deep_copy()
	negated_head[1][0] = "true"

	gs_cond = sp.get_conditions("gs")[0]
	ParseGDLBodyToCondition([negated_head], sp, gs_cond, None, None, var_mapper)

	op_elab = sp.add_op_proposal("+ > =")
	op_elab.add_ground_wme_action("name", op_name)
	for i in range(len(params)):
		if params[i][0] == '?':
			op_elab.add_id_wme_action("p%d" % (i+1), var_mapper.get_var(params[i]))
		else:
			op_elab.add_ground_wme_action("p%d" % (i+1), params[i])
	
	if token != "":
		sp.first_state_cond.add_id_predicate(token)

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
	for i in range(len(param_vars)):
		next_act.add_id_wme_action("p%d" % (i+1), param_vars[i])

	if token != "":
		token_var = ap.first_state_cond.add_id_predicate(token)
		ap.add_action().remove_id_wme_action(token, token_var)
	
	if remove:
		# we have to create another apply rule that removes the old instance
		ap_copy = ap.copy(name_gen.get_name(ap.name))
		ap.add_condition(gs_var).add_id_predicate(rel_name, negate=True)
		old_rel_var = ap_copy.add_condition(gs_var).add_id_predicate(rel_name)
		ap_copy.add_action(gs_var).remove_id_wme_action(rel_name, old_rel_var)
		return [sp, ap, ap_copy]

	return [sp, ap]

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
				if c[0] == "distinct":
					# preserve distinct
					newb.append(c)
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
			if b[i][0] == "distinct":
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
	for ci in combinations:
		x += 1
		comb = []
		for i in range(len(ci)):
			comb.append(new_bodies[i][ci[i]])

		comb_body = ElementGGP.Combine(comb + distinct_literals)
		
		op_name = "remove-%s-%d" % (axiom_name,x)
		sp = MakeTemplateProduction("propose*%s" % op_name, game_name)
		var_mapper = GDLSoarVarMapper(sp.var_gen)
		gs_cond = sp.get_conditions("gs")[0]
		fact_cond = sp.get_conditions("facts")[0]
		move_cond = sp.get_conditions("io.input-link.last-moves")[0]

		# first, add the condition to check for presence of head relation
		ParseGDLBodyToCondition([frame_literal], sp, gs_cond, fact_cond, move_cond, var_mapper)

		head_relation_var = sp.get_conditions("gs.%s" % head[1][0])[0].head_var

		# now add the conditions sufficient for removing the head relation from the
		# next state
		ParseGDLBodyToCondition(comb_body, sp, gs_cond, fact_cond, move_cond, var_mapper)

		# propose the operator
		op_elab = sp.add_op_proposal("+ > =")
		op_elab.add_ground_wme_action("name", op_name)
		op_elab.add_id_wme_action("relation", head_relation_var)

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

def TranslateDescription(game_name, description):
	productions = []
	init_axioms = []
	fact_axioms = []
	tokens = []
	implications = []
	rels_with_frame_axioms = set([])
	role = ""

	frame_axioms = [] # [(head, body)]
	for axiom in description:

		if axiom[0] == "<=":
			head = axiom[1]
			body = [axiom[i] for i in range(2, len(axiom))]

			# check to see if this is a frame axiom
			if head[0] == "next":
				frame_literal = head.deep_copy()
				frame_literal[0] = "true"
				if frame_literal in body:
					# this is a frame axiom, save it for later
					frame_axioms.append((head, body))
					rels_with_frame_axioms.add(frame_literal[1][0])
					continue

			implications.append(axiom)

		elif axiom[0] == "role":
			role = axiom[1]
		elif axiom[0] == "init":
			init_axioms.append(axiom[1])
		else: # these should all be facts
			fact_axioms.append(axiom)

	for axiom in implications:
		head = axiom[1]
		body = [axiom[i] for i in range(2, len(axiom))]
		if head[1][0] in rels_with_frame_axioms:
			productions.extend(TranslateImplication(game_name, head, body, tokens, False))
		else:
			productions.extend(TranslateImplication(game_name, head, body, tokens, True))

	# get the init rules
	productions.extend(MakeInitRule(game_name, role, init_axioms, fact_axioms))

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
			if b[0] == "distinct":
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
	sp = SoarProduction("propose*progress-state")
	sp.get_ol_cond().add_id_predicate("<cmd-name>", name="cmd")
	sp.add_op_proposal("+ <").add_ground_wme_action("name", "progress-state")
	productions.append(sp)

	ap_cmd_remove = sp.get_apply_rules(name_gen)[0]
	ap_cmd_remove.first_state_cond.add_ground_predicate("name", game_name)
	cmd_var = ap_cmd_remove.get_ol_cond().add_id_predicate("<cmd-name>", name="cmd")
	remove_cmd = ap_cmd_remove.add_action(ap_cmd_remove.get_ol_cond().head_var)
	remove_cmd.remove_id_wme_action("<cmd-name>", cmd_var)
	productions.append(ap_cmd_remove)

	for t in tokens:
		# add an apply rule for each token
		ap = sp.get_apply_rules(name_gen)[0]
		ap.first_state_cond.add_ground_predicate("name", game_name)
		token_var = ap.first_state_cond.add_id_predicate(t, negate = True)
		ap.add_action().add_id_wme_action(t, token_var)
		productions.append(ap)

	for p in productions:
		print p


description="""(
(role robot)
(init (clear b))
(init (clear c))
(init (on c a))
(init (table a))
(init (table b))
(init (step 1))
(<= (next (on ?x ?y))
    (does robot (s ?x ?y)))
(<= (next (on ?x ?y))
    (does robot (s ?u ?v))
    (true (on ?x ?y)))
(<= (next (table ?x))
    (does robot (s ?u ?v))
    (true (table ?x))
    (distinct ?u ?x))
(<= (next (clear ?y))
    (does robot (s ?u ?v))
    (true (clear ?y))
    (distinct ?v ?y))
(<= (next (on ?x ?y))
    (does robot (u ?u ?v))
    (true (on ?x ?y))
    (distinct ?u ?x))
(<= (next (table ?x))
    (does robot (u ?x ?y)))
(<= (next (table ?x))
    (does robot (u ?u ?v))
    (true (table ?x)))
(<= (next (clear ?y))
    (does robot (u ?x ?y)))
(<= (next (clear ?x))
    (does robot (u ?u ?v))
    (true (clear ?x)))
(<= (next (step ?y))
    (true (step ?x))
    (succ ?x ?y))
(succ 1 2)
(succ 2 3)
(succ 3 4)
(<= (legal robot (s ?x ?y))
    (true (clear ?x))
    (true (table ?x))
    (true (clear ?y))
    (distinct ?x ?y))
(<= (legal robot (u ?x ?y))
    (true (clear ?x))
    (true (on ?x ?y)))
(<= (goal robot 100)
    (true (on a b))
    (true (on b c)))
(<= (goal robot 0)
    (not (true (on a b))))
(<= (goal robot 0)
    (not (true (on b c))))
(<= terminal
    (true (step 4)))
(<= terminal
    (true (on a b))
    (true (on b c)))
)"""

TranslateDescription("blocksworld", ElemGGP(description.replace('\n', "")))

