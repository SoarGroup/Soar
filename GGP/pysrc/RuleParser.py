#!/usr/bin/python

from SoarProduction import UniqueNameGenerator
from SoarProduction import SoarProduction
from SoarProduction import SoarifyStr
from ElementGGP import ElementGGP as ElemGGP
from GGPRule import GGPRule
from GGPSentence import GGPSentence
import ElementGGP

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
def MakeTemplateProduction(type, name, game_name, gs = True, move = True):
	p = SoarProduction(name_gen.get_name(name), type)
	s_cond = p.add_condition()
	s_cond.add_ground_predicate("name", game_name)
	if gs:
		p.add_condition(s_cond.add_id_predicate("gs"))

	if move:
		p.add_condition(p.get_il_cond().add_id_predicate("last-moves"))

	return p

	
def ParseGDLBodyToCondition(body, prod, var_map):
	for b in body:
		if b.name() != "distinct":
			b.make_soar_conditions(prod, var_map)


def ParseDistinctions(body, sp, var_map):
	x = 0
	for sentence in body:
		if sentence.name() == "distinct":
			v1 = var_map.get_var(str(sentence.term(0)))
			if sentence.term(1).type() == "variable":
				v2 = var_map.get_var(str(sentence.term(1)))
				sp.add_var_distinction(v1, v2)
			else:
				sp.add_var_distinction(v1, str(sentence.term(1)))


def MakeInitRule(game_name, role, init_conds, fact_rules, min_success_score):
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
#	game_elabs_var = state_action.add_id_wme_action("elaborations")
	gs_actions = asp.add_action(game_state_var)
	gs_actions.add_ground_wme_action("role", role)

	var_map = GDLSoarVarMapper(UniqueNameGenerator())
	for ic in init_conds:
		ic.head().make_soar_actions(asp, var_map)

	for f in fact_rules:
		f.head().make_soar_actions(asp, var_map)
	return [sp, asp]

# Process axioms of the form
#
# (<= (head)
#     (body1)
#     (body2)
#     ...
# )
def TranslateImplication(game_name, rule, min_success_score, make_remove_rule):
	head = rule.head()
	body = rule.body()
	
	if head.name() == "terminal":
		return TranslateTerminal(game_name, head, body)
	elif head.name() == "legal":
		return TranslateLegal(game_name, head, body)
	elif head.name() == "next":
		return TranslateNext(game_name, head, body, make_remove_rule)
	elif head.name() == "goal":
		return TranslateGoal(game_name, head, body, min_success_score)
	else:
		# this can only be some implied relation
		return TranslateImpliedRelation(game_name, head, body)
		

def TranslateImpliedRelation(game_name, head, body):
	sp = SoarProduction(name_gen.get_name(SoarifyStr(str(head))), "elaborate")
	var_map = GDLSoarVarMapper(sp.var_gen)
	if len(body) > 0:
		ParseGDLBodyToCondition(body, sp, var_map)
		ParseDistinctions(body, sp, var_map)
	head.make_soar_actions(sp, var_map)
	return [sp]
	
def TranslateTerminal(game_name, head, body):
	sp = MakeTemplateProduction("elaborate", name_gen.get_name(SoarifyStr(str(head))), game_name)
	var_map = GDLSoarVarMapper(sp.var_gen)
	ParseGDLBodyToCondition(body, sp, var_map)
	ParseDistinctions(body, sp, var_map)
	
	# different actions depending on if we're in the selection space
	# or operating for real
	sp_sel = sp.copy(name_gen.get_name(sp.name))
	sp_sel.first_state_cond.add_id_predicate("duplicate-of*")
	sp_sel.add_action().add_id_wme_action("terminal")
	
	sp.first_state_cond.add_id_predicate("duplicate-of*", negate = True)
	sp.add_halt()
	
	return [sp, sp_sel]

def TranslateLegal(game_name, head, body):
	move = head.term(1).name()
	
	sp = MakeTemplateProduction("propose", name_gen.get_name(SoarifyStr(str(head))), game_name, False, False)
	var_map = GDLSoarVarMapper(sp.var_gen)
	for relation in body:
		if relation.name() != "distinct":
			relation.make_soar_conditions(sp, var_map)
	
	# have to also check that no moves have been made
	ol_cond = sp.get_ol_cond()
	ol_cond.add_id_predicate("<cmd-name>", negate = True)
	
	head.make_soar_actions(sp, var_map)
	ParseDistinctions(body, sp, var_map)
	
	# apply rule

	ap = sp.get_apply_rules(name_gen)[0]
	ap.first_state_cond.add_ground_predicate("name", game_name)
	op_cond = ap.get_conditions("operator")[0]
	ol_var = ap.get_ol_cond().head_var

	ap_var_map = GDLSoarVarMapper(ap.var_gen)
	head.term(1).make_soar_cond_no_id(ap, op_cond, ap_var_map, 1)

	#move_act = ap.add_action(ap.add_action(ol_var).add_id_wme_action(move))
	#head.term(1).make_soar_action_no_id(ap, move_act, ap_var_map)
	head.term(1).make_soar_action(ap, ap.add_action(ol_var), 0, ap_var_map)
	
	return [sp, ap]

def TranslateNext(game_name, head, body, make_remove_rule = True):
	ap = MakeTemplateProduction("apply", name_gen.get_name(SoarifyStr(str(head))), game_name, False, False)
	var_map = GDLSoarVarMapper(ap.var_gen)
	ap.add_context_cond(game_name)
	ap.add_op_cond("progress-state")
	
	for sentence in body:
		if sentence.name() != "distinct":
			sentence.make_soar_conditions(ap, var_map)
	
	head.make_soar_actions(ap, var_map)
	ParseDistinctions(body, ap, var_map)
	
	# there are no frame axioms, so make productions that get rid of this after one
	# step
	if make_remove_rule:
		rap = SoarProduction(name_gen.get_name("remove*%s" % SoarifyStr(str(head))), "apply")
		var_map = GDLSoarVarMapper(rap.var_gen)
		rap.add_context_cond(game_name)
		rap.add_op_cond("progress-state")
		head.make_soar_actions(rap, var_map, remove = True)
		return [ap, rap]
	else:
		return [ap]


def TranslateGoal(game_name, head, body, score):
	sp = MakeTemplateProduction("elaborate", name_gen.get_name(SoarifyStr(str(head))), game_name, False, False)
	var_map = GDLSoarVarMapper(sp.var_gen)
	sp.first_state_cond.add_id_predicate("terminal")
	desired_var = sp.first_state_cond.add_id_predicate("desired")
	
	ParseGDLBodyToCondition(body, sp, var_map)
	ParseDistinctions(body, sp, var_map)
	
	if int(str(head.term(1))) >= score:
		sp.add_action().add_id_wme_action("success", desired_var)
	else:
		sp.add_action().add_id_wme_action("failure", desired_var)

	return [sp]

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
	frame_sentence = head.true_analogue()

	distinct_literals = []
	new_bodies = []
	# eliminate frame conditions from body
	# negate each condition in each body
	for b in bodies:
		newb = []
		for s in b:
			if s != frame_sentence:
				if s.name() == "distinct":
					# preserve distinct
					newb.append(s)
				else:
					newb.append(s.negate())
		new_bodies.append(newb)

	# mangle variables in each body to prevent name collisions
	name_gen = UniqueNameGenerator()
	for b in new_bodies:
		mangled = dict()
		for c in b:
			c.mangle_vars(mangled, name_gen)

	# collect all distinctions
	for b in new_bodies:
		i = 0
		while i < len(b):
			if b[i].name() == "distinct":
				distinct_literals.append(b[i])
				b.pop(i)
			else:
				i += 1

	# take all possible combinations of one condition from each body
	#   (this is the same as ANDing all bodies together and distributing
	#    ANDs over ORs [ the ORs resulted from distributing negations
	#    into the ANDs implicit in each body])

	combinations = GenCombinations([len(b) for b in new_bodies])
	props = []
	op_name = name_gen.get_name("remove-%s" % axiom_name)
	for ci, x in zip(combinations, range(len(combinations))):
		comb = []
		for i in range(len(ci)):
			if new_bodies[i][ci[i]] not in comb:
				comb.append(new_bodies[i][ci[i]])

		comb_body = ElementGGP.Combine(list(comb) + distinct_literals)
		prod_name = name_gen.get_name("%s%d" % (op_name, x))
		sp = MakeTemplateProduction("apply", prod_name, game_name)
		sp.add_op_cond("progress-state")
		var_mapper = GDLSoarVarMapper(sp.var_gen)

		# first, add the condition to check for presence of head relation
		frame_sentence.make_soar_conditions(sp, var_mapper)

		# now add the conditions sufficient for removing the head relation from the
		# next state
		ParseGDLBodyToCondition(comb_body, sp, var_mapper)
		
		frame_sentence.next_analogue().make_soar_actions(sp, var_mapper, remove = True)

		# set up the variable distinctions
		ParseDistinctions(comb_body, sp, var_mapper)

		productions.append(sp)

	return productions


def MakeSelectionSpaceFakeResponseRule(game_name, role):
	sp = SoarProduction(name_gen.get_name("%s*elaborate*fake-response" % game_name))
#	sp.add_condition().add_id_predicate("duplicate-of*")
	act_var = sp.get_ol_cond().add_id_predicate("<action-name>")
	sp.add_condition(act_var)
	
	il_var = sp.get_il_cond().head_var
	lm_var = sp.add_action(il_var).add_id_wme_action("last-moves")
	role_var = sp.add_action(lm_var).add_id_wme_action(role)
	sp.add_action(role_var).add_id_wme_action("<action-name>", act_var)

	return sp

def MakeSelectionSpaceRules(game_name):
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

	fake_io_sp = SoarProduction("%s*elaborate*selection-space-fake-io" % game_name)
	s_cond = fake_io_sp.add_condition()
	s_cond.add_id_predicate("duplicate-of*")

	fake_io_var = fake_io_sp.add_action(s_cond.head_var).add_id_wme_action("io")
	io_act = fake_io_sp.add_action(fake_io_var)
	io_act.add_id_wme_action("input-link")
	io_act.add_id_wme_action("output-link")

	return [problem_space_sp, fake_io_sp]

def StandardizeVars(sentence, prefix, map = dict(), count = 0):
	for i in range(sentence.num_terms()):
		c = sentence.term(i)
		if c.type() == "variable":
			if not map.has_key(str(c)):
				count += 1
				map[str(c)] = count
			sentence.term(i).rename("%s%d" % (prefix, map[str(c)]))
		elif c.type() == "function":
			count = StandardizeVars(c, prefix, map, count)

	return count

def TranslateDescription(game_name, description, filename):
	productions = []
	init_rules = []
	cond_elabs = set([]) 
	uncond_elabs = dict() # rel_name -> [rules]
	implications = []
	funcs_with_frame_rules = []
	role = ""

	best_score = -99999

	# first run through the set of rules, expand all "or" sentences
	new_description = []
	for axiom in description:
		if axiom.or_child() != None:
			new_description.extend(SplitOr(axiom))
		else:
			new_description.append(axiom)
	
	# second run through the set of rules, note the frame rules, function
	# constants with frame rules, parse out the "role" rules, "init" rules, and facts
	frame_rules = [] # [(head, body)]
	for r in new_description:
		rule = GGPRule(r)

		if rule.head().name() == "goal":
			# we want to take note of the best possible single score
			score_term = rule.head().term(1)
			if score_term.type() == "constant" and int(str(score_term)) > best_score:
				best_score = int(str(score_term))
		
		# now we want to filter out rules with frame axioms, "role" rules, "init" rules,
		# and facts, to be processed differently from normal rules
		if rule.head().name() == "next":
			frame_term = rule.head().true_analogue()
			# check to see if this is a frame rule
			# frame rules are rules of the type
			#
			# (<= (next <some term>)
			#     ...
			#     (true <some term>)
			#     ...)
			if rule.has_body() and frame_term in rule.body():
				# this is a frame rule, process it differently later
				frame_rules.append(rule)
				# we also remember that this function constant is supported by frame rules
				funcs_with_frame_rules.append(frame_term.term(0))
			else:
				# normal implications, to be processed in the next step
				implications.append(rule)
		
		elif rule.head().name() == "role":
			role = str(rule.head().term(0))
		elif rule.head().name() == "init":
			init_rules.append(rule)
		elif rule.head().name() not in GGPSentence.DEFINED_RELS:
			# these can be one of two possible things, conditional or unconditional
			# elaborations. I call unconditional elaborations facts, and they
			# should be put into the ^facts structure in the state to be shallow
			# copied during a look-ahead. On the other hand conditional elaborations
			# should be put on the ^elaborations structure. But if the same relation
			# appears in both conditional and unconditional elaborations, all
			# elaborations for that relation should be treated as conditional
			if rule.has_body():
				# conditional
				if rule.head().name() in uncond_elabs:
					# there were other unconditional elabs for this relation,
					# move them over to the conditional side
					implications.extend(uncond_elabs[rule.head().name()])
					uncond_elabs.pop(rule.head().name())
				cond_elabs.add(rule.head().name())
				implications.append(rule)
			else:
				# unconditional
				if rule.head().name() not in cond_elabs:
					if rule.head().name() not in uncond_elabs:
						uncond_elabs[rule.head().name()] = [rule]
					else:
						uncond_elabs[rule.head().name()].append(rule)
				else:
					implications.append(rule)
		else:
			# normal implications, to be processed in the next step
			implications.append(rule)

	# Since we can't really tell what the best possible score will be, rely on a
	# heuristic that says that we've succeeded if we obtain a score higher than or equal
	# to the single highest reward
	if best_score == -99999:
		raise Exception("There are no goal rules")
	
	fact_rules = []
	for r in uncond_elabs.values():
		fact_rules.extend(r)
		
	GGPSentence.fact_rels = set([ f.head().name() for f in fact_rules ])
	
	# here we process all normal rules
	
	for rule in implications:
		if rule.head().name() == "next":
			have_removal_rule = False
			for f in funcs_with_frame_rules:
				if f.covers(rule.head().term(0)):
					have_removal_rule = True
					break
				
			if have_removal_rule:
				productions.extend(TranslateImplication(game_name, rule, best_score, make_remove_rule = False))
			else:
				# if a "next" relation isn't supported by frame rules, we have to make
				# a rule that explicitly removes it every time it's been on the game state
				# for one step
				productions.extend(TranslateImplication(game_name, rule, best_score, make_remove_rule = True))
				funcs_with_frame_rules.append(rule.head().term(0))
		else:
			productions.extend(TranslateImplication(game_name, rule, best_score, make_remove_rule = False))

	# get the init rules
	productions.extend(MakeInitRule(game_name, role, init_rules, fact_rules, best_score))
	
	# process the frame axioms

	# First, we have to merge those frame axioms whose heads are identical except for variable names.
	# We do this by constructing an N x N matrix M, where N is the number of variables in the head,
	# and M[i][j] = 1 if the ith and jth variables are specified to be distinct

	var_prefix = "std_soar_var"
	std_frame_rules = []
	for f in frame_rules:
		# standardize all variable names. This is so that if two head literals are the same, they will
		# match string-wise
		head = f.head()
		body = f.body()
		var_map = dict()
		count = 0
		count = StandardizeVars(head, var_prefix, var_map, count)
		for b in body:
			tmp_count = StandardizeVars(b, var_prefix, var_map, count)

		# create the matrix
		count = 0
		var_map = dict()
		for i in range(head.term(0).num_terms()):
			func_term = head.term(0).term(i)
			if func_term.type() == "variable":
				var_map[str(func_term)] = count
				count += 1
	
		m = [[0] * count for i in range(count)]

		for b in body:
			if b.name() == "distinct":
				if var_map.has_key(str(b.term(0))) and var_map.has_key(str(b.term(1))):
					m[var_map[str(b.term(0))]][var_map[str(b.term(1))]] = 1
		
		std_frame_rules.append((head, [body], m))

	# go through the list, finding the repetitions
	i = 0
	while i < len(std_frame_rules):
		j = i + 1
		while j < len(std_frame_rules):
			h1 = std_frame_rules[i][0]
			m1 = std_frame_rules[i][2]
			h2 = std_frame_rules[j][0]
			m2 = std_frame_rules[j][2]

			if h1 == h2 and m1 == m2:
				# these two are the same, so we have to merge them together
				std_frame_rules[i][1].extend(std_frame_rules[j][1])

				#remove the merged entry
				std_frame_rules.pop(j)
				continue

			j += 1
		i += 1

	# merge the frame axioms into production rules
	for f in std_frame_rules:
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

	# create selection space rules
	productions.append(MakeSelectionSpaceFakeResponseRule(game_name, role))
	productions.extend(MakeSelectionSpaceRules(game_name))

	sp = SoarProduction("elab-link", "elaborate")
	sp.state_cond().add_ground_predicate("type", "state")
	sp.add_action().add_id_wme_action("elaborations")
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
