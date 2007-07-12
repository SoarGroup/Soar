#!/usr/bin/python

from UniqueNameGen import UniqueNameGenerator
from SoarProd import *
from ElementGGP import ElementGGP as ElemGGP
from GGPRule import GGPRule
from GGPSentence import GGPSentence
import re
import ElementGGP
import pdb

name_gen = UniqueNameGenerator()

def SoarifyStr(s):
	p = re.compile('(\s|\?|\(|\)|\.)')
	result = p.sub('_', str(s))
	if len(result.strip()) == 0:
		return 'a'
		
	return result.strip()

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
def MakeTemplateProduction(name, type = "", game_name = 'game'):
	if type != "":
		prod_name = "%s*%s" % (type, name_gen.get_name(name))
	else:
		prod_name = name_gen.get_name(name)
	p = SoarProd(prod_name, game_name)
	return p

def MakeApplyRule(sp):
	op_names = sp.get_proposed_op_names()

	assert len(op_names) > 0, "Operator doesn't propose any operators"
	assert len(op_names) == 1, "Proposing more than one operator with one rule"

	prod_name = sp.get_name().replace('propose', 'apply')
	ap = SoarProd(prod_name, sp.get_state_name())
	ap.add_operator_test(op_names[0])
	return ap
	
def ParseGDLBodyToCondition(body, prod, var_map):
	for b in body:
		if not b.name() in ["distinct", '<', '>', '>=']:
			b.make_soar_conditions(prod, var_map)


def ParseComparisons(body, sp, var_map):
	for sentence in body:
		if sentence.name() not in ['distinct', '<', '>', '>=']:
			continue
		if sentence.term(0).type() == 'variable':
			lhs_index = 0
			rhs_index = 1
			reverse = False
		else:
			assert sentence.term(1).type() == 'variable', "Have to have at least one variable in a comparison"
			lhs_index = 1
			rhs_index = 0
			reverse = True

		if sentence.name() == "distinct":
			v1 = var_map.get_var(str(sentence.term(lhs_index)))
			if sentence.term(rhs_index).type() == "variable":
				v2 = var_map.get_var(str(sentence.term(rhs_index)))
				sp.add_id_predicate(v1, "<>", v2)
			else:
				sp.add_predicate(v1, "<>", str(sentence.term(rhs_index)))
		elif sentence.name() in ['<', '>', '>=']:
			rev_map = {'<':'>', '>':'<', '>=':'<=', '<=':'>='}
			if reverse:
				comp = rev_map[sentence.name()]
			else:
				comp = sentence.name()
			v1 = var_map.get_var(str(sentence.term(lhs_index)))
			if sentence.term(rhs_index).type() == "variable":
				v2 = var_map.get_var(str(sentence.term(rhs_index)))
				sp.add_id_predicate(v1, comp, v2)
			else:
				sp.add_predicate(v1, comp, str(sentence.term(rhs_index)))


def MakeInitRule(game_name, role, init_conds, fact_rules, min_success_score):
	sp = MakeTemplateProduction("init-%s" % game_name, "propose", game_name);
	sp.add_attrib(sp.get_state_id(), "superstate", "nil")
	sp.add_neg_attrib(sp.get_state_id(), "name", "")
	op_id = sp.add_operator_prop("init-%s" % game_name, "+ >")

	asp = MakeApplyRule(sp)

	asp.add_create_constant(asp.get_state_id(), 'name', game_name)
	asp.add_create_constant(asp.get_state_id(), 'next-action', '0')

	asp.add_create_id(asp.get_state_id(), 'desired')

	gs_id = asp.add_create_id(asp.get_state_id(), 'gs')
	fact_id = asp.add_create_id(asp.get_state_id(), 'facts')
#	game_elabs_var = state_action.add_id_wme_action("elaborations")
	asp.add_create_constant(gs_id, 'action-counter', '0')

	if role == "":
		print "Warning: No role defined"
	else:
		asp.add_create_constant(gs_id, 'role', role)

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
	sp = MakeTemplateProduction(SoarifyStr(str(head)), 'elaborate')
	var_map = GDLSoarVarMapper(sp.get_name_gen())
	if len(body) > 0:
		ParseGDLBodyToCondition(body, sp, var_map)
		ParseComparisons(body, sp, var_map)
	head.make_soar_actions(sp, var_map)
	return [sp]
	
def TranslateTerminal(game_name, head, body):
	sp = MakeTemplateProduction(SoarifyStr(str(head)), 'elaborate')
	var_map = GDLSoarVarMapper(sp.get_name_gen())
	ParseGDLBodyToCondition(body, sp, var_map)
	ParseComparisons(body, sp, var_map)
	
	# different actions depending on if we're in the selection space
	# or operating for real
	sp_sel = sp.copy(name_gen.get_name(sp.get_name()))
	sp_sel.add_id_attrib(sp_sel.get_state_id(), 'duplicate-of')
	sp_sel.add_create_id(sp_sel.get_state_id(), 'terminal')
	
	sp.add_neg_id_attrib(sp.get_state_id(), 'duplicate-of')
	sp.add_rhs_func_call('halt')
	
	return [sp, sp_sel]

def TranslateLegal(game_name, head, body):
	move = head.term(1).name()
	
	sp = MakeTemplateProduction(SoarifyStr(str(head)), "propose", game_name)
	var_map = GDLSoarVarMapper(sp.get_name_gen())
	for relation in body:
		if not relation.name() in ["distinct", ">", "<", ">="]:
			relation.make_soar_conditions(sp, var_map)
	
	# have to also check that no moves have been made
	olink_id = sp.get_or_make_id_chain(['io','output-link'])[0]
	sp.add_neg_id_attrib(olink_id, '<cmd-name>')
	
	head.make_soar_actions(sp, var_map)
	ParseComparisons(body, sp, var_map)
	
	# apply rule

	ap = MakeApplyRule(sp)
	op_id = ap.get_ids(ap.get_state_id(), 'operator')[0]
	ol_id = ap.get_or_make_id_chain(['io','output-link'])[0]

	ap_var_map = GDLSoarVarMapper(ap.get_name_gen())
	head.term(1).make_soar_cond_no_id(ap, op_id, ap_var_map, 1)

	#move_act = ap.add_action(ap.add_action(ol_var).add_id_wme_action(move))
	#head.term(1).make_soar_action_no_id(ap, move_act, ap_var_map)
	head.term(1).make_soar_action(ap, ol_id, 0, ap_var_map)
	
	return [sp, ap]

def TranslateNext(game_name, head, body, make_remove_rule = True):
	ap = MakeTemplateProduction(SoarifyStr(str(head)), 'apply', game_name)
	var_map = GDLSoarVarMapper(ap.get_name_gen())
	ap.add_operator_test('update-state')
	
	for sentence in body:
		if not sentence.name() in ["distinct", '>', '<', '>=']:
			sentence.make_soar_conditions(ap, var_map)
	
	head.make_soar_actions(ap, var_map)
	ParseComparisons(body, ap, var_map)
	
	# there are no frame axioms, so make productions that get rid of this after one
	# step
	if make_remove_rule:
		rap = MakeTemplateProduction("remove*%s" % SoarifyStr(str(head)), 'apply', game_name)
		var_map = GDLSoarVarMapper(rap.get_name_gen())
		rap.add_operator_test("update-state")
		head.make_soar_actions(rap, var_map, remove = True)
		return [ap, rap]
	else:
		return [ap]


def TranslateGoal(game_name, head, body, score):
	sp = MakeTemplateProduction(SoarifyStr(str(head)), 'elaborate', game_name)
	var_map = GDLSoarVarMapper(sp.get_name_gen())
	sp.add_id_attrib(sp.get_state_id(), 'terminal')
	desired_id = sp.add_id_attrib(sp.get_state_id(), 'desired')
	
	ParseGDLBodyToCondition(body, sp, var_map)
	ParseComparisons(body, sp, var_map)
	
	if int(str(head.term(1))) >= score:
		sp.add_create_bound_id(sp.get_state_id(), "success-detected", desired_id)
	else:
		sp.add_create_bound_id(sp.get_state_id(), "failure-detected", desired_id)

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
#def TranslateFrameAxioms(game_name, head, bodies):
#	productions = []
#
#	axiom_name = SoarifyStr(str(head))
#	frame_sentence = head.true_analogue()
#
#	comparison_literals = []
#	new_bodies = []
#	# eliminate frame conditions from body
#	# negate each condition in each body
#	for b in bodies:
#		newb = []
#		for s in b:
#			if s != frame_sentence:
#				if s.name() in ['distinct', '<', '>', '>=']:
#					# preserve distinct
#					newb.append(s)
#				else:
#					newb.append(s.negate())
#		new_bodies.append(newb)
#
#	# mangle variables in each body to prevent name collisions
##	name_gen = UniqueNameGenerator()
#	for b,i in zip(new_bodies, range(len(new_bodies))):
#		for c in b:
#			c.mangle_vars('__r%d__' % i)
#
##	for b in new_bodies:
##		mangled = dict()
##		for c in b:
##			c.mangle_vars(mangled, name_gen)
#
#	# collect all comparisons
#	for b in new_bodies:
#		i = 0
#		while i < len(b):
#			if b[i].name() in ['distinct', '<', '>', '>=']:
#				comparison_literals.append(b[i])
#				b.pop(i)
#			else:
#				i += 1
#
#	# take all possible combinations of one condition from each body
#	#   (this is the same as ANDing all bodies together and distributing
#	#    ANDs over ORs [ the ORs resulted from distributing negations
#	#    into the ANDs implicit in each body])
#
#	combinations = GenCombinations([len(b) for b in new_bodies])
#	props = []
#	op_name = name_gen.get_name("remove-%s" % axiom_name)
#	for ci, x in zip(combinations, range(len(combinations))):
#		comb = []
#		for i in range(len(ci)):
#			if new_bodies[i][ci[i]] not in comb:
#				comb.append(new_bodies[i][ci[i]])
#
#		comb_body = ElementGGP.Combine(list(comb) + comparison_literals)
#		prod_name = name_gen.get_name("%s%d" % (op_name, x))
#		sp = MakeTemplateProduction("apply", prod_name, game_name)
#		sp.add_op_cond("update-state")
#		var_mapper = GDLSoarVarMapper(sp.var_gen)
#
#		# first, add the condition to check for presence of head relation
#		frame_sentence.make_soar_conditions(sp, var_mapper)
#
#		# now add the conditions sufficient for removing the head relation from the
#		# next state
#		ParseGDLBodyToCondition(comb_body, sp, var_mapper)
#		
#		frame_sentence.next_analogue().make_soar_actions(sp, var_mapper, remove = True)
#
#		# set up the variable distinctions
#		ParseComparisons(comb_body, sp, var_mapper)
#
#		productions.append(sp)
#
#	return productions

def TranslateFrameAxioms(game_name, head, bodies):
	productions = []

	axiom_name = SoarifyStr(str(head))
	frame_sentence = head.true_analogue()

	comparison_literals = []
	new_bodies = []
	# eliminate frame conditions from body
	for b in bodies:
		newb = []
		for s in b:
			if s != frame_sentence:
				newb.append(s)
		new_bodies.append(newb)

	# mangle variables in each body to prevent name collisions
#	name_gen = UniqueNameGenerator()
	for i, b in enumerate(new_bodies):
		for c in b:
			c.mangle_vars('__r%d__' % i)

	sp = MakeTemplateProduction("remove-frame-%s" % axiom_name, "apply", game_name)
	sp.add_operator_test("update-state")
	var_mapper = GDLSoarVarMapper(sp.get_name_gen())

	# first, add the condition to check for presence of head relation
	frame_sentence.make_soar_conditions(sp, var_mapper)

	# combine the bodies of all frame axioms together
	for body in new_bodies:
		sp.begin_negative_conjunction() # wrap all the added conditions into a negation
		ParseGDLBodyToCondition(body, sp, var_mapper)
		ParseComparisons(body, sp, var_mapper)
		sp.end_negative_conjunction()

	frame_sentence.next_analogue().make_soar_actions(sp, var_mapper, remove = True)

	return sp

def StandardizeVars(sentence, add_new, prefix, map = dict(), count = 0):
	for i in range(sentence.num_terms()):
		c = sentence.term(i)
		if c.type() == "variable":
			if not map.has_key(str(c)) and add_new:
				map[str(c)] = count
				count += 1
			if map.has_key(str(c)):
				new_name = "%s%d" % (prefix, map[str(c)])
				sentence.term(i).rename(new_name)
		elif c.type() == "function":
			count = StandardizeVars(c, add_new, prefix, map, count)

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
			if frame_term in rule.body():
				if len(rule.body()) > 1:
					# this is a frame rule, process it differently later
					frame_rules.append(rule)
				else:
					# this is a frame rule which will always be true, so the structure
					# will never be removed
					# why wasn't this fixed a long time ago?
					print rule
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
		#raise Exception("There are no goal rules")
		print "Warning: There are no goal rules"
	
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
		count = StandardizeVars(head, True, var_prefix, var_map, count)
		for b in body:
			#tmp_count = StandardizeVars(b, var_prefix, var_map, count)
			# what's going on here? I have no idea why I used tmp_count
			# instead of just count
			count = StandardizeVars(b, False, var_prefix, var_map, count)

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
		#productions.extend(TranslateFrameAxioms(game_name, f[0], f[1]))
		productions.append(TranslateFrameAxioms(game_name, f[0], f[1]))

	f = open(filename, 'w')
	f.write("source header.soar\n")

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
